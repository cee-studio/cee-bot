#include <string.h>
#include <inttypes.h> /* PRIu64 */

#include <orca/discord.h>

#include "interactions.h"

static void
mentorship_channel_delete(struct discord *client,
                          struct discord_async_ret *ret)
{
  struct client_context *client_cxt = discord_get_data(client);
  const struct discord_channel *channel = ret->ret;
  struct async_context *async_cxt = ret->data;
  struct discord_edit_original_interaction_response_params params = {
    .content = "Couldn't complete operation. Make sure to use command from "
               "your channel.",
  };

  if (channel->permission_overwrites)
    for (int i = 0; channel->permission_overwrites[i]; ++i)
      if (async_cxt->user_id == channel->permission_overwrites[i]->id
          && client_cxt->category_id == channel->parent_id)
      {
        /* delete user channel */
        discord_async_next(client, NULL);
        discord_delete_channel(client, channel->id, NULL);

        /* remove mentorship role from user */
        discord_async_next(client, NULL);
        discord_remove_guild_member_role(client, client_cxt->guild_id,
                                         async_cxt->user_id,
                                         client_cxt->roles.mentorship_id);

        log_info("Remove mentorship role from %" PRIu64, async_cxt->user_id);

        params.content = "Channel has been deleted succesfully";

        break;
      }

  discord_async_next(client, NULL);
  discord_edit_original_interaction_response(client, async_cxt->application_id,
                                             async_cxt->token, &params, NULL);
}

void
react_mentorship_channel_delete(
  struct discord *client,
  struct discord_interaction_response *params,
  const struct discord_interaction *interaction,
  struct discord_application_command_interaction_data_option **options)
{
  struct discord_guild_member *member = interaction->member;
  bool confirm_action = false;
  char *reason = NULL;

  if (options)
    for (int i = 0; options[i]; ++i) {
      char *name = options[i]->name;
      char *value = options[i]->value;

      if (0 == strcmp(name, "confirm"))
        confirm_action = (0 == strcmp(value, "yes"));
      else if (0 == strcmp(name, "reason"))
        reason = value;
    }

  if (!confirm_action) {
    params->data->content = "No operation will be taking place.";
    return;
  }

  /* TODO: post to a logging channel */
  if (reason) {
    log_info("User %" PRIu64 " channel deletion reason: %s", member->user->id,
             reason);
  }

  struct async_context *async_cxt = malloc(sizeof *async_cxt);
  async_cxt->user_id = member->user->id;
  async_cxt->application_id = interaction->application_id;
  snprintf(async_cxt->token, sizeof(async_cxt->token), "%s",
           interaction->token);

  discord_async_next(client, &(struct discord_async_attr){
                               .done = &mentorship_channel_delete,
                               .data = async_cxt,
                               .cleanup = &free,
                             });
  discord_get_channel(client, interaction->channel_id, NULL);

  params->type =
    DISCORD_INTERACTION_CALLBACK_DEFERRED_CHANNEL_MESSAGE_WITH_SOURCE;
}
