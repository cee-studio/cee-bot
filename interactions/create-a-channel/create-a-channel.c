#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <orca/discord.h>
#include <orca/cee-utils.h>

#define COMPONENTS_FILE "components.json"

int
main(int argc, char *argv[])
{
  struct discord *client;
  struct sized_buffer json = { 0 };
  /* load components from COMPONENTS_FILE */
  struct discord_component **components = NULL;
  /* load guild from 'config.json' guild_id */
  struct discord_guild guild;
  struct logconf *conf;
  ORCAcode code;

  client = discord_config_init((argc > 1) ? argv[1] : "../../config.json");
  assert(NULL != client && "Couldn't initialize client");

  /* load guild */
  conf = discord_get_logconf(client);
  json = logconf_get_field(conf, "cee_bot.guild_id");
  assert(json.size != 0 && "Missing cee_bot.guild_id");

  code = discord_get_guild(client, strtoull(json.start, NULL, 10), &guild);
  if (code != ORCA_OK) {
    log_fatal("%s", discord_strerror(code, client));
    exit(EXIT_FAILURE);
  }

  /* load JSON 'message components' string */
  json.start = cee_load_whole_file(COMPONENTS_FILE, &json.size);
  discord_component_list_from_json(json.start, json.size, &components);
  free(json.start);

  struct discord_create_message_params params = {
    .content =
      "** Pick your mentorship channel **\n\n"
      "If you are just learning C and would specialized support for your "
      "project, you may create your own channel for that purpose.",
    .components = components
  };
  discord_create_message(client, guild.rules_channel_id, &params, NULL);

  discord_component_list_free(components);
  discord_cleanup(client);
}
