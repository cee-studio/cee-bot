#ifndef INTERACTIONS_H
#define INTERACTIONS_H

/* see https://discordapi.com/permissions.html#377957256256 */
#define PERMS_DEFAULT 377957256256
#define PERMS_ALL     (enum discord_bitwise_permission_flags) - 1

/** @brief The client environment to work with */
struct client_context {
  /** the guild our client will react to */
  u64_snowflake_t guild_id;
  /** the mentorship channels category id */
  u64_snowflake_t category_id;
  struct {
    /** role for users that own a mentorship channel */
    u64_snowflake_t mentorship_id;
    /**
     * role for users that have read/write access to public and private
     *        mentorship channels
     */
    u64_snowflake_t helper_id;
    /**
     * role for users that have read/write access to public mentorship channels
     */
    u64_snowflake_t lurker_id;
  } roles;
};

/** @brief Per-request context storing for async functions */
struct async_context {
  /** the user that triggered the interaction */
  u64_snowflake_t user_id;
  /** the client's application id */
  u64_snowflake_t application_id;
  /** the interaction token */
  char token[256];
  /** arbitrary data */
  void *data;
};

/**
 * @brief React to mentorship channel selection menu
 *
 * @param client the bot client
 * @param params the interaction response to be sent at `main.c`
 * @param interaction the interaction object received
 */
void react_mentorship_channel_menu(
  struct discord *client,
  struct discord_interaction_response *params,
  const struct discord_interaction *interaction);

/**
 * @brief React to mentorship channel 'delete' command
 *
 * @param client the bot client
 * @param params the interaction response to be sent at `main.c`
 * @param interaction the interaction object received
 * @param options the options selected by user
 */
void react_mentorship_channel_delete(
  struct discord *client,
  struct discord_interaction_response *params,
  const struct discord_interaction *interaction,
  struct discord_application_command_interaction_data_option **options);

/**
 * @brief React to mentorship channel configure
 *
 * @param client the bot client
 * @param params the interaction response to be sent at `main.c`
 * @param interaction the interaction object received
 * @param options the options selected by user
 */
void react_mentorship_channel_configure(
  struct discord *client,
  struct discord_interaction_response *params,
  const struct discord_interaction *interaction,
  struct discord_application_command_interaction_data_option **options);

/******************************************************************************
 * Utility functions
 ******************************************************************************/

/**
 * @brief Check if channel is owned by user
 *
 * @param the channel to be checked against
 * @param mentorship_category_id the mentorship channels category id
 * @param user_id user to be checked for ownership
 * @return `true` if channel belongs to user
 */
bool is_user_channel(const struct discord_channel *channel,
                     u64_snowflake_t mentorship_category_id,
                     u64_snowflake_t user_id);

#endif /* INTERACTIONS_H */
