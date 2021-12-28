#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <assert.h>

#include <orca/discord.h>

#include "interactions.h"

struct discord *ceebot;

void
on_interaction_create(struct discord *ceebot,
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
        .content =
          "⚠️ Internal Error! Interaction is malfunctioning, please "
          "report to the staff.",
      },
  };

  switch (interaction->type) {
  case DISCORD_INTERACTION_APPLICATION_COMMAND:
    if (0 == strcmp(interaction->data->name, "mychannel")) {

      if (interaction->data->options)
        for (int i = 0; interaction->data->options[i]; ++i) {
          char *cmd = interaction->data->options[i]->name;

          if (0 == strcmp(cmd, "action")) {
          }
          else if (0 == strcmp(cmd, "configure")) {
            react_rubberduck_channel_configure(
              ceebot, &params, interaction,
              interaction->data->options[i]->options);
          }
          else if (0 == strcmp(cmd, "delete")) {
            react_rubberduck_channel_delete(
              ceebot, &params, interaction,
              interaction->data->options[i]->options);
          }
        }
    }
    break;
  case DISCORD_INTERACTION_MESSAGE_COMPONENT:
    if (0 == strcmp(interaction->data->custom_id, "create-channel"))
      react_rubberduck_channel_menu(ceebot, &params, interaction);
    break;
  default:
    log_error("%s (%d) is not dealt with",
              discord_interaction_types_print(interaction->type),
              interaction->type);
    break;
  }

  discord_async_next(ceebot, NULL);
  discord_create_interaction_response(ceebot, interaction->id,
                                      interaction->token, &params, NULL);
}

/* shutdown gracefully on SIGINT received */
void
sigint_handler(int signum)
{
  (void)signum;
  log_info("SIGINT received, shutting down ...");
  discord_shutdown(ceebot);
}

void
on_ready(struct discord *ceebot)
{
  const struct discord_user *bot = discord_get_self(ceebot);

  log_info("Cee-Bot succesfully connected to Discord as %s#%s!", bot->username,
           bot->discriminator);
}

struct ceebot_primitives
ceebot_get_primitives(struct discord *ceebot)
{
  struct ceebot_primitives primitives = { 0 };
  struct sized_buffer json;
  struct logconf *conf = discord_get_logconf(ceebot);

  /* get guild id */
  json = logconf_get_field(conf, "cee_bot.guild_id");
  primitives.guild_id = strtoull(json.start, NULL, 10);

  /* get rubberduck channels category id */
  json = logconf_get_field(conf, "cee_bot.category_id");
  primitives.category_id = strtoull(json.start, NULL, 10);

  /* get roles */
  json = logconf_get_field(conf, "cee_bot.roles.rubberduck_id");
  primitives.roles.rubberduck_id = strtoull(json.start, NULL, 10);
  json = logconf_get_field(conf, "cee_bot.roles.helper_id");
  primitives.roles.helper_id = strtoull(json.start, NULL, 10);
  json = logconf_get_field(conf, "cee_bot.roles.lurker_id");
  primitives.roles.lurker_id = strtoull(json.start, NULL, 10);
  json = logconf_get_field(conf, "cee_bot.roles.watcher_id");
  primitives.roles.watcher_id = strtoull(json.start, NULL, 10);
  json = logconf_get_field(conf, "cee_bot.roles.announcements_id");
  primitives.roles.announcements_id = strtoull(json.start, NULL, 10);

  return primitives;
}

int
main(int argc, char *argv[])
{
  struct ceebot_primitives primitives;

  signal(SIGINT, &sigint_handler);
  orca_global_init();

  ceebot = discord_config_init((argc > 1) ? argv[1] : "config.json");
  assert(NULL != ceebot && "Couldn't initialize client");

  primitives = ceebot_get_primitives(ceebot);
  discord_set_data(ceebot, &primitives);

  discord_set_on_ready(ceebot, &on_ready);
  discord_set_on_interaction_create(ceebot, &on_interaction_create);

  discord_run(ceebot);

  discord_cleanup(ceebot);
  orca_global_cleanup();
}
