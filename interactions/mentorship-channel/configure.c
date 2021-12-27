#include <string.h>
#include <inttypes.h> /* PRIu64 */

#include <orca/discord.h>

#include "interactions.h"

static void
mentorship_channel_modify(struct discord *client,
                          struct discord_async_ret *ret)
{
  struct discord_edit_original_interaction_response_params params = { 0 };
  struct client_context *client_cxt = discord_get_data(client);
  const struct discord_channel *channel = ret->ret;
  struct async_context *async_cxt = ret->data;
  bool priv = async_cxt->data;

  if (!is_user_channel(channel, client_cxt->category_id, async_cxt->user_id)) {
    params.content = "Couldn't complete operation. Make sure to use command "
                     "from your channel.";
  }
  else {
    /* edit user channel */
    discord_async_next(client, NULL);
    discord_edit_channel_permissions(
      client, channel->id, client_cxt->roles.lurker_id,
      &(struct discord_edit_channel_permissions_params){
        .type = 0,
        .allow = priv ? 0 : PERMS_DEFAULT,
      });

    params.content = "Channel visibility has been changed succesfully";
  }

  discord_async_next(client, NULL);
  discord_edit_original_interaction_response(client, async_cxt->application_id,
                                             async_cxt->token, &params, NULL);
}

void
react_mentorship_channel_configure(
  struct discord *client,
  struct discord_interaction_response *params,
  const struct discord_interaction *interaction,
  struct discord_application_command_interaction_data_option **options)
{
  struct discord_guild_member *member = interaction->member;
  char *visibility = NULL;

  if (options)
    for (int i = 0; options[i]; ++i) {
      char *name = options[i]->name;
      char *value = options[i]->value;

      if (0 == strcmp(name, "visibility")) visibility = value;
    }

  if (!visibility) {
    params->data->content = "No operation will be taking place.";
    return;
  }

  struct async_context *async_cxt = malloc(sizeof *async_cxt);
  async_cxt->user_id = member->user->id;
  async_cxt->application_id = interaction->application_id;
  snprintf(async_cxt->token, sizeof(async_cxt->token), "%s",
           interaction->token);
  async_cxt->data = (0 == strcmp(visibility, "private")) ? (void *)1 : NULL;

  discord_async_next(client, &(struct discord_async_attr){
                               .done = &mentorship_channel_modify,
                               .data = async_cxt,
                               .cleanup = &free,
                             });
  discord_get_channel(client, interaction->channel_id, NULL);

  params->type =
    DISCORD_INTERACTION_CALLBACK_DEFERRED_CHANNEL_MESSAGE_WITH_SOURCE;
}
