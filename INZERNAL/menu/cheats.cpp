#pragma once
#include <menu/menu.h>
#include <sdk/Consts.h>
#include <sdk/ResourceManager.h>
#include <sdk/sdk.h>
#include <hooks/SendPacket.h>
#include <hooks/SendPacketRaw.h>
#include "sdk/world/WorldCamera.h"
#include <d3d/DrawingLUL.h>
#include "core/globals.h"
#include "proton/Math/rtRect.h"
#include <hooks/ProcessTankUpdatePacket.h>
#include "items.h"
#include <Keyauth/Requests.hpp>



void log(std::string msg) {
    variantlist_t var{ "OnConsoleMessage" };
    var[1] = "`0[`^INZERNAL`0] " + msg;
    gt::send_varlist_self(var, -1, 0, true);

}

void menu::cheats_tab() {
    auto logic = sdk::GetGameLogic();
    auto local = sdk::GetGameLogic()->GetLocalPlayer();
    auto locals = sdk::GetGameLogic()->GetNetObjMgr();
    auto tilemap = logic->GetTileMap();
    auto object = logic->GetObjectMap();
    auto w2s = logic->GetWorldRenderer()->camera;
    auto pos2f = local->GetPos();
    auto draw = ImGui::GetBackgroundDrawList();
    if (local) {


    }
}