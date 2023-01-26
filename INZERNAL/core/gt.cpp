#pragma once
#include <core/gt.h>
#include <hooks/ProcessTankUpdatePacket.h>
#include <hooks/SendPacket.h>
#include <hooks/SendPacketRaw.h>
#include <intrin.h>
#include <sdk/GameUpdatePacket.h>
#include <sdk/sdk.h>
#include <algorithm>
#include <PathFinder/Astar.hpp>
#include <filesystem>
#include <core/json.hpp>
#include <iostream>
#include <fstream>
#include <string>


using namespace nlohmann;
std::string gt::generate_mac(const std::string& prefix) {
    std::string x = prefix + ":";
    for (int i = 0; i < 5; i++) {
        x += utils::hex_str(utils::random(0, 255));
        if (i != 4)
            x += ":";
    }
    return x;
}

std::string gt::generate_rid() {
    std::string rid_str;

    for (int i = 0; i < 16; i++)
        rid_str += utils::hex_str(utils::random(0, 255));

    std::transform(rid_str.begin(), rid_str.end(), rid_str.begin(), std::toupper);

    return rid_str;
}

std::string gt::generate_meta() {
    return utils::rnd(utils::random(5, 10)) + ".com";
}

std::string gt::get_random_flag() {
    static bool done = false;
    static std::vector<string> candidates{};

    if (!done) {
        CHAR NPath[MAX_PATH];
        GetCurrentDirectoryA(MAX_PATH, NPath);
        std::string pattern(string(NPath) + "\\interface\\flags\\*.rttex");
        _WIN32_FIND_DATAA data{};
        HANDLE hFind;
        if ((hFind = FindFirstFileA(pattern.c_str(), &data)) != INVALID_HANDLE_VALUE) {
            do {
                auto temp = string(data.cFileName);
                if (utils::replace(temp, ".rttex", ""))
                    if (temp.length() == 2)
                        candidates.push_back(temp);

            } while (FindNextFileA(hFind, &data) != 0);
            FindClose(hFind);
        }
        done = true;
    }

    return candidates[utils::random(0, (int)candidates.size())];
}

std::string gt::get_type_string(uint8_t type) {
    static const char* types[]{ "PACKET_STATE", "PACKET_CALL_FUNCTION", "PACKET_UPDATE_STATUS", "PACKET_TILE_CHANGE_REQUEST", "PACKET_SEND_MAP_DATA",
        "PACKET_SEND_TILE_UPDATE_DATA", "PACKET_SEND_TILE_UPDATE_DATA_MULTIPLE", "PACKET_TILE_ACTIVATE_REQUEST", "PACKET_TILE_APPLY_DAMAGE",
        "PACKET_SEND_INVENTORY_STATE", "PACKET_ITEM_ACTIVATE_REQUEST", "PACKET_ITEM_ACTIVATE_OBJECT_REQUEST", "PACKET_SEND_TILE_TREE_STATE",
        "PACKET_MODIFY_ITEM_INVENTORY", "PACKET_ITEM_CHANGE_OBJECT", "PACKET_SEND_LOCK", "PACKET_SEND_ITEM_DATABASE_DATA", "PACKET_SEND_PARTICLE_EFFECT",
        "PACKET_SET_ICON_STATE", "PACKET_ITEM_EFFECT", "PACKET_SET_CHARACTER_STATE", "PACKET_PING_REPLY", "PACKET_PING_REQUEST", "PACKET_GOT_PUNCHED",
        "PACKET_APP_CHECK_RESPONSE", "PACKET_APP_INTEGRITY_FAIL", "PACKET_DISCONNECT", "PACKET_BATTLE_JOIN", "PACKET_BATTLE_EVENT", "PACKET_USE_DOOR",
        "PACKET_SEND_PARENTAL", "PACKET_GONE_FISHIN", "PACKET_STEAM", "PACKET_PET_BATTLE", "PACKET_NPC", "PACKET_SPECIAL", "PACKET_SEND_PARTICLE_EFFECT_V2",
        "PACKET_ACTIVE_ARROW_TO_ITEM", "PACKET_SELECT_TILE_INDEX", "PACKET_SEND_PLAYER_TRIBUTE_DATA", "PACKET_SET_EXTRA_MODS", "PACKET_ON_STEP_ON_TILE_MOD",
        "PACKET_ERRORTYPE" };

    if (type >= PACKET_MAXVAL)
        type = PACKET_MAXVAL - 1; //will set any unknown type as errortype and keep us from crashing

    if (type > 43)
        return "PACKET_FIXMELATER";

    return types[type];
}
#define WORDn(x, n) (*((WORD*)&(x) + n))
int16_t gt::get_cpuid() {
    int32_t regs[4];
    __cpuid((int*)regs, 0);

    return WORDn(regs[0], 1) + WORDn(regs[1], 1) + WORDn(regs[2], 1) + WORDn(regs[3], 1) + regs[0] + regs[1] + regs[2] + regs[3];
}

