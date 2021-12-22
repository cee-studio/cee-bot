#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <assert.h>

#include <orca/discord.h>

struct discord *client;
struct logconf *conf;

u64_snowflake_t g_category_id;
u64_snowflake_t g_role_id;
u64_snowflake_t g_guild_id;

void
handle_create_a_channel(struct discord *client,
                        const struct discord_interaction *interaction)
{
  struct discord_guild_member *member = interaction->member;
  bool private = false;

  /* skip user with already assigned channel
   * TODO: edit existing channel */
  if (member->roles)
    for (int i = 0; member->roles[i]; ++i) {
      u64_snowflake_t role_id = member->roles[i]->value;

      if (g_role_id == role_id) return;
    }

  /* get channel visibility */
  if (interaction->data->values)
    for (int i = 0; interaction->data->values[i]; ++i) {
      char *value = interaction->data->values[i]->value;

      if (0 == strcmp(value, "private")) {
        private = true;
      }
    }

  /* create user channel */
  discord_create_guild_channel(
    client, g_guild_id,
    &(struct discord_create_guild_channel_params){
      .name = member->user->username,
      .permission_overwrites =
        (struct discord_overwrite *[]){
          /* see https://discordapi.com/permissions.html#377957256256 */
          &(struct discord_overwrite){
            .id = member->user->id,
            .type = 1,
            .allow = 1377957256256,
          },
          /* hide from @everyone only if 'private' has been set */
          private ? &(struct discord_overwrite){
            .id = g_guild_id,
            .type = 0,
            .deny = (enum discord_bitwise_permission_flags) - 1,
          } : NULL,
          NULL, /* END OF OVERWRITE LIST */
        },
      .parent_id = g_category_id,
    },
    NULL);

  /* assign mentorship role to user */
  discord_add_guild_member_role(client, g_guild_id, member->user->id,
                                g_role_id);
}

void
on_interaction_create(struct discord *client,
                      const struct discord_interaction *interaction)
{
  /* Return in case of missing user input */
  if (!interaction->data) return;

  switch (interaction->type) {
  case DISCORD_INTERACTION_MESSAGE_COMPONENT:
    if (0 == strcmp(interaction->data->custom_id, "create-a-channel"))
      handle_create_a_channel(client, interaction);
    break;
  default:
    log_error("%s (%d) is not dealt with",
              discord_interaction_types_print(interaction->type),
              interaction->type);
    break;
  }
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

  signal(SIGINT, &sigint_handler);
  orca_global_init();

  client = discord_config_init((argc > 1) ? argv[1] : "config.json");
  assert(NULL != client && "Couldn't initialize client");

  conf = discord_get_logconf(client);

  discord_set_on_ready(client, &on_ready);
  discord_set_on_interaction_create(client, &on_interaction_create);

  /* get guild id */
  json = logconf_get_field(conf, "cee_bot.guild_id");
  g_guild_id = strtoull(json.start, NULL, 10);
  /* get mentorship role id */
  json = logconf_get_field(conf, "cee_bot.mentorship.role_id");
  g_role_id = strtoull(json.start, NULL, 10);
  /* get mentorship channels category id */
  json = logconf_get_field(conf, "cee_bot.mentorship.channels_category_id");
  g_category_id = strtoull(json.start, NULL, 10);

  discord_run(client);

  discord_cleanup(client);
  orca_global_cleanup();
}
