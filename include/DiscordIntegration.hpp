#pragma once
#include <discord_rpc.h>


namespace Discord {
    void HandleReady(const DiscordUser* user);
    void HandleDisconnected(int err, const char* msg);
    void HandleError(int err, const char* msg);
    void HandleJoin(const char* secret);
    void HandleSpectate(const char* secret);
    void HandleJoinRequest(const DiscordUser* request);
    
    extern DiscordRichPresence RichPresence;
};