int gt::decrypt_piece(uint8_t* data, uint32_t size, int seed) {
    auto seed_mod = seed;
    char unk = -2 - seed;

    int smth = 0;
    int smth2 = 0;
    do {
        int temp1 = smth;
        int temp2 = *data;
        smth2 = temp1 + temp2;
        *data = unk + temp2;
        --unk;
        ++data;
        smth = seed_mod + smth2;
        ++seed_mod;
        --size;
    } while (size);

    return seed_mod + smth2 - 1;
}

//TLDR gt stores badly encrypted cached versions of hash, wk, mac into registry and they take priority over real one
//this is why unban used to require u to delete the keys too.
//stealers getting the mac from here would be ideal to use for save decryption, since real mac used by gt cant be known
//and since gt uses mac to encrypt pass (see my save decrypter for decrypter without bruteforce)
void gt::decrypt_reg_vals() {
    //did this while bored and it was too easy
    uint32_t uint3 = (uint16_t)gt::get_cpuid() + 1;
    LPCSTR key = ("Software\\Microsoft\\" + std::to_string(uint3)).c_str();
    BYTE data[1024];
    DWORD data_len = 1024;
    memset(data, 0, 1024);
    if (!utils::read_reg_value(key, (std::to_string((uint3 >> 1))).c_str(), data, &data_len)) {
        printf("failed at reading reg value %s!\n", key);
        return;
    }


    auto hashc = gt::decrypt_piece(data, data_len - 1, 25532);
    auto hash = utils::format("%s", data);
    auto hash_int = atoi((char*)data);
    memset(data, 0, 1024);
    if (!utils::read_reg_value(key, (std::to_string((uint3 >> 1)) + "c").c_str(), data, &data_len)) {
        printf("failed at reading reg value %s!\n", key);
        return;
    }
    auto hashc2 = atol((const char*)data);
    if (!utils::read_reg_value(key, (std::to_string((uint3 >> 1)) + "w").c_str(), data, &data_len)) {
        printf("failed at reading reg value %s!\n", key);
        return;
    }
    auto wkc = gt::decrypt_piece(data, data_len - 1, 25532);
    auto wk = utils::format("%s", data);
    memset(data, 0, 1024);
    if (!utils::read_reg_value(key, (std::to_string((uint3 >> 1)) + "wc").c_str(), data, &data_len)) {
        printf("failed at reading reg value %s!\n", key);
        return;
    }
    auto wkc2 = atol((const char*)data);
    memset(data, 0, 1024);
    auto keymac = std::to_string(abs(hash_int / 3));
    auto valmac = std::to_string(abs(hash_int / 4));

    if (!utils::read_reg_value(keymac.c_str(), valmac.c_str(), data, &data_len)) {
        printf("failed at reading reg value %s\n", keymac.c_str());
        return;
    }

    auto macc = gt::decrypt_piece(data, data_len - 1, 25532);
    auto mac = utils::format("%s", data);

    printf("hash: %s\nwk: %s\nmac: %s\n", hash.c_str(), wk.c_str(), mac.c_str());
    //got bored at this point and couldnt be bothered to read the c for mac too
}

