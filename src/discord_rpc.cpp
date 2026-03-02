#include "discord_rpc.h"

#if defined(REWOR_HAS_DISCORD_RPC)
#include <cstdint>
#include <ctime>

extern "C" {

struct DiscordUser {
  const char* userId;
  const char* username;
  const char* discriminator;
  const char* avatar;
};

struct DiscordEventHandlers {
  void (*ready)(const DiscordUser* request);
  void (*disconnected)(int errorCode, const char* message);
  void (*errored)(int errorCode, const char* message);
  void (*joinGame)(const char* joinSecret);
  void (*spectateGame)(const char* spectateSecret);
  void (*joinRequest)(const DiscordUser* request);
};

struct DiscordRichPresence {
  const char* state;
  const char* details;
  std::int64_t startTimestamp;
  std::int64_t endTimestamp;
  const char* largeImageKey;
  const char* largeImageText;
  const char* smallImageKey;
  const char* smallImageText;
  const char* partyId;
  int partySize;
  int partyMax;
  int partyPrivacy;
  const char* matchSecret;
  const char* joinSecret;
  const char* spectateSecret;
  std::int8_t instance;
};

void Discord_Initialize(const char* applicationId,
                        DiscordEventHandlers* handlers,
                        int autoRegister,
                        const char* optionalSteamId);
void Discord_Shutdown(void);
void Discord_RunCallbacks(void);
void Discord_UpdatePresence(const DiscordRichPresence* presence);

} // extern "C"
#endif

namespace {

constexpr const char* kDiscordApplicationId = "1477849970194251979"; // Replace with your Discord Application ID from discord.com/developers.

#if defined(REWOR_HAS_DISCORD_RPC)
bool g_discord_initialized = false;

void handle_ready(const DiscordUser*) {}
void handle_disconnected(int, const char*) {}
void handle_error(int, const char*) {}
#endif

} // namespace

void discord_init() {
#if defined(REWOR_HAS_DISCORD_RPC)
  if (g_discord_initialized) {
    return;
  }

  DiscordEventHandlers handlers{};
  handlers.ready = &handle_ready;
  handlers.disconnected = &handle_disconnected;
  handlers.errored = &handle_error;

  Discord_Initialize(kDiscordApplicationId, &handlers, 1, nullptr);

  DiscordRichPresence presence{};
  presence.state = "Warriors of Rock";
  presence.details = "Playing Guitar Hero";
  presence.largeImageKey = "game_icon";
  presence.largeImageText = "Guitar Hero: Warriors of Rock";
  presence.startTimestamp = static_cast<std::int64_t>(std::time(nullptr));

  Discord_UpdatePresence(&presence);
  g_discord_initialized = true;
#else
  (void)kDiscordApplicationId;
#endif
}

void discord_update() {
#if defined(REWOR_HAS_DISCORD_RPC)
  if (g_discord_initialized) {
    Discord_RunCallbacks();
  }
#endif
}

void discord_shutdown() {
#if defined(REWOR_HAS_DISCORD_RPC)
  if (!g_discord_initialized) {
    return;
  }

  Discord_Shutdown();
  g_discord_initialized = false;
#endif
}
