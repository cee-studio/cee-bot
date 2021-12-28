#include <string.h>
#include <inttypes.h> /* PRIu64 */

#include <orca/discord.h>

#include "interactions.h"

static void
on_rubberduck_channel_create(struct discord *ceebot,
                             struct discord_async_ret *ret)
{
  struct ceebot_primitives *primitives = discord_get_data(ceebot);
  char welcome_msg[DISCORD_MAX_MESSAGE_LEN] = "";

  const struct discord_channel *channel = ret->ret;
  u64_snowflake_t *user_id = ret->data;

  /* assign rubberduck role to user */
  discord_async_next(ceebot, NULL);
  discord_add_guild_member_role(ceebot, primitives->guild_id, *user_id,
                                primitives->roles.rubberduck_id);

  snprintf(welcome_msg, sizeof(welcome_msg),
           "Welcome <@!%" PRIu64 ">, "
           "I hope you enjoy your new space! Check your available commands by "
           "typing `/mychannel` for further channel customization!",
           *user_id);

  discord_async_next(ceebot, NULL);
  discord_create_message(ceebot, channel->id,
                         &(struct discord_create_message_params){
                           .content = welcome_msg,
                         },
                         NULL);
}

void
react_rubberduck_channel_menu(struct discord *ceebot,
                              struct discord_interaction_response *params,
                              const struct discord_interaction *interaction)
{
  struct ceebot_primitives *primitives = discord_get_data(ceebot);

  struct discord_guild_member *member = interaction->member;
  bool priv = false;

  /* skip user with already assigned channel */
  if (is_included_role(member->roles, primitives->roles.rubberduck_id)) {
    params->data->content =
      "It seems you already have a channel, please edit it from within";
    return;
  }

  /* get channel visibility */
  if (interaction->data->values)
    for (int i = 0; interaction->data->values[i]; ++i) {
      char *value = interaction->data->values[i]->value;

      if (0 == strcmp(value, "private")) priv = true;
    }

  u64_snowflake_t *user_id = malloc(sizeof(u64_snowflake_t));
  *user_id = member->user->id;

  /* create user channel */
  discord_async_next(ceebot, &(struct discord_async_attr){
                               .done = &on_rubberduck_channel_create,
                               .data = user_id,
                               .cleanup = &free });
  discord_create_guild_channel(
    ceebot, primitives->guild_id,
    &(struct discord_create_guild_channel_params){
      .name = member->user->username,
      .permission_overwrites =
        (struct discord_overwrite *[]){
          /* give read/write permission for user */
          &(struct discord_overwrite){
            .id = *user_id,
            .type = 1,
            .allow = PERMS_DEFAULT,
          },
          /* give read/write permission for @helper */
          &(struct discord_overwrite){
            .id = primitives->roles.helper_id,
            .type = 0,
            .allow = PERMS_DEFAULT,
          },
          /* hide it from @watcher only if 'priv' has been set */
          &(struct discord_overwrite){
            .id = primitives->roles.watcher_id,
            .type = 0,
            .allow = priv ? 0 : PERMS_DEFAULT,
          },
          /* hide it from @everyone */
          &(struct discord_overwrite){
            .id = primitives->guild_id,
            .type = 0,
            .deny = PERMS_ALL,
          },
          NULL, /* END OF OVERWRITE LIST */
        },
      .parent_id = primitives->category_id,
    },
    NULL);

  params->data->content = "Your channel will be created shortly";
}
