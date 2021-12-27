#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <orca/discord.h>
#include <orca/cee-utils.h>

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

struct discord_create_message_params *
get_components(const char fname[])
{
  struct discord_create_message_params *params = NULL;
  size_t fsize = 0;
  char *fcontents;

  fcontents = cee_load_whole_file(fname, &fsize);
  assert(fcontents != NULL && "Missing file");
  assert(fsize != 0 && "Empty file");

  discord_create_message_params_from_json_p(fcontents, fsize, &params);

  return params;
}

int
main(int argc, char *argv[])
{
  struct discord_create_message_params *params;
  struct discord_guild *guild;
  struct discord *client;

  assert(argc > 1 && "Expect: ./menu <path>/menu.json <?config_file>");

  client = discord_config_init((argc > 2) ? argv[2] : "../config.json");
  assert(NULL != client && "Couldn't initialize client");

  guild = get_guild(client);
  params = get_components(argv[1]);

  discord_create_message(client, guild->rules_channel_id, params, NULL);
}