void gt::set_extra_character_mods(NetAvatar* player, uint8_t flags) {
    static auto func = types::SetCharacterExtraMods(sigs::get(sig::setextracharactermods));

    func(player, &flags);
}

void gt::send(int type, std::string message, bool hook_send) {
    static auto func = types::SendPacket(sigs::get(sig::sendpacket));
    if (hook_send) //if we want to apply our own code or just log shit
        SendPacketHook::Execute(type, message, sdk::GetPeer());
    else
        func(type, message, sdk::GetPeer());
}

void gt::send(GameUpdatePacket* packet, int extra_size, bool hook_send) {
    static auto func = types::SendPacketRaw(sigs::get(sig::sendpacketraw));
    void* PacketSender = nullptr;
    if (hook_send) //if we want to apply our own code or just log shit
        SendPacketRawHook::Execute(NET_MESSAGE_GAME_PACKET, packet, 56 + extra_size, PacketSender, sdk::GetPeer(), ENET_PACKET_FLAG_RELIABLE);
    else
        func(NET_MESSAGE_GAME_PACKET, packet, 56 + extra_size, PacketSender, sdk::GetPeer(), ENET_PACKET_FLAG_RELIABLE);
}

void gt::send_self(GameUpdatePacket* packet, bool hook_send) {
    static auto func = types::ProcessTankUpdatePacket(sigs::get(sig::processtankupdatepacket));
    if (hook_send) //if we want to apply our own code or just log shit
        ProcessTankUpdatePacketHook::Execute(sdk::GetGameLogic(), packet);
    else
        func(sdk::GetGameLogic(), packet);
}

void gt::send_varlist_self(variantlist_t variantlist, int netid, int delay, bool hook_send) {
    uint32_t data_size = 0;
    void* data = variantlist.serialize_to_mem(&data_size, nullptr);

    auto packet = (GameUpdatePacket*)calloc(sizeof(GameUpdatePacket) + data_size, 1);
    packet->type = PACKET_CALL_FUNCTION;
    packet->netid = netid;
    packet->flags |= 8;
    packet->int_data = delay;
    packet->data_size = data_size;
    memcpy(&packet->data, data, data_size);

    static auto func = types::ProcessTankUpdatePacket(sigs::get(sig::processtankupdatepacket));
    if (hook_send) //if we want to apply our own code or just log shit
        ProcessTankUpdatePacketHook::Execute(sdk::GetGameLogic(), packet);
    else
        func(sdk::GetGameLogic(), packet);
    free(packet);
    free(data);
}

bool gt::patch_banbypass() {
    try {
        static auto banbypass = sigs::get(sig::banbypass);
        if (!banbypass) //did not find ban bypass
            throw std::runtime_error("could not find ban bypass");

        auto bypassed = utils::patch_bytes<2>(banbypass, "\x90\x90");
        if (!bypassed)
            throw std::runtime_error("could not patch ban bypass");
        else {
            printf("patched ban bypass\n");
            return true;
        }
    } catch (std::exception exception) {
        printf("exception thrown: %s\n", exception.what());
        utils::read_key();
        return false;
    }
    return false;
}



