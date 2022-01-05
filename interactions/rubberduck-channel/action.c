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
  /** the interaction token */
  char token[256];
  /** the user to be muted or unmuted */
  u64_snowflake_t target_id;
  /**
   * permissions to be denied from user
   *  - PERMS_WRITE: User will have his write access to channel revoked
   *  - 0: User won't be denied write access
   */
  u64_bitmask_t perms;
};

static void
done_edit_permissions(struct discord *ceebot, struct discord_async_ret *ret)
{
  struct context *cxt = ret->data;
  char diagnosis[256];

  snprintf(diagnosis, sizeof(diagnosis),
           "Completed action targeted to <@!%" PRIu64 ">.", cxt->target_id);

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
           "Failed action targeted to <@!%" PRIu64 ">", cxt->target_id);

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
                   "`/mycommand` from your channel",
      },
      NULL);

    free(cxt);
    return;
  }

  discord_async_next(ceebot, &(struct discord_async_attr){
                               .done = &done_edit_permissions,
                               .fail = &fail_edit_permissions,
                               .data = cxt,
                             });
  discord_edit_channel_permissions(
    ceebot, channel->id, cxt->target_id,
    &(struct discord_edit_channel_permissions_params){
      .type = 1,
      .deny = cxt->perms,
    });
}

static u64_snowflake_t
get_unmute_target(
  struct discord_guild_member *member,
  struct discord_application_command_interaction_data_option **options)
{
  u64_snowflake_t target_id = 0ULL;

  if (options)
    for (int i = 0; options[i]; ++i) {
      char *name = options[i]->name;
      char *value = options[i]->value;

      if (0 == strcmp(name, "user")) target_id = strtoull(value, NULL, 10);
    }

  /* TODO: post to a logging channel */
  log_info("Attempt to unmute user(%" PRIu64 ") by %s#%s", target_id,
           member->user->username, member->user->discriminator);

  return target_id;
}

static u64_snowflake_t
get_mute_target(
  struct discord_guild_member *member,
  struct discord_application_command_interaction_data_option **options)
{
  u64_snowflake_t target_id = 0ULL;
  char *reason = NULL;

  if (options)
    for (int i = 0; options[i]; ++i) {
      char *name = options[i]->name;
      char *value = options[i]->value;

      if (0 == strcmp(name, "user"))
        target_id = strtoull(value, NULL, 10);
      else if (0 == strcmp(name, "reason"))
        reason = value;
    }

  /* TODO: post to a logging channel */
  log_info("Attempt to mute user(%" PRIu64 ") by %s#%s (%s)", target_id,
           member->user->username, member->user->discriminator, reason);

  return target_id;
}

void
react_rubberduck_channel_action(
  struct discord *ceebot,
  struct discord_interaction_response *params,
  const struct discord_interaction *interaction,
  struct discord_application_command_interaction_data_option **options)
{
  struct discord_guild_member *member = interaction->member;

  u64_bitmask_t perms;
  u64_snowflake_t target_id = 0ULL;

  if (options)
    for (int i = 0; options[i]; ++i) {
      char *name = options[i]->name;

      if (0 == strcmp(name, "mute")) {
        target_id = get_mute_target(member, options[i]->options);
        perms = PERMS_WRITE;
      }
      else if (0 == strcmp(name, "unmute")) {
        target_id = get_unmute_target(member, options[i]->options);
        perms = 0;
      }
    }

  if (target_id == member->user->id) {
    params->data->content = "You can't mute yourself!";
    return;
  }

  struct context *cxt = malloc(sizeof *cxt);
  *cxt = (struct context){
    .user_id = member->user->id,
    .application_id = interaction->application_id,
    .target_id = target_id,
    .perms = perms,
  };
  snprintf(cxt->token, sizeof(cxt->token), "%s", interaction->token);

  discord_async_next(ceebot, &(struct discord_async_attr){
                               .done = done_get_channel,
                               .fail = fail_edit_permissions,
                               .data = cxt,
                             });
  discord_get_channel(ceebot, interaction->channel_id, NULL);

  params->type =
    DISCORD_INTERACTION_CALLBACK_DEFERRED_CHANNEL_MESSAGE_WITH_SOURCE;
}
