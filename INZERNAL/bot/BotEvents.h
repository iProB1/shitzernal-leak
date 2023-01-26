#pragma once
#include "ENetClient2.h"
#include <proton/Variant.h>
#include "BotVarl.h"


extern ENetClient2* g_botClient; // nah lets use it as global link

namespace GTEvents {
	void ProcessTankUpdatePacket(ENetPeer* peer, void* rawData);
	void ProcessServerHello(ENetPeer* peer); // Login
	void ProcessTextPacket(ENetPeer* peer, std::string text, bool isGame = false); // isGame = if m type is 3
	void ProcessTrackPacket(ENetPeer* peer, std::string track);
}