//#include <memory>
//#include <winternl.h>
//
//#pragma comment(lib, "ntdll")
//
//#define NT_SUCCESS(status) (status >= 0)
//
//#define STATUS_INFO_LENGTH_MISMATCH ((NTSTATUS)0xC0000004L)
//
//
//typedef struct _PROCESS_HANDLE_TABLE_ENTRY_INFO {
//    HANDLE HandleValue;
//    ULONG_PTR HandleCount;
//    ULONG_PTR PointerCount;
//    ULONG GrantedAccess;
//    ULONG ObjectTypeIndex;
//    ULONG HandleAttributes;
//    ULONG Reserved;
//} PROCESS_HANDLE_TABLE_ENTRY_INFO, *PPROCESS_HANDLE_TABLE_ENTRY_INFO;
//
//// private
//typedef struct _PROCESS_HANDLE_SNAPSHOT_INFORMATION {
//    ULONG_PTR NumberOfHandles;
//    ULONG_PTR Reserved;
//    PROCESS_HANDLE_TABLE_ENTRY_INFO Handles[1];
//} PROCESS_HANDLE_SNAPSHOT_INFORMATION, *PPROCESS_HANDLE_SNAPSHOT_INFORMATION;
//
//extern "C" NTSTATUS NTAPI NtQueryInformationProcess(_In_ HANDLE ProcessHandle, _In_ PROCESSINFOCLASS ProcessInformationClass,
//    _Out_writes_bytes_(ProcessInformationLength) PVOID ProcessInformation, _In_ ULONG ProcessInformationLength, _Out_opt_ PULONG ReturnLength);

bool gt::patch_mutex() {
    static auto mutex1 = sigs::get(sig::mutexbypass1);
    static auto mutex2 = sigs::get(sig::mutexbypass2);

    if (!mutex2 || !mutex1) {
        printf("If you're using a modified GT executable that supports multiboxing (not meaning patcher that comes with INZERNAL)\n"
            "Then just ignore this warning, although some things might be broken if the file size is different from original, otherwise should be fine.\n"
            "If that's not the case, you are probably using wrong gt version, or something went horribly wrong.\n");
        return true;
    }

    mutex2 -= 9; //Didnt bother fixing negative offsets in sig manager cuz I'm lazy, not a big deal (for now) at least

    //this checks for presence of gt's mutex 247, and will freeze if you delete them
    auto patched1 = utils::patch_bytes<6>(mutex1, "\x48\x83\xc4\x50\x5b\xc3"); //basic frame stuff and ret

    if (!patched1)
        return false;

    //the length will probably change with each GT version so I have to remember to change this
    //(no way im calculating the offset by adding another sig for where to jump etcetc) well prob will actually if I get bored
    utils::patch_bytes<2>(mutex2, "\xEB\x53"); 


    //fow now mutex bypass only works with patcher, since I was too lazy to add proper handle closing thats required for injector
    //will add it prob by V0.6

    printf("Patched mutex checks\n");
    return true;
}

void gt::hit_tile(CL_Vec2i where) {
    auto local = sdk::GetGameLogic()->GetLocalPlayer();
    if (!local)
        return;

    GameUpdatePacket packet{ 0 };
    packet.type = PACKET_TILE_CHANGE_REQUEST;
    packet.int_data = 18;
    packet.int_x = where.x;
    packet.int_y = where.y;
    auto pos = local->GetPos();
    packet.pos_x = pos.x;
    packet.pos_y = pos.y;
    SendPacketRawHook::Execute(4, &packet, 56, NULL, sdk::GetPeer(), ENET_PACKET_FLAG_RELIABLE);
}

void gt::place_tile(int id, CL_Vec2i where) {
    auto local = sdk::GetGameLogic()->GetLocalPlayer();
    if (!local)
        return;

    GameUpdatePacket packet{ 0 };
    packet.type = PACKET_TILE_CHANGE_REQUEST;
    packet.int_data = id;
    packet.int_x = where.x;
    packet.int_y = where.y;
    auto pos = local->GetPos();
    packet.pos_x = pos.x;
    packet.pos_y = pos.y;
    gt::send(&packet);
}

