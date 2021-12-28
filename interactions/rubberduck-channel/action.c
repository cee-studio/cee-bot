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
  /** the user to be muted or unmuted */
  u64_snowflake_t target_id;
  /** the interaction token */
  char token[256];
};

static void
rubberduck_channel_unmute(struct discord *ceebot,
                          struct discord_async_ret *ret)
{
  char diagnosis[256];
  struct discord_edit_original_interaction_response_params params = {
    .content = diagnosis
  };

  struct ceebot_primitives *primitives = discord_get_data(ceebot);
  const struct discord_channel *channel = ret->ret;
  struct context *cxt = ret->data;

  if (!is_user_rubberduck_channel(channel, primitives->category_id,
                                  cxt->user_id))
  {
    snprintf(diagnosis, sizeof(diagnosis),
             "Couldn't complete operation. Make sure to use command from your "
             "channel.");
  }
  else {
    /* allow write permission for user */
    discord_async_next(ceebot, NULL);
    discord_edit_channel_permissions(
      ceebot, channel->id, cxt->target_id,
      &(struct discord_edit_channel_permissions_params){
        .type = 1,
        .deny = 0,
      });

    snprintf(diagnosis, sizeof(diagnosis),
             "<@!%" PRIu64 "> has been unmuted from channel '%s'.",
             cxt->target_id, channel->name);
  }

  discord_async_next(ceebot, NULL);
  discord_edit_original_interaction_response(ceebot, cxt->application_id,
                                             cxt->token, &params, NULL);
}

static void
rubberduck_channel_action_unmute(
  const struct discord_interaction *interaction,
  struct discord_application_command_interaction_data_option **options,
  struct discord_async_attr *attr)
{
  struct discord_guild_member *member = interaction->member;
  u64_snowflake_t target_id = 0ULL;
  char *reason = NULL;

  if (options)
    for (int i = 0; options[i]; ++i) {
      char *name = options[i]->name;
      char *value = options[i]->value;

      if (0 == strcmp(name, "user")) target_id = strtoull(value, NULL, 10);
    }

  /* TODO: post to a logging channel */
  log_info("User ID %" PRIu64 " unmuted by %s#%s (%s)", target_id,
           member->user->username, member->user->discriminator, reason);

  struct context *cxt = malloc(sizeof *cxt);
  *cxt = (struct context){
    .user_id = member->user->id,
    .application_id = interaction->application_id,
    .target_id = target_id,
  };
  snprintf(cxt->token, sizeof(cxt->token), "%s", interaction->token);

  *attr = (struct discord_async_attr){
    .done = &rubberduck_channel_unmute,
    .data = cxt,
    .cleanup = &free,
  };
}

static void
rubberduck_channel_mute(struct discord *ceebot, struct discord_async_ret *ret)
{
  char diagnosis[256];
  struct discord_edit_original_interaction_response_params params = {
    .content = diagnosis
  };

  struct ceebot_primitives *primitives = discord_get_data(ceebot);
  const struct discord_channel *channel = ret->ret;
  struct context *cxt = ret->data;

  if (!is_user_rubberduck_channel(channel, primitives->category_id,
                                  cxt->user_id))
  {
    snprintf(diagnosis, sizeof(diagnosis),
             "Couldn't complete operation. Make sure to use command from your "
             "channel.");
  }
  else {
    /* remove write permission from user */
    discord_async_next(ceebot, NULL);
    discord_edit_channel_permissions(
      ceebot, channel->id, cxt->target_id,
      &(struct discord_edit_channel_permissions_params){
        .type = 1,
        .deny = PERMS_WRITE,
      });

    snprintf(diagnosis, sizeof(diagnosis),
             "<@!%" PRIu64 "> has been muted from channel '%s'.",
             cxt->target_id, channel->name);
  }

  discord_async_next(ceebot, NULL);
  discord_edit_original_interaction_response(ceebot, cxt->application_id,
                                             cxt->token, &params, NULL);
}

static void
rubberduck_channel_action_mute(
  const struct discord_interaction *interaction,
  struct discord_application_command_interaction_data_option **options,
  struct discord_async_attr *attr)
{
  struct discord_guild_member *member = interaction->member;
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
  log_info("User ID %" PRIu64 " muted by %s#%s (%s)", target_id,
           member->user->username, member->user->discriminator, reason);

  struct context *cxt = malloc(sizeof *cxt);
  *cxt = (struct context){
    .user_id = member->user->id,
    .application_id = interaction->application_id,
    .target_id = target_id,
  };
  snprintf(cxt->token, sizeof(cxt->token), "%s", interaction->token);

  *attr = (struct discord_async_attr){
    .done = &rubberduck_channel_mute,
    .data = cxt,
    .cleanup = &free,
  };
}

void
react_rubberduck_channel_action(
  struct discord *ceebot,
  struct discord_interaction_response *params,
  const struct discord_interaction *interaction,
  struct discord_application_command_interaction_data_option **options)
{
  struct discord_async_attr attr = { 0 };

  if (options)
    for (int i = 0; options[i]; ++i) {
      char *name = options[i]->name;

      if (0 == strcmp(name, "mute"))
        rubberduck_channel_action_mute(interaction, options[i]->options,
                                       &attr);
      else if (0 == strcmp(name, "unmute"))
        rubberduck_channel_action_unmute(interaction, options[i]->options,
                                         &attr);
    }

  discord_async_next(ceebot, &attr);
  discord_get_channel(ceebot, interaction->channel_id, NULL);

  params->type =
    DISCORD_INTERACTION_CALLBACK_DEFERRED_CHANNEL_MESSAGE_WITH_SOURCE;
}
