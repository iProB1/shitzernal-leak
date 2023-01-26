#include "ENetClient2.h"
#include "BotEvents.h"
#include "Botutils.hpp"
#include <ostream>
#include <fstream>
#include <iostream>
#include <proton/Variant.h>

ENetClient2* g_botClient = NULL;

void GTEvents::ProcessTankUpdatePacket(ENetPeer* peer, void* rawData)
{
	//printf("ProcessTankUpdatePacket, received packet with type: %d\n", raw->header.packetType);


	GameUpdatePacket* raw = (GameUpdatePacket*)rawData;

	switch (raw->type)
	{
	case PACKET_STATE:
		break;
	case PACKET_CALL_FUNCTION:
	{

	}
	case PACKET_TILE_APPLY_DAMAGE:
	{

		break;
	}
	case PACKET_SEND_MAP_DATA:
	{
		break;
	}
	default:
		//LOGC << "Unknown packet type: " << (int)raw->header.packetType << ENDL;
		break;
	}
}

void GTEvents::ProcessServerHello(ENetPeer* peer)
{
	//LOG("Processing server-hello, will send login data as response...");

	if (!peer->data) return;

	ClientData* cData = peer_info(peer);
	cData->winkey = utils::rnd(32);

	std::string logonStr = utils::CreateLogonPacket(peer, peer->data);
	//LOGI("Responding to server hello, logon detail: %s", logonStr.c_str());
	utils::SendPacket(peer, logonStr, 2);
}

void GTEvents::ProcessTextPacket(ENetPeer* peer, std::string text, bool isGame)
{
	// Message Type 3 is dedicated to slightly different actions than normal text packet iirc.
	int msgType = isGame ? 3 : 2;

#ifdef ISDEBUGBUILD
	LOGI("[?] Text is here: %s", text.c_str());
#endif
	//LOGI("[?] Text is here: %s", text.c_str());
	//LOGC << "[?] Text is here: " << text << ENDL;

	if (isGame) {

	}
	else {

	}
}