void gt::wrench_tile(CL_Vec2i where) {
    auto local = sdk::GetGameLogic()->GetLocalPlayer();
    if (!local)
        return;

    GameUpdatePacket packet{ 0 };
    packet.type = PACKET_TILE_CHANGE_REQUEST;
    packet.int_data = 32;
    packet.int_x = where.x;
    packet.int_y = where.y;
    auto pos = local->GetPos();
    packet.pos_x = pos.x;
    packet.pos_y = pos.y;
    gt::send(&packet);
}
void gt::water_tile(CL_Vec2i where) {
    auto local = sdk::GetGameLogic()->GetLocalPlayer();
    if (!local)
        return;

    GameUpdatePacket packet{ 0 };
    packet.type = PACKET_STATE;
    packet.int_data = 822;
    packet.int_x = where.x;
    packet.int_y = where.y;
    packet.flags = (1 << 5) | (1 << 10) | (1 << 11); //no enum rn so using raw values

    auto pos = local->GetPos();
    packet.pos_x = pos.x;
    packet.pos_y = pos.y;
    gt::send(&packet);
}

void gt::solve_captcha(std::string text) {
    utils::replace(text,
        "set_default_color|`o\nadd_label_with_icon|big|`wAre you Human?``|left|206|\nadd_spacer|small|\nadd_textbox|What will be the sum of the following "
        "numbers|left|\nadd_textbox|",
        "");
    utils::replace(text, "|left|\nadd_text_input|captcha_answer|Answer:||32|\nend_dialog|captcha_submit||Submit|", "");
    auto number1 = text.substr(0, text.find(" +"));
    auto number2 = text.substr(number1.length() + 3, text.length());
    int result = atoi(number1.c_str()) + atoi(number2.c_str());
    gt::sendlog("Solved captcha as `b" + std::to_string(result) + "``");
    gt::send(2, "action|dialog_return\ndialog_name|captcha_submit\ncaptcha_answer|" + std::to_string(result), sdk::GetPeer());
    //g_server->send(false, "action|dialog_return\ndialog_name|captcha_submit\ncaptcha_answer|" + std::to_string(result));
}

void gt::join_world(std::string worlddname) {
    std::string p = "action|join_request\nname|" + worlddname + "\ninvitedWorld|0";
    gt::send(3, p, false);
}

void gt::sendlog(std::string text) {
    variantlist_t var{ "OnConsoleMessage" };
    var[1] = "`0[`^INZERNAL`0] " + text;
    gt::send_varlist_self(var, -1, 0, true);
}





void gt::drop_item(std::string itemid, std::string amount) {
    std::string packet1 = "action|drop\nitemID|" + itemid;
    SendPacketHook::Execute(2, packet1, sdk::GetPeer());
    std::string packet2 = "action|dialog_return\ndialog_name|drop_item\nitemID|" + itemid + "\ncount|" + amount + "\n";
    SendPacketHook::Execute(2, packet2, sdk::GetPeer());
}

