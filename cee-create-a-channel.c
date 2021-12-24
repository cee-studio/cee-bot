/* see https://discordapi.com/permissions.html#377957256256 */
#define PERMS_DEFAULT 1377957256256
#define PERMS_ALL     (enum discord_bitwise_permission_flags) - 1

void
on_mentorship_channel_create(struct discord *client,
                             struct discord_async_ret *ret)
{
  struct context *cxt = discord_get_data(client);
  char welcome_msg[DISCORD_MAX_MESSAGE_LEN] = "";

  const struct discord_channel *channel = ret->ret;
  u64_snowflake_t *user_id = ret->data;

  /* assign mentorship role to user */
  discord_async_next(client, NULL);
  discord_add_guild_member_role(client, cxt->guild_id, *user_id,
                                cxt->roles.mentorship_id);

  snprintf(welcome_msg, sizeof(welcome_msg),
           "Welcome <@!%" PRIu64 ">, "
           "I hope you enjoy your new space! Check the available commands by "
           "typing `/`, and check how you can further customize it.",
           *user_id);

  discord_async_next(client, NULL);
  discord_create_message(client, channel->id,
                         &(struct discord_create_message_params){
                           .content = welcome_msg,
                         },
                         NULL);
}

void
cee_create_a_channel(struct discord *client,
                     struct discord_interaction_response *params,
                     const struct discord_interaction *interaction)
{
  struct context *cxt = discord_get_data(client);

  struct discord_guild_member *member = interaction->member;
  bool priv = false;

  /* skip user with already assigned channel */
  if (member->roles)
    for (int i = 0; member->roles[i]; ++i) {
      u64_snowflake_t role_id = member->roles[i]->value;

      if (role_id == cxt->roles.mentorship_id) {
        params->data->content =
          "It seems you already have a channel, please edit it from within";
        return;
      }
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
  discord_async_next(client, &(struct discord_async_attr){
                               .done = &on_mentorship_channel_create,
                               .data = user_id,
                               .cleanup = &free });
  discord_create_guild_channel(
    client, cxt->guild_id,
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
            .id = cxt->roles.helper_id,
            .type = 0,
            .allow = PERMS_DEFAULT,
          },
          /* hide it from @lurker only if 'priv' has been set */
          &(struct discord_overwrite){
            .id = cxt->roles.lurker_id,
            .type = 0,
            .allow = priv ? 0 : PERMS_DEFAULT,
          },
          /* hide it from @everyone */
          &(struct discord_overwrite){
            .id = cxt->guild_id,
            .type = 0,
            .deny = PERMS_ALL,
          },
          NULL, /* END OF OVERWRITE LIST */
        },
      .parent_id = cxt->category_id,
    },
    NULL);

  params->data->content = "Your channel will be created shortly";
}

#undef PERMS_DEFAULT
#undef PERMS_ALL
