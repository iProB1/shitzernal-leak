#pragma once
#include <menu/menu.h>
#include <sdk/sdk.h>
#include <hooks/SendPacket.h>
#include <hooks/SendPacketRaw.h>

void menu::playerpog() {
    if (ImGui::Button("Ban All")) {
        for (auto pair : sdk::GetGameLogic()->GetNetObjMgr()->players) {
            std::string name = pair.second->name;
            std::string name2 = name.substr(2);
            utils::replace(name2, "``", "");
            std::string packet = "action|input\n|text|/ban " + name2;
            SendPacketHook::Execute(2, packet, sdk::GetPeer());
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Pull All")) {
        for (auto pair : sdk::GetGameLogic()->GetNetObjMgr()->players) {
            std::string name = pair.second->name;
            std::string name2 = name.substr(2);
            utils::replace(name2, "``", "");
            std::string packet = "action|input\n|text|/pull " + name2;
            SendPacketHook::Execute(2, packet, sdk::GetPeer());
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Kick All")) {
        for (auto pair : sdk::GetGameLogic()->GetNetObjMgr()->players) {
            std::string name = pair.second->name;
            std::string name2 = name.substr(2);
            utils::replace(name2, "``", "");
            std::string packet = "action|input\n|text|/kick " + name2;
            SendPacketHook::Execute(2, packet, sdk::GetPeer());
        }
    }
    ImGui::Text("Player List");
    if (sdk::GetGameLogic()->GetNetObjMgr()) {
        for (auto pair : sdk::GetGameLogic()->GetNetObjMgr()->players) {
            std::string name = pair.second->name;
            std::string name2 = name.substr(2);
            utils::replace(name2, "``", "");
            ImGui::Text("NetID: %d | Name: %s\n", pair.first, name2);
        }
    }
}
