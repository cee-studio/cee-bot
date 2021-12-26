#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <orca/discord.h>
#include <orca/cee-utils.h>

#define COMMANDS_FILE "mentorship-channel/cmd.json"

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

u64_snowflake_t
get_application_id(struct discord *client)
{
  struct logconf *conf = discord_get_logconf(client);
  struct sized_buffer app_id = { 0 };

  app_id = logconf_get_field(conf, "cee_bot.application_id");
  assert(app_id.size != 0 && "Missing cee_bot.application_id");

  return (u64_snowflake_t)strtoull(app_id.start, NULL, 10);
}

struct discord_create_guild_application_command_params *
get_application_commands(const char fname[])
{
  struct discord_create_guild_application_command_params *params;
  size_t fsize = 0;
  char *fcontents;

  params = calloc(1, sizeof *params);
  fcontents = cee_load_whole_file(fname, &fsize);

  discord_create_guild_application_command_params_from_json(fcontents, fsize,
                                                            params);

  return params;
}

int
main(int argc, char *argv[])
{
  struct discord_create_guild_application_command_params *params;
  u64_snowflake_t application_id;
  struct discord_guild *guild;
  struct discord *client;

  client = discord_config_init((argc > 1) ? argv[1] : "../config.json");
  assert(NULL != client && "Couldn't initialize client");

  guild = get_guild(client);
  application_id = get_application_id(client);
  params = get_application_commands(COMMANDS_FILE);

  discord_create_guild_application_command(client, application_id, guild->id,
                                           params, NULL);
}
