#ifndef ENDIAN_BOT_H_
#define ENDIAN_BOT_H_

#include <concord/discord.h>

struct bot {
  struct discord* discord_bot;
};

#ifdef ENDIAN_ENGINE

void bot_init();
void bot_cleanup();

void bot_start();

#endif

struct bot* bot_get_global();

#endif