void gt::ReadConfig(const char* filename)
{
    std::ifstream file(filename);

    if (file.is_open())
    {
        nlohmann::json jsonConfig = nullptr;
        try
        {
            // We are reading a small file so we do not need to worry about inefficiency rn
            std::string json((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

            jsonConfig = nlohmann::json::parse(json);
        }
        catch (...)
        {
            MessageBoxA(0, "Failed to read config!", "Garhal", MB_OK | MB_ICONWARNING);
        }

        if (jsonConfig != NULL)
        {
 
            opt::cheat::vitemidX1 = jsonConfig["hat"];
            opt::cheat::vitemidX2 == jsonConfig["shoes"];
            opt::cheat::vitemidX3 = jsonConfig["wing"];

            opt::cheat::vitemidY1 = jsonConfig["shirt"];
            opt::cheat::vitemidY2 = jsonConfig["idk"];
            opt::cheat::vitemidY3 = jsonConfig["hair"];


            opt::cheat::vitemidZ1 = jsonConfig["pant"];
            opt::cheat::vitemidZ2 = jsonConfig["hand"];
            opt::cheat::vitemidZ2 = jsonConfig["neck"];
      

            file.close();
        }
    }
}



void gt::WriteConfig(const char* filename)
{
    FILE* pFile;
    fopen_s(&pFile, filename, "w");

    if (!pFile)
    {
        MessageBoxA(0, "Failed to save config!", "Garhal", MB_OK | MB_ICONWARNING);
        return;
    }

    nlohmann::json jsonConfig;

    jsonConfig["hat"] = opt::cheat::vitemidX1;
    jsonConfig["shoes"] = opt::cheat::vitemidX2;
    jsonConfig["wing"] = opt::cheat::vitemidX3;

    jsonConfig["shirt"] = opt::cheat::vitemidY1;
    jsonConfig["idk"] = opt::cheat::vitemidY2;
    jsonConfig["hair"] = opt::cheat::vitemidY3;


    jsonConfig["pant"] = opt::cheat::vitemidZ1;
    jsonConfig["hand"] = opt::cheat::vitemidZ2;
    jsonConfig["neck"] = opt::cheat::vitemidZ3;




    fprintf(pFile, "%s\n", jsonConfig.dump().c_str());

    fclose(pFile);
}


void gt::AddBlack(std::string playername)
{
    gt::sendlog(toString(playername));
}

int gt::get_punch_id(const int id_) {
    switch (id_) {
    case 138: return 1;
    case 366:
    case 1464:
        return 2;
    case 472: return 3;
    case 594:
    case 10130:
    case 5424:
    case 5456:
    case 4136:
    case 10052:
        return 4;
    case 768: return 5;
    case 900:
    case 7760:
    case 9272:
    case 5002:
    case 7758:
        return 6;
    case 910:
    case 4332:
        return 7;
    case 930:
    case 1010:
    case 6382:
        return 8;
    case 1016:
    case 6058:
        return 9;
    case 1204:
    case 9534:
        return 10;
    case 1378: return 11;
    case 1440: return 12;
    case 1484:
    case 5160:
    case 9802:
        return 13;
    case 1512:
    case 1648:
        return 14;
    case 1542: return 15;
    case 1576: return 16;
    case 1676:
    case 7504:
        return 17;
    case 1748:
    case 8006:
    case 8008:
    case 8010:
    case 8012:
        return 19;
    case 1710:
    case 4644:
    case 1714:
    case 1712:
    case 6044:
    case 1570:
        return 18;
    case 1780: return 20;
    case 1782:
    case 5156:
    case 9776:
    case 9782:
    case 9810:
        return 21;
    case 1804:
    case 5194:
    case 9784:
        return 22;
    case 1868:
    case 1998:
        return 23;
    case 1874: return 24;
    case 1946:
    case 2800:
        return 25;
    case 1952:
    case 2854:
        return 26;
    case 1956: return 27;
    case 1960: return 28;
    case 2908:
    case 6312:
    case 9496:
    case 8554:
    case 3162:
    case 9536:
    case 4956:
    case 3466:
    case 4166:
    case 4506:
    case 2952:
    case 9520:
    case 9522:
    case 8440:
    case 3932:
    case 3934:
    case 8732:
    case 3108:
    case 9766:
    case 12368:
        return 29;
    case 1980: return 30;
    case 2066:
    case 4150:
    case 11082:
    case 11080:
    case 11078:
        return 31;
    case 2212:
    case 5174:
    case 5004:
    case 5006:
    case 5008:
        return 32;
    case 2218: return 33;
    case 2220: return 34;
    case 2266: return 35;
    case 2386: return 36;
    case 2388: return 37;
    case 2450:
        return 38;
    case 2476:
    case 4208:
    case 12308:
    case 10336:
    case 9804:
        return 39;
    case 4748:
    case 4294:
        return 40;
    case 2512:
    case 9732:
    case 6338:
        return 41;
    case 2572: return 42;
    case 2592:
    case 9396:
    case 2596:
    case 9548:
    case 9812:
        return 43;
    case 2720: return 44;
    case 2752: return 45;
    case 2754: return 46;
    case 2756: return 47;
    case 2802: return 49;
    case 2866: return 50;
    case 2876: return 51;
    case 2878:
    case 2880:
        return 52;
    case 2906:
    case 4170:
    case 4278:
        return 53;
    case 2886: return 54;
    case 2890: return 55;
    case 2910: return 56;
    case 3066: return 57;
    case 3124: return 58;
    case 3168: return 59;
    case 3214:
    case 9194:
        return 60;
    case 7408:
    case 3238:
        return 61;
    case 3274: return 62;
    case 3300: return 64;
    case 3418: return 65;
    case 3476: return 66;
    case 3596: return 67;
    case 3686: return 68;
    case 3716: return 69;
    case 4290: return 71;
    case 4474: return 72;
    case 4464:
    case 9500:
        return 73;
    case 4746: return 75;
    case 4778:
    case 6026: case 7784:
        return 76;
    case 4996:
    case 3680:
    case 5176:
        return 77;
    case 4840: return 78;
    case 5206: return 79;
    case 5480:
    case 9770:
    case 9772:
        return 80;
    case 6110: return 81;
    case 6308: return 82;
    case 6310: return 83;
    case 6298: return 84;
    case 6756: return 85;
    case 7044: return 86;
    case 6892: return 87;
    case 6966: return 88;
    case 7088:
    case 11020:
        return 89;
    case 7098:
    case 9032:
        return 90;
    case 7192: return 91;
    case 7136:
    case 9738:
        return 92;
    case 3166: return 93;
    case 7216: return 94;
    case 7196:
    case 9340:
        return 95;
    case 7392:
    case 9604:
        return 96;
    case 7384: return 98;
    case 7414: return 99;
    case 7402: return 100;
    case 7424: return 101;
    case 7470: return 102;
    case 7488: return 103;
    case 7586:
    case 7646:
    case 9778:
        return 104;
    case 7650: return 105;
    case 6804:
    case 6358:
        return 106;
    case 7568:
    case 7570:
    case 7572:
    case 7574:
        return 107;
    case 7668: return 108;
    case 7660:
    case 9060:
        return 109;
    case 7584:
        return 110;
    case 7736:
    case 9116:
    case 9118:
    case 7826:
    case 7828:
    case 11440:
    case 11442:
    case 11312:
    case 7830:
    case 7832:
    case 10670:
    case 9120:
    case 9122:
    case 10680:
    case 10626:
    case 10578:
    case 10334:
    case 11380:
    case 11326:
    case 7912:
    case 11298:
    case 10498:
    case 12342:
        return 111;
    case 7836:
    case 7838:
    case 7840:
    case 7842:
        return 112;
    case 7950: return 113;
    case 8002: return 114;
    case 8022: return 116;
    case 8036: return 118;
    case 9348:
    case 8372:
        return 119;
    case 8038: return 120;
    case 8816:
    case 8818:
    case 8820:
    case 8822:
        return 128;
    case 8910: return 129;
    case 8942: return 130;
    case 8944:
    case 5276:
        return 131;
    case 8432:
    case 8434:
    case 8436:
    case 8950:
        return 132;
    case 8946: case 9576: return 133;
    case 8960: return 134;
    case 9006: return 135;
    case 9058: return 136;
    case 9082:
    case 9304:
        return 137;
    case 9066:
        return 138;
    case 9136: return 139;
    case 9138:
        return 140;
    case 9172: return 141;
    case 9254: return 143;
    case 9256: return 144;
    case 9236: return 145;
    case 9342: return 146;
    case 9542: return 147;
    case 9378: return 148;
    case 9376: return 149;
    case 9410: return 150;
    case 9462: return 151;
    case 9606:
        return 152;
    case 9716:
    case 5192:
        return 153;
    case 10048: return 167;
    case 10064: return 168;
    case 10046: return 169;
    case 10050: return 170;
    case 10128: return 171;
    case 10210:
    case 9544:
        return 172;
    case 10330: return 178;
    case 10398: return 179;
    case 10388:
    case 9524:
    case 9598:
        return 180;
    case 10442: return 184;
    case 10506: return 185;
    case 10652: return 188;
    case 10676: return 191;
    case 10694: return 193;
    case 10714: return 194;
    case 10724: return 195;
    case 10722: return 196;
    case 10754: return 197;
    case 10800: return 198;
    case 10888: return 199;
    case 10886:
    case 11308:
        return 200;
    case 10890: return 202;
    case 10922: case 9550: return 203;
    case 10990: return 205;
    case 10998: return 206;
    case 10952: return 207;
    case 11000: return 208;
    case 11006: return 209;
    case 11046: return 210;
    case 11052: return 211;
    case 10960: return 212;
    case 10956:
    case 9774:
        return 213;
    case 10958: return 214;
    case 10954: return 215;
    case 11076: return 216;
    case 11084: return 217;
    case 11118:
    case 9546:
    case 9574:
        return 218;
    case 11120: return 219;
    case 11116: return 220;
    case 11158: return 221;
    case 11162: return 222;
    case 11142: return 223;
    case 11232: return 224;
    case 11140: return 225;
    case 11248:
    case 9596:
    case 9636:
        return 226;
    case 11240: return 227;
    case 11250: return 228;
    case 11284: return 229;
    case 11292: return 231;
    case 11314: return 233;
    case 11316: return 234;
    case 11324: return 235;
    case 11354: return 236;
    case 11760:
    case 11464:
    case 11438:
    case 12230:
    case 11716:
    case 11718:
    case 11674:
    case 11630:
    case 11786:
    case 11872:
    case 11762:
    case 11994:
    case 12172:
    case 12184:
    case 11460:
    case 12014:
    case 12016:
    case 12018:
    case 12020:
    case 12022:
    case 12024:
    case 12246:
    case 12248:
    case 12176:
    case 12242:
    case 11622:
    case 12350:
    case 12300:
    case 12374:
    case 12356:
        return 237;
    case 11814:
    case 12232:
    case 12302:
        return 241;
    case 11548:
    case 11552:
        return 242;
    case 11704:
    case 11706:
        return 243;
    case 12180:
    case 12346:
    case 12344:
        return 244;
    case 11506:
    case 11508:
    case 11562:
    case 11768:
    case 11882:
    case 11720:
    case 11884:
        return 245;
    case 12432:
    case 12434:
        return 246;
    case 11818:
    case 11876:
    case 12000:
    case 12240:
    case 12642:
    case 12644:
        return 248;

    }
    return 0;
}


std::string gt::lowercase(std::string s)
{
    std::string s2 = s;
    std::transform(s2.begin(), s2.end(), s2.begin(), tolower);
    return s2;
}


void gt::place_with_item_id(int x, int y, int itemid) {
    auto local = sdk::GetGameLogic()->GetLocalPlayer();
    GameUpdatePacket packet{ 0 };
    packet.type = 3;
    packet.int_data = itemid;
    packet.pos_x = local->pos.x;
    packet.pos_y = local->pos.y;
    packet.int_x = x;
    packet.int_y = y;
    gt::send(&packet);
}

void gt::SendPunch(int x, int y) {
    auto local = sdk::GetGameLogic()->GetLocalPlayer();
    GameUpdatePacket packet{ 0 };
    packet.type = 3;
    packet.int_data = 18;
    packet.int_x = x;
    packet.int_y = y;
    packet.pos_x = local->pos.x;
    packet.pos_y = local->pos.y;
    gt::send(&packet);
}

