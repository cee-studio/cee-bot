#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <signal.h>
#include <assert.h>

#include <orca/discord.h>

struct discord *client;
struct logconf *conf;

struct context {
  u64_snowflake_t guild_id;
  u64_snowflake_t category_id;
  struct {
    u64_snowflake_t mentorship_id;
    u64_snowflake_t helper_id;
    u64_snowflake_t lurker_id;
  } roles;
};

#include "cee-create-a-channel.c" /* cee_create_a_channel() */

void
on_interaction_create(struct discord *client,
                      const struct discord_interaction *interaction)
{
  /* Return in case of missing user input */
  if (!interaction->data) return;

  /* initialize interaction response with some default values */
  struct discord_interaction_response params = {
    .type = DISCORD_INTERACTION_CALLBACK_CHANNEL_MESSAGE_WITH_SOURCE,
    .data =
      &(struct discord_interaction_callback_data){
        .flags = DISCORD_INTERACTION_CALLBACK_DATA_EPHEMERAL,
      },
  };

  switch (interaction->type) {
  case DISCORD_INTERACTION_MESSAGE_COMPONENT:
    if (0 == strcmp(interaction->data->custom_id, "create-a-channel"))
      cee_create_a_channel(client, &params, interaction);
    break;
  default:
    log_error("%s (%d) is not dealt with",
              discord_interaction_types_print(interaction->type),
              interaction->type);
    return;
  }

  discord_async_next(client, NULL);
  discord_create_interaction_response(client, interaction->id,
                                      interaction->token, &params, NULL);
}

/* shutdown gracefully on SIGINT received */
void
sigint_handler(int signum)
{
  (void)signum;
  log_info("SIGINT received, shutting down ...");
  discord_shutdown(client);
}

void
on_ready(struct discord *client)
{
  const struct discord_user *bot = discord_get_self(client);

  log_info("Cee-Bot succesfully connected to Discord as %s#%s!", bot->username,
           bot->discriminator);
}

int
main(int argc, char *argv[])
{
  struct sized_buffer json;
  struct context cxt = { 0 };

  signal(SIGINT, &sigint_handler);
  orca_global_init();

  client = discord_config_init((argc > 1) ? argv[1] : "config.json");
  assert(NULL != client && "Couldn't initialize client");

  conf = discord_get_logconf(client);

  discord_set_on_ready(client, &on_ready);
  discord_set_on_interaction_create(client, &on_interaction_create);

  /* get guild id */
  json = logconf_get_field(conf, "cee_bot.guild_id");
  cxt.guild_id = strtoull(json.start, NULL, 10);
  /* get mentorship channels category id */
  json = logconf_get_field(conf, "cee_bot.category_id");
  cxt.category_id = strtoull(json.start, NULL, 10);
  /* get roles */
  json = logconf_get_field(conf, "cee_bot.roles.mentorship_id");
  cxt.roles.mentorship_id = strtoull(json.start, NULL, 10);
  json = logconf_get_field(conf, "cee_bot.roles.helper_id");
  cxt.roles.helper_id = strtoull(json.start, NULL, 10);
  json = logconf_get_field(conf, "cee_bot.roles.lurker_id");
  cxt.roles.lurker_id = strtoull(json.start, NULL, 10);

  discord_set_data(client, &cxt);

  discord_run(client);

  discord_cleanup(client);
  orca_global_cleanup();
}
