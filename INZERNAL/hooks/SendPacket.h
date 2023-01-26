#pragma once
#include <core/globals.h>
#include <core/gt.h>
#include <proton\MiscUtils.h>
#include <sdk/sdk.h>
#include <proton/RTParam.hpp>
#include <string>
#include <menu/imgui/imgui.h>



static bool dropwl = false;


static bool find_command(std::string chat, std::string name) {
    bool found = chat.find("/" + name) == 0;
    if (found)
        gt::sendlog("`6" + chat);
    return found;
}

class SendPacketHook {
public:
    static void Execute(int type, std::string& packet, ENetPeer* peer) {
        static auto orig = decltype(&hooks::SendPacket)(hookmgr->orig(sig::sendpacket));
        if (type > NET_MESSAGE_GAME_PACKET) //dont send any tracking packets, crash logs, or other shit to gt.
            return;



        if (packet.find("game_version|") != -1 && opt::spoof_login) {
            RTVar var = RTVar::parse(packet);
            auto mac = gt::generate_mac();
            auto hash_str = mac + "RT";
            auto hash2 = HashString(hash_str.c_str(), hash_str.length());
            var.set("mac", mac);
            var.set("wk", gt::generate_rid());
            var.set("rid", gt::generate_rid());
            var.set("fz", std::to_string(utils::random(INT_MIN, INT_MAX)));
            var.set("zf", std::to_string(utils::random(INT_MIN, INT_MAX)));
            var.set("hash", std::to_string(utils::random(INT_MIN, INT_MAX)));
            var.set("hash2", std::to_string(hash2));
            var.set("country", opt::flag);
            var.set("player_age", "20");
            if (var.find("tankIDName")) {
                opt::cheat::name = var.find("tankIDName")->m_values[0];
            }
            if (var.find("tankIDPass")) {
                opt::cheat::password = var.find("tankIDPass")->m_values[0];
            }
            if (opt::cheat::random_flag) {
                var.set("country", gt::get_random_flag());
            }
            if (opt::spoof_name)
                var.set("requestedName", utils::rnd(utils::random(4, 10)));

            packet = var.serialize();
            printf("Spoofing login info\n");
        }
        if (packet.find("buttonClicked|gfloat") != -1) {
            std::string droppedshit = "add_spacer|small|\nadd_label_with_icon_button_list|small|`w%s : %s|left|findObject_|itemID_itemAmount|";
            auto logic = sdk::GetGameLogic();
            auto objects = logic->GetObjectMap();

            int itemid;
            int count = 0;
            std::vector<pair<int, int>> ditems;
            std::string itemlist = "";
            for (auto object = objects->objects.begin(); object != objects->objects.end(); ++object)
            {

                bool copy = true;
                for (int i = 0; i < ditems.size(); i++)
                {
                    if (ditems.at(i).first == object->item_id)
                    {
                        ditems.at(i).second += object->amount;
                        copy = false;
                    }
                }
                if (copy)
                    ditems.push_back(make_pair(object->item_id, object->amount));
            }

            for (int i = 0; i < ditems.size(); i++)
            {
                itemlist = itemlist + std::to_string(ditems.at(i).first) + "," + std::to_string(ditems.at(i).second) + ",";
            }
            std::string paket = "set_default_color|`o\nadd_label_with_icon|big|`wFloating Items``|left|6016|\n" + droppedshit + itemlist.substr(0, itemlist.size() - 1) + "\nadd_quick_exit\nend_dialog|dropped||Back|\n";
            variantlist_t liste{ "OnDialogRequest" };
            liste[1] = paket;
            gt::send_varlist_self(liste);
            return;
        }
        if (opt::cheat::GetNetid == true)
        {
            if (packet.find("action|wrench") != -1) {
                opt::cheat::GetNetids = std::stoi(packet.substr(packet.find("netid|") + 6, packet.length() - packet.find("netid|") - 1));
            }
        }
        RTVar var = RTVar::parse(packet);
        if (!var.valid())
            return;

        if (var.get(0).m_key == "action" && var.get(0).m_value == "input") {
            if (var.size() < 2)
                return;
            if (var.get(1).m_values.size() < 2)
                return;
            auto chat = var.get(1).m_values[1];

        }
        if (logging::enabled && logging::console & logging::sendpacket)
            printf("[sendpacket] type: %d\n%s\n", type, packet.c_str());
        printf("[sendpacket] type: %d\n%s\n", type, packet.c_str());

        orig(type, packet, peer);
    }
};