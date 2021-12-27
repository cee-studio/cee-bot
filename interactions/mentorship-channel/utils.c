#include <orca/discord.h>

#include "interactions.h"

bool
is_user_channel(const struct discord_channel *channel,
                u64_snowflake_t mentorship_category_id,
                u64_snowflake_t user_id)
{
  if (channel->permission_overwrites)
    for (int i = 0; channel->permission_overwrites[i]; ++i)
      if (user_id == channel->permission_overwrites[i]->id
          && mentorship_category_id == channel->parent_id)
      {
        return true;
      }

  return false;
}
