#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <orca/discord.h>
#include <orca/cee-utils.h>

struct discord_guild *
get_guild(struct discord *ceebot)
{
  struct discord_guild *guild = calloc(1, sizeof *guild);
  struct logconf *conf = discord_get_logconf(ceebot);
  struct sized_buffer guild_id = { 0 };
  ORCAcode code;

  guild_id = logconf_get_field(conf, "cee_bot.guild_id");
  assert(guild_id.size != 0 && "Missing cee_bot.guild_id");

  code = discord_get_guild(ceebot, strtoull(guild_id.start, NULL, 10), guild);
  if (code != ORCA_OK) {
    log_fatal("%s", discord_strerror(code, ceebot));
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
  struct discord *ceebot;

  assert(argc > 1 && "Expect: ./menu <path>/menu.json <?config_file>");

  ceebot = discord_config_init((argc > 2) ? argv[2] : "../config.json");
  assert(NULL != ceebot && "Couldn't initialize ceebot");

  guild = get_guild(ceebot);
  params = get_components(argv[1]);

  discord_create_message(ceebot, guild->rules_channel_id, params, NULL);
}
