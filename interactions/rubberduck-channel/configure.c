#include <stdlib.h>
#include <string.h>
#include <inttypes.h> /* PRIu64 */

#include <orca/discord.h>

#include "interactions.h"

/** @brief Per-request context storage for async functions */
struct context {
  /** the user that triggered the interaction */
  u64_snowflake_t user_id;
  /** the client's application id */
  u64_snowflake_t application_id;
  /** user's rubberduck channel */
  u64_snowflake_t channel_id;
  /** the interaction token */
  char token[256];
  /** whether rubberduck channel is private */
  bool priv;
};

static void
done_edit_permissions(struct discord *ceebot, struct discord_async_ret *ret)
{
  struct context *cxt = ret->data;
  char diagnosis[256];

  snprintf(diagnosis, sizeof(diagnosis),
           "Completed action targeted to <#%" PRIu64 ">.", cxt->channel_id);

  discord_async_next(ceebot, NULL);
  discord_edit_original_interaction_response(
    ceebot, cxt->application_id, cxt->token,
    &(struct discord_edit_original_interaction_response_params){
      .content = diagnosis,
    },
    NULL);

  free(cxt);
}

static void
fail_edit_permissions(struct discord *ceebot, struct discord_async_err *err)
{
  struct context *cxt = err->data;
  char diagnosis[256];

  snprintf(diagnosis, sizeof(diagnosis),
           "Failed action targeted to <#%" PRIu64 ">.", cxt->channel_id);

  discord_async_next(ceebot, NULL);
  discord_edit_original_interaction_response(
    ceebot, cxt->application_id, cxt->token,
    &(struct discord_edit_original_interaction_response_params){
      .content = diagnosis,
    },
    NULL);

  free(cxt);
}

static void
done_get_channel(struct discord *ceebot, struct discord_async_ret *ret)
{
  struct ceebot_primitives *primitives = discord_get_data(ceebot);
  const struct discord_channel *channel = ret->ret;
  struct context *cxt = ret->data;

  if (!is_user_rubberduck_channel(channel, primitives->category_id,
                                  cxt->user_id))
  {
    discord_async_next(ceebot, NULL);
    discord_edit_original_interaction_response(
      ceebot, cxt->application_id, cxt->token,
      &(struct discord_edit_original_interaction_response_params){
        .content = "Couldn't complete operation. Make sure to use "
                   "`/mycommand` from your channel" },
      NULL);

    free(cxt);
    return;
  }

  cxt->channel_id = channel->id;

  /* edit user channel */
  discord_async_next(ceebot, &(struct discord_async_attr){
                               .done = &done_edit_permissions,
                               .fail = &fail_edit_permissions,
                               .data = cxt,
                             });
  discord_edit_channel_permissions(
    ceebot, channel->id, primitives->roles.watcher_id,
    &(struct discord_edit_channel_permissions_params){
      .type = 0,
      .allow = cxt->priv ? 0 : PERMS_READ,
    });
}

void
react_rubberduck_channel_configure(
  struct discord *ceebot,
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

  struct context *cxt = malloc(sizeof *cxt);
  cxt->user_id = member->user->id;
  cxt->application_id = interaction->application_id;
  snprintf(cxt->token, sizeof(cxt->token), "%s", interaction->token);
  cxt->priv = (0 == strcmp(visibility, "private"));

  discord_async_next(ceebot, &(struct discord_async_attr){
                               .done = &done_get_channel,
                               .fail = &fail_edit_permissions,
                               .data = cxt
                             });
  discord_get_channel(ceebot, interaction->channel_id, NULL);

  params->type =
    DISCORD_INTERACTION_CALLBACK_DEFERRED_CHANNEL_MESSAGE_WITH_SOURCE;
}
