#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <orca/discord.h>
#include <orca/cee-utils.h>

#define COMPONENTS_FILE "components.json"

struct discord_guild *
get_guild(struct discord *client)
{
  struct discord_guild *guild = calloc(1, sizeof *guild);
  struct logconf *conf = discord_get_logconf(client);
  struct sized_buffer guild_id = { 0 };
  ORCAcode code;

  guild_id = logconf_get_field(conf, "cee_bot.guild_id");
  assert(guild_id.size != 0 && "Missing cee_bot.guild_id");

  code = discord_get_guild(client, strtoull(guild_id.start, NULL, 10), guild);
  if (code != ORCA_OK) {
    log_fatal("%s", discord_strerror(code, client));
    exit(EXIT_FAILURE);
  }

  return guild;
}

struct discord_component **
get_components(const char fname[])
{
  struct discord_component **components;
  size_t fsize = 0;
  char *fcontents;

  fcontents = cee_load_whole_file(fname, &fsize);

  discord_component_list_from_json(fcontents, fsize, &components);

  return components;
}

int
main(int argc, char *argv[])
{
  struct discord *client;
  struct discord_guild *guild;
  struct discord_component **components;

  client = discord_config_init((argc > 1) ? argv[1] : "../../config.json");
  assert(NULL != client && "Couldn't initialize client");

  guild = get_guild(client);
  components = get_components(COMPONENTS_FILE);

  struct discord_create_message_params params = {
    .content =
      "** Pick your mentorship channel **\n\n"
      "If you are just learning C and would specialized support for your "
      "project, you may create your own channel for that purpose.",
    .components = components
  };
  discord_create_message(client, guild->rules_channel_id, &params, NULL);
}
