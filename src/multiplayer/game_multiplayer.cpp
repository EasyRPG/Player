#include <array>
#include <map>
#include <memory>
#include <queue>
#include <charconv>
#include <utility>
#include <bitset>
#include <algorithm>
#include <limits>

#include <lcf/data.h>
#include <lcf/reader_util.h>

#include "game_multiplayer.h"
#include "../output.h"
#include "../game_player.h"
#include "../sprite_character.h"
#include "../window_base.h"
#include "../drawable_mgr.h"
#include "../scene.h"
#include "../audio.h"
#include "../bitmap.h"
#include "../font.h"
#include "../input.h"
#include "../game_map.h"
#include "../game_party.h"
#include "../game_system.h"
#include "../game_screen.h"
#include "../game_switches.h"
#include "../game_variables.h"
#include "../battle_animation.h"
#include "../player.h"
#include "../cache.h"
#include "chatui.h"
#include "nametag.h"
#include "game_playerother.h"
#include "playerother.h"
#include "messages.h"
#include "server.h"

using namespace Messages;

static Game_Multiplayer _instance;

Game_Multiplayer& Game_Multiplayer::Instance() {
	return _instance;
}

Game_Multiplayer::Game_Multiplayer() {
	InitConnection();
}

void Game_Multiplayer::SpawnOtherPlayer(int id) {
	auto& player = Main_Data::game_player;
	auto& nplayer = players[id].ch;
	nplayer.reset(new Game_PlayerOther(id));
	nplayer->SetSpriteGraphic(player->GetSpriteName(), player->GetSpriteIndex());
	nplayer->SetMoveSpeed(player->GetMoveSpeed());
	nplayer->SetMoveFrequency(player->GetMoveFrequency());
	nplayer->SetThrough(true);
	nplayer->SetLayer(player->GetLayer());
	nplayer->SetMultiplayerVisible(false);
	nplayer->SetBaseOpacity(0);

	auto scene_map = Scene::Find(Scene::SceneType::Map);
	if (!scene_map) {
		Output::Error("MP: unexpected, {}:{}", __FILE__, __LINE__);
		return;
	}
	auto old_list = &DrawableMgr::GetLocalList();
	DrawableMgr::SetLocalList(&scene_map->GetDrawableList());
	auto& sprite = players[id].sprite;
	sprite = std::make_unique<Sprite_Character>(nplayer.get());
	sprite->SetTone(Main_Data::game_screen->GetTone());
	DrawableMgr::SetLocalList(old_list);
}

// this assumes that the player is stopped
// return true if player moves normally, false if players teleports
static bool MovePlayerToPos(Game_PlayerOther& player, int x, int y) {
	if (!player.IsStopping()) {
		Output::Error("MP: MovePlayerToPos unexpected error: the player is busy being animated");
	}
	int dx = x - player.GetX();
	int dy = y - player.GetY();
	int adx = abs(dx);
	int ady = abs(dy);
	if (Game_Map::LoopHorizontal() && adx == Game_Map::GetTilesX() - 1) {
		dx = dx > 0 ? -1 : 1;
		adx = 1;
	}
	if (Game_Map::LoopVertical() && ady == Game_Map::GetTilesY() - 1) {
		dy = dy > 0 ? -1 : 1;
		ady = 1;
	}
	if (adx > 1 || ady > 1 || (dx == 0 && dy == 0) || !player.IsMultiplayerVisible()) {
		player.SetX(x);
		player.SetY(y);
		return false;
	}
	using D = Game_Character::Direction;
	constexpr int dir[3][3]{
		{D::UpLeft,   D::Up,   D::UpRight},
		{D::Left,     0,       D::Right},
		{D::DownLeft, D::Down, D::DownRight},
	};
	player.Move(dir[dy+1][dx+1]);
	return true;
}

void Game_Multiplayer::ResetRepeatingFlash() {
	frame_index = -1;
	last_flash_frame_index = -1;
	last_frame_flash.reset();
	repeating_flashes.clear();
}

void Game_Multiplayer::InitConnection() {
	using SystemMessage = ClientConnection::SystemMessage;
	using Connection = Multiplayer::Connection;

	connection.RegisterSystemHandler(SystemMessage::OPEN, [this](Connection& _) {
		CUI().SetStatusConnection(true);
		SendBasicData();
		connection.SendPacket(NamePacket(cfg.client_chat_name.Get()));
	});
	connection.RegisterSystemHandler(SystemMessage::CLOSE, [this](Connection& _) {
		CUI().SetStatusConnection(false);
		if (active) {
			Output::Debug("MP: connection is closed");
			if (reconnect_wait) return;
			reconnect_wait = true;
			std::thread([this]() {
				std::this_thread::sleep_for(std::chrono::seconds(3));
				reconnect_wait = false;
				if (active) {
					Output::Info("MP: reconnecting: ID={}", room_id);
					Connect();
				}
			}).detach();
		}
	});
	connection.RegisterSystemHandler(SystemMessage::EXIT, [this](Connection& _) {
		CUI().GotInfo("!! Server exited");
		Disconnect();
	});
	connection.RegisterSystemHandler(SystemMessage::ACCESSDENIED_TOO_MANY_USERS, [this](Connection& _) {
		Disconnect();
		CUI().GotInfo("!! Access denied. Too many users");
	});

	// ->> unused code
	connection.RegisterHandler<SyncSwitchPacket>([this](SyncSwitchPacket& p) {
		int value_bin = (int) Main_Data::game_switches->GetInt(p.switch_id);
		if (p.sync_type != 1) {
			connection.SendPacketAsync<SyncSwitchPacket>(p.switch_id, value_bin);
		}
		if (p.sync_type >= 1) {
			sync_switches.push_back(p.switch_id);
		}
	});
	connection.RegisterHandler<SyncVariablePacket>([this](SyncVariablePacket& p) {
		auto value = 0;
		switch (p.var_id) {
			case 10000:
				value = Main_Data::game_party->GetGold();
				break;
			case 10001:
				value = Main_Data::game_party->GetActor(0)->GetBaseMaxHp();
				break;
			default:
				value = (int) Main_Data::game_variables->Get(p.var_id);
				break;
		}
		if (p.sync_type != 1) {
			connection.SendPacketAsync<SyncVariablePacket>(p.var_id, value);
		}
		if (p.sync_type >= 1) {
			sync_vars.push_back(p.var_id);
		}
	});
	connection.RegisterHandler<SyncEventPacket>([this](SyncEventPacket& p) {
		if (p.trigger_type != 1) {
			sync_events.push_back(p.event_id);
		}
		if (p.trigger_type >= 1) {
			sync_action_events.push_back(p.event_id);
		}
	});
	connection.RegisterHandler<SyncPicturePacket>([this](SyncPicturePacket& p) {
		sync_picture_names.push_back(p.picture_name);
	});
	// <<-
	connection.RegisterHandler<PictureNameListSyncPacket>([this](PictureNameListSyncPacket& p) {
		std::vector<std::string>* list;
		switch (p.type) {
		case 0:
			list = &global_sync_picture_names;
			break;
		case 1:
			list = &global_sync_picture_prefixes;
			break;
		default:
			std::terminate();
		}
		list->assign(p.names.begin(), p.names.end());
	});
	connection.RegisterHandler<BattleAnimIdListSyncPacket>([this](BattleAnimIdListSyncPacket& p) {
		sync_battle_anim_ids.assign(p.ids.begin(), p.ids.end());
	});
	connection.RegisterHandler<RoomPacket>([this](RoomPacket& p) {
		if (p.room_id != room_id) {
			SwitchRoom(room_id); // wrong room, resend
			return;
		}
		// server syned. accept other players spawn
		switching_room = false;
	});
	connection.RegisterHandler<JoinPacket>([this](JoinPacket& p) {
		// I am entering a new room and don't care about players in the old(server side) room
		if (switching_room)
			return;
		if (players.find(p.id) == players.end())
			SpawnOtherPlayer(p.id);
	});
	connection.RegisterHandler<LeavePacket>([this](LeavePacket& p) {
		{
			auto it = global_players_system.find(p.id);
			if (it != global_players_system.end())
				global_players_system.erase(it);
		}
		auto it = players.find(p.id);
		if (it == players.end()) return;
		auto& player = it->second;
		if (player.name_tag) {
			auto scene_map = Scene::Find(Scene::SceneType::Map);
			if (!scene_map) {
				Output::Error("MP: unexpected, {}:{}", __FILE__, __LINE__);
				//return;
			}
			auto old_list = &DrawableMgr::GetLocalList();
			DrawableMgr::SetLocalList(&scene_map->GetDrawableList());
			player.name_tag.reset();
			DrawableMgr::SetLocalList(old_list);
		}
		dc_players.emplace_back(std::move(player));
		players.erase(it);
		repeating_flashes.erase(p.id);
		if (Main_Data::game_pictures) {
			Main_Data::game_pictures->EraseAllMultiplayerForPlayer(p.id);
		}
	});
	connection.RegisterHandler<ChatPacket>([this](ChatPacket& p) {
		if (p.type == 0)
			CUI().GotInfo(p.message);
		else if (p.type == 1) {
			std::string sys_graphic = "";
			auto it = global_players_system.find(p.id);
			if (it != global_players_system.end())
				sys_graphic = it->second;
			CUI().GotMessage(p.visibility, p.room_id, p.name, p.message, sys_graphic);
		}
	});
	connection.RegisterHandler<MovePacket>([this](MovePacket& p) {
		if (players.find(p.id) == players.end()) return;
		auto& player = players[p.id];
		int x = Utils::Clamp(p.x, 0, Game_Map::GetTilesX() - 1);
		int y = Utils::Clamp(p.y, 0, Game_Map::GetTilesY() - 1);
		player.mvq.emplace_back(x, y);
	});
	connection.RegisterHandler<JumpPacket>([this](JumpPacket& p) {
		if (players.find(p.id) == players.end()) return;
		auto& player = players[p.id];
		int x = Utils::Clamp(p.x, 0, Game_Map::GetTilesX() - 1);
		int y = Utils::Clamp(p.y, 0, Game_Map::GetTilesY() - 1);
		auto rc = player.ch->Jump(x, y);
		if (rc) {
			player.ch->SetMaxStopCount(player.ch->GetMaxStopCountForStep(player.ch->GetMoveFrequency()));
		}
	});
	connection.RegisterHandler<FacingPacket>([this](FacingPacket& p) {
		if (players.find(p.id) == players.end()) return;
		auto& player = players[p.id];
		int facing = Utils::Clamp(p.facing, 0, 3);
		player.ch->SetFacing(facing);
	});
	connection.RegisterHandler<SpeedPacket>([this](SpeedPacket& p) {
		if (players.find(p.id) == players.end()) return;
		auto& player = players[p.id];
		int speed = Utils::Clamp(p.speed, 1, 6);
		player.ch->SetMoveSpeed(speed);
	});
	connection.RegisterHandler<SpritePacket>([this](SpritePacket& p) {
		if (players.find(p.id) == players.end()) return;
		auto& player = players[p.id];
		int idx = Utils::Clamp(p.index, 0, 7);
		player.ch->SetSpriteGraphic(std::string(p.name), idx);
	});
	connection.RegisterHandler<FlashPacket>([this](FlashPacket& p) {
		if (players.find(p.id) == players.end()) return;
		auto& player = players[p.id];
		player.ch->Flash(p.r, p.g, p.b, p.p, p.f);
	});
	connection.RegisterHandler<RepeatingFlashPacket>([this](RepeatingFlashPacket& p) {
		if (players.find(p.id) == players.end()) return;
		auto& player = players[p.id];
		auto flash_array = std::array<int, 5>{ p.r, p.g, p.b, p.p, p.f };
		repeating_flashes[p.id] = std::array<int, 5>(flash_array);
		player.ch->Flash(p.r, p.g, p.b, p.p, p.f);
	});
	connection.RegisterHandler<RemoveRepeatingFlashPacket>([this](RemoveRepeatingFlashPacket& p) {
		if (players.find(p.id) == players.end()) return;
		repeating_flashes.erase(p.id);
	});
	connection.RegisterHandler<HiddenPacket>([this](HiddenPacket& p) {
		if (players.find(p.id) == players.end()) return;
		auto& player = players[p.id];
		player.ch->SetSpriteHidden(p.hidden_bin == 1);
	});

	auto SetGlobalPlayersSystemGraphic = [this] (SystemPacket& p) {
		auto it = global_players_system.find(p.id);
		if (it != global_players_system.end()) {
			if (it->second == p.name)
				return;
		}
		global_players_system[p.id] = p.name;
		if (players.find(p.id) == players.end()) return;
		auto& player = players[p.id];
		auto name_tag = player.name_tag.get();
		if (name_tag) {
			name_tag->SetSystemGraphic(p.name);
		}
	};

	connection.RegisterHandler<SystemPacket>([this, SetGlobalPlayersSystemGraphic](SystemPacket& p) {
		// local Player always return ture
		if (Cache::System(p.name)) {
			SetGlobalPlayersSystemGraphic(p);
		} else {
			FileRequestAsync* request = AsyncHandler::RequestFile("System", p.name);
			sys_graphic_request_id = request->Bind([this, &p, SetGlobalPlayersSystemGraphic](FileRequestResult* result) {
				if (!result->success) {
					return;
				}
				SetGlobalPlayersSystemGraphic(p);
			});
			request->SetGraphicFile(true);
			request->Start();
		}
	});
	connection.RegisterHandler<SEPacket>([this](SEPacket& p) { // se: sound effect
		if (players.find(p.id) == players.end()) return;
		if (settings.enable_sounds) {
			auto& player = players[p.id];

			int px = Main_Data::game_player->GetX();
			int py = Main_Data::game_player->GetY();
			int ox = player.ch->GetX();
			int oy = player.ch->GetY();

			int hmw = Game_Map::GetTilesX() / 2;
			int hmh = Game_Map::GetTilesY() / 2;

			int rx;
			int ry;

			if (Game_Map::LoopHorizontal() && px - ox >= hmw) {
				rx = Game_Map::GetTilesX() - (px - ox);
			} else if (Game_Map::LoopHorizontal() && px - ox < hmw * -1) {
				rx = Game_Map::GetTilesX() + (px - ox);
			} else {
				rx = px - ox;
			}

			if (Game_Map::LoopVertical() && py - oy >= hmh) {
				ry = Game_Map::GetTilesY() - (py - oy);
			} else if (Game_Map::LoopVertical() && py - oy < hmh * -1) {
				ry = Game_Map::GetTilesY() + (py - oy);
			} else {
				ry = py - oy;
			}

			int dist = std::sqrt(rx * rx + ry * ry);
			float dist_volume = 75.0f - ((float)dist * 10.0f);
			float sound_volume_multiplier = float(p.snd.volume) / 100.0f;
			int real_volume = std::max((int)(dist_volume * sound_volume_multiplier), 0);

			lcf::rpg::Sound sound;
			sound.name = p.snd.name;
			sound.volume = real_volume;
			sound.tempo = p.snd.tempo;
			sound.balance = p.snd.balance;

			Main_Data::game_system->SePlay(sound);
		}
	});

	auto modify_args = [] (PicturePacket& pa) {
		if (Game_Map::LoopHorizontal()) {
			int alt_map_x = pa.map_x + Game_Map::GetTilesX() * TILE_SIZE * TILE_SIZE;
			if (std::abs(pa.map_x - Game_Map::GetPositionX()) > std::abs(alt_map_x - Game_Map::GetPositionX())) {
				pa.map_x = alt_map_x;
			}
		}
		if (Game_Map::LoopVertical()) {
			int alt_map_y = pa.map_y + Game_Map::GetTilesY() * TILE_SIZE * TILE_SIZE;
			if (std::abs(pa.map_y - Game_Map::GetPositionY()) > std::abs(alt_map_y - Game_Map::GetPositionY())) {
				pa.map_y = alt_map_y;
			}
		}
		pa.params.position_x += (int)(std::floor((pa.map_x / TILE_SIZE) - (pa.pan_x / (TILE_SIZE * 2))) - std::floor((Game_Map::GetPositionX() / TILE_SIZE) - Main_Data::game_player->GetPanX() / (TILE_SIZE * 2)));
		pa.params.position_y += (int)(std::floor((pa.map_y / TILE_SIZE) - (pa.pan_y / (TILE_SIZE * 2))) - std::floor((Game_Map::GetPositionY() / TILE_SIZE) - Main_Data::game_player->GetPanY() / (TILE_SIZE * 2)));
	};

	connection.RegisterHandler<ShowPicturePacket>([this, modify_args](ShowPicturePacket& p) {
		if (players.find(p.id) == players.end()) return;
		modify_args(p);
		int pic_id = p.pic_id + (p.id + 1) * 50; //offset to avoid conflicting with others using the same picture
		Main_Data::game_pictures->Show(pic_id, p.params);
	});
	connection.RegisterHandler<MovePicturePacket>([this, modify_args](MovePicturePacket& p) {
		if (players.find(p.id) == players.end()) return;
		int pic_id = p.pic_id + (p.id + 1) * 50; //offset to avoid conflicting with others using the same picture
		modify_args(p);
		Main_Data::game_pictures->Move(pic_id, p.params);
	});
	connection.RegisterHandler<ErasePicturePacket>([this](ErasePicturePacket& p) {
		if (players.find(p.id) == players.end()) return;
		int pic_id = p.pic_id + (p.id + 1) * 50; //offset to avoid conflicting with others using the same picture
		Main_Data::game_pictures->Erase(pic_id);
	});
	connection.RegisterHandler<ShowPlayerBattleAnimPacket>([this](ShowPlayerBattleAnimPacket& p) {
		if (players.find(p.id) == players.end()) return;
		const lcf::rpg::Animation* anim = lcf::ReaderUtil::GetElement(lcf::Data::animations, p.anim_id);
		if (anim) {
			players[p.id].battle_animation.reset(new BattleAnimationMap(*anim, *players[p.id].ch, false, true, true));
		} else {
			players[p.id].battle_animation.reset();
		}
	});
	connection.RegisterHandler<NamePacket>([this](NamePacket& p) {
		if (players.find(p.id) == players.end()) return;
		auto& player = players[p.id];
		auto scene_map = Scene::Find(Scene::SceneType::Map);
		if (!scene_map) {
			Output::Error("MP: unexpected, {}:{}", __FILE__, __LINE__);
			//return;
		}
		auto old_list = &DrawableMgr::GetLocalList();
		DrawableMgr::SetLocalList(&scene_map->GetDrawableList());
		player.name_tag = std::make_unique<NameTag>(p.id, player, std::string(p.name));
		DrawableMgr::SetLocalList(old_list);
	});
}

void Game_Multiplayer::SetConfig(const Game_ConfigMultiplayer& _cfg) {
	cfg = _cfg;
	Server().SetConfig(_cfg);
	if (cfg.server_auto_start.Get())
		Server().Start();
	connection.SetConfig(&cfg);
	// Heartbeat
	if (!cfg.no_heartbeats.Get()) {
		connection.RegisterHandler<HeartbeatPacket>([this](HeartbeatPacket& p) {});
		std::thread([this]() {
			while (true) {
				std::this_thread::sleep_for(std::chrono::seconds(3));
				if (active && connection.IsConnected()) {
					connection.SendPacket(HeartbeatPacket());
				}
			}
		}).detach();
	}
}

Game_ConfigMultiplayer Game_Multiplayer::GetConfig() const {
	return cfg;
}

void Game_Multiplayer::SetChatName(std::string chat_name) {
	cfg.client_chat_name.Set(std::string(chat_name));
	connection.SendPacket(NamePacket(cfg.client_chat_name.Get()));
}

std::string Game_Multiplayer::GetChatName() {
	return cfg.client_chat_name.Get();
}

void Game_Multiplayer::SetRemoteAddress(std::string address) {
	cfg.client_remote_address.Set(std::string(address));
	connection.SetAddress(cfg.client_remote_address.Get());
}

void Game_Multiplayer::Connect() {
	if (connection.IsConnected()) return;
	active = true;
	connection.SetAddress(cfg.client_remote_address.Get());
	CUI().SetStatusConnection(false, true);
	connection.Open();
	if (room_id != -1)
		SwitchRoom(room_id);
}

void Game_Multiplayer::Disconnect() {
	active = false;
	Reset();
	connection.Close();
	CUI().SetStatusConnection(false);
}

void Game_Multiplayer::SwitchRoom(int map_id, bool room_switch) {
	SetNametagMode(cfg.client_name_tag_mode.Get());
	CUI().Refresh();
	CUI().SetStatusRoom(map_id);
	Output::Debug("MP: room_id=map_id={}", map_id);
	room_id = map_id;
	if (!active) {
		bool auto_connect = cfg.client_auto_connect.Get();
		if (auto_connect) {
			active = true;
			Connect();
		}
		Output::Debug("MP: active={} auto_connect={}", active, auto_connect);
		return;
	}
	switching_room = true;
	if (room_switch) {
		switched_room = false;
	}
	Reset();
	dc_players.clear();
	if (connection.IsConnected())
		SendBasicData();
}

void Game_Multiplayer::Reset() {
	players.clear();
	sync_switches.clear();
	sync_vars.clear();
	sync_events.clear();
	sync_action_events.clear();
	sync_picture_names.clear();
	ResetRepeatingFlash();
	if (Main_Data::game_pictures) {
		Main_Data::game_pictures->EraseAllMultiplayer();
	}
}

void Game_Multiplayer::MapQuit() {
	SetNametagMode(cfg.client_name_tag_mode.Get());
	CUI().Refresh();
	Reset();
}

void Game_Multiplayer::Quit() {
	Disconnect();
}

void Game_Multiplayer::SendChatMessage(int visibility, std::string message) {
	connection.SendPacket(ChatPacket(visibility, message));
}

void Game_Multiplayer::SendBasicData() {
	auto& player = Main_Data::game_player;
	connection.SendPacketAsync<MovePacket>(player->GetX(), player->GetY());
	connection.SendPacketAsync<SpeedPacket>(player->GetMoveSpeed());
	connection.SendPacketAsync<SpritePacket>(player->GetSpriteName(),
				player->GetSpriteIndex());
	if (player->GetFacing() > 0) {
		connection.SendPacketAsync<FacingPacket>(player->GetFacing());
	}
	connection.SendPacketAsync<HiddenPacket>(player->IsSpriteHidden());
	auto sysn = Main_Data::game_system->GetSystemName();
	connection.SendPacketAsync<SystemPacket>(ToString(sysn));
	connection.SendPacketAsync<RoomPacket>(room_id);
}

void Game_Multiplayer::MainPlayerMoved(int dir) {
	auto& p = Main_Data::game_player;
	connection.SendPacketAsync<MovePacket>(p->GetX(), p->GetY());
}

void Game_Multiplayer::MainPlayerFacingChanged(int dir) {
	connection.SendPacketAsync<FacingPacket>(dir);
}

void Game_Multiplayer::MainPlayerChangedMoveSpeed(int spd) {
	connection.SendPacketAsync<SpeedPacket>(spd);
}

void Game_Multiplayer::MainPlayerChangedSpriteGraphic(std::string name, int index) {
	connection.SendPacketAsync<SpritePacket>(name, index);
}

void Game_Multiplayer::MainPlayerJumped(int x, int y) {
	auto& p = Main_Data::game_player;
	connection.SendPacketAsync<JumpPacket>(x, y);
}

void Game_Multiplayer::MainPlayerFlashed(int r, int g, int b, int p, int f) {
	std::array<int, 5> flash_array = std::array<int, 5>{ r, g, b, p, f };
	if (last_flash_frame_index == frame_index - 1 && (last_frame_flash.get() == nullptr || *last_frame_flash == flash_array)) {
		if (last_frame_flash.get() == nullptr) {
			last_frame_flash = std::make_unique<std::array<int, 5>>(flash_array);
			connection.SendPacketAsync<RepeatingFlashPacket>(r, g, b, p, f);
		}
	} else {
		connection.SendPacketAsync<FlashPacket>(r, g, b, p, f);
		last_frame_flash.reset();
	}
	last_flash_frame_index = frame_index;
}

void Game_Multiplayer::MainPlayerChangedSpriteHidden(bool hidden) {
	int hidden_bin = hidden ? 1 : 0;
	connection.SendPacketAsync<HiddenPacket>(hidden_bin);
}

void Game_Multiplayer::MainPlayerTeleported(int map_id, int x, int y) {
	connection.SendPacketAsync<TeleportPacket>(x, y);
}

void Game_Multiplayer::MainPlayerTriggeredEvent(int event_id, bool action) {
	auto sep = [this, event_id](int action) {
		connection.SendPacketAsync<SyncEventPacket>(event_id, action);
	};
	if (action) {
		if (std::find(sync_action_events.begin(), sync_action_events.end(), event_id) != sync_action_events.end()) {
			sep(1);
		}
	} else {
		if (std::find(sync_events.begin(), sync_events.end(), event_id) != sync_events.end()) {
			sep(0);
		}
	}
}

void Game_Multiplayer::SystemGraphicChanged(StringView sys) {
	CUI().Refresh();
	connection.SendPacketAsync<SystemPacket>(ToString(sys));
}

void Game_Multiplayer::SePlayed(const lcf::rpg::Sound& sound) {
	if (!Main_Data::game_player->IsMenuCalling()) {
		connection.SendPacketAsync<SEPacket>(sound);
	}
}

bool Game_Multiplayer::IsPictureSynced(int pic_id, Game_Pictures::ShowParams& params) {
	bool picture_synced = false;

	for (auto& picture_name : global_sync_picture_names) {
		if (picture_name == params.name) {
			picture_synced = true;
			break;
		}
	}

	if (!picture_synced) {
		for (auto& picture_prefix : global_sync_picture_prefixes) {
			std::string name_lower = params.name;
			std::transform(name_lower.begin(), name_lower.end(), name_lower.begin(), [](unsigned char c) { return std::tolower(c); });
			if (name_lower.rfind(picture_prefix, 0) == 0) {
				picture_synced = true;
				break;
			}
		}
	}

	sync_picture_cache[pic_id] = picture_synced;

	if (!picture_synced) {
		for (auto& picture_name : sync_picture_names) {
			if (picture_name == params.name) {
				picture_synced = true;
				break;
			}
		}
	}

	return picture_synced;
}

void Game_Multiplayer::PictureShown(int pic_id, Game_Pictures::ShowParams& params) {
	if (IsPictureSynced(pic_id, params)) {
		auto& p = Main_Data::game_player;
		connection.SendPacketAsync<ShowPicturePacket>(pic_id, params,
			Game_Map::GetPositionX(), Game_Map::GetPositionY(),
			p->GetPanX(), p->GetPanY());
	}
}

void Game_Multiplayer::PictureMoved(int pic_id, Game_Pictures::MoveParams& params) {
	if (sync_picture_cache.count(pic_id) && sync_picture_cache[pic_id]) {
		auto& p = Main_Data::game_player;
		connection.SendPacketAsync<MovePicturePacket>(pic_id, params,
			Game_Map::GetPositionX(), Game_Map::GetPositionY(),
			p->GetPanX(), p->GetPanY());
	}
}

void Game_Multiplayer::PictureErased(int pic_id) {
	if (sync_picture_cache.count(pic_id) && sync_picture_cache[pic_id]) {
		sync_picture_cache.erase(pic_id);
		connection.SendPacketAsync<ErasePicturePacket>(pic_id);
	}
}

bool Game_Multiplayer::IsBattleAnimSynced(int anim_id) {
	bool anim_synced = false;

	for (auto& battle_anim_id : sync_battle_anim_ids) {
		if (battle_anim_id == anim_id) {
			anim_synced = true;
			break;
		}
	}

	return anim_synced;
}

void Game_Multiplayer::PlayerBattleAnimShown(int anim_id) {
	if (IsBattleAnimSynced(anim_id)) {
		connection.SendPacketAsync<ShowPlayerBattleAnimPacket>(anim_id);
	}
}

void Game_Multiplayer::ApplyPlayerBattleAnimUpdates() {
	for (auto& p : players) {
		if (p.second.battle_animation) {
			auto& ba = p.second.battle_animation;
			if (!ba->IsDone()) {
				ba->Update();
			}
			if (ba->IsDone()) {
				ba.reset();
			}
		}
	}
}

void Game_Multiplayer::ApplyFlash(int r, int g, int b, int power, int frames) {
	for (auto& p : players) {
		p.second.ch->Flash(r, g, b, power, frames);
		auto name_tag = p.second.name_tag.get();
		if (name_tag)
			name_tag->SetFlashFramesLeft(frames);
	}
}

void Game_Multiplayer::ApplyRepeatingFlashes() {
	for (auto& rf : repeating_flashes) {
		if (players.find(rf.first) != players.end()) {
			std::array<int, 5> flash_array = rf.second;
			players[rf.first].ch->Flash(flash_array[0], flash_array[1], flash_array[2], flash_array[3], flash_array[4]);
			auto name_tag = players[rf.first].name_tag.get();
			if (name_tag)
				name_tag->SetFlashFramesLeft(flash_array[4]);
		}
	}
}

void Game_Multiplayer::ApplyTone(Tone tone) {
	for (auto& p : players) {
		p.second.sprite->SetTone(tone);
		auto name_tag = p.second.name_tag.get();
		if (name_tag)
			name_tag->SetEffectsDirty();
	}
}

void Game_Multiplayer::SwitchSet(int switch_id, int value_bin) {
	if (std::find(sync_switches.begin(), sync_switches.end(), switch_id) != sync_switches.end()) {
		connection.SendPacketAsync<SyncSwitchPacket>(switch_id, value_bin);
	}
}

void Game_Multiplayer::VariableSet(int var_id, int value) {
	if (std::find(sync_vars.begin(), sync_vars.end(), var_id) != sync_vars.end()) {
		connection.SendPacketAsync<SyncVariablePacket>(var_id, value);
	}
}

void Game_Multiplayer::ApplyScreenTone() {
	ApplyTone(Main_Data::game_screen->GetTone());
}

void Game_Multiplayer::Update() {
	if (active) {
		connection.Receive();
	}
}

void Game_Multiplayer::MapUpdate() {
	if (active) {
		if (last_flash_frame_index > -1 && frame_index > last_flash_frame_index) {
			connection.SendPacketAsync<RemoveRepeatingFlashPacket>();
			last_flash_frame_index = -1;
			last_frame_flash.reset();
		}

		++frame_index;

		bool check_name_tag_overlap = frame_index % (8 + ((players.size() >> 4) << 3)) == 0;

		for (auto& p : players) {
			auto& q = p.second.mvq;
			auto& ch = p.second.ch;
			// if player moves too fast
			if (q.size() > settings.moving_queue_limit) {
				q.erase(
					q.begin(),
					std::next(q.begin(), q.size() - settings.moving_queue_limit)
				);
			}
			if (!q.empty() && ch->IsStopping()) {
				auto [x, y] = q.front();
				MovePlayerToPos(*ch, x, y);
				if (!switched_room) {
					ch->SetMultiplayerVisible(true);
					ch->SetBaseOpacity(32);
				}
				// Re-implement when we find a way to improve this visually
				/*struct {
					int x, y;
				} previous{
					ch->GetX(), ch->GetY()
				};
				auto isNormalMove = MovePlayerToPos(*ch, x, y);
				if (!isNormalMove) {
					// fade in at new position
					ch->SetBaseOpacity(0);
					dc_players.emplace_back(p.second.Shadow(previous.x, previous.y));
				}*/
				q.pop_front();
				if (!ch->IsMultiplayerVisible()) {
					ch->SetMultiplayerVisible(true);
				}
			}
			if (ch->IsMultiplayerVisible() && ch->GetBaseOpacity() < 32) {
				ch->SetBaseOpacity(ch->GetBaseOpacity() + 1);
			}
			ch->SetProcessed(false);
			ch->Update();
			p.second.sprite->Update();

			if (check_name_tag_overlap) {
				bool overlap = false;
				int x = ch->GetX();
				int y = ch->GetY();
				for (auto& p2 : players) {
					auto& ch2 = p2.second.ch;
					int x2 = ch2->GetX();
					if (x == x2) {
						int y2 = ch2->GetY();
						if (y == 0) {
							if (Game_Map::LoopVertical() && y2 == Game_Map::GetTilesY() - 1) {
								overlap = true;
								break;
							}
						} else if (y2 == y - 1) {
							overlap = true;
							break;
						}
					}
				}
				if (!overlap) {
					auto& player = Main_Data::game_player;
					if (x == player->GetX()) {
						if (y == 0) {
							if (Game_Map::LoopVertical() && player->GetY() == Game_Map::GetTilesY() - 1) {
								overlap = true;
							}
						} else if (player->GetY() == y - 1) {
							overlap = true;
						}
					}
				}
				auto name_tag = p.second.name_tag.get();
				if (name_tag)
					name_tag->SetTransparent(overlap);
			}
		}

		if (!switching_room && !switched_room) {
			switched_room = true;
		}
	}

	if (!dc_players.empty()) {
		auto scene_map = Scene::Find(Scene::SceneType::Map);
		if (!scene_map) {
			Output::Error("MP: unexpected, {}:{}", __FILE__, __LINE__);
			return;
		}

		auto old_list = &DrawableMgr::GetLocalList();
		DrawableMgr::SetLocalList(&scene_map->GetDrawableList());

		for (auto dcpi = dc_players.rbegin(); dcpi != dc_players.rend();) {
			auto& ch = dcpi->ch;
			if (ch->GetBaseOpacity() > 0) {
				ch->SetBaseOpacity(ch->GetBaseOpacity() - 1);
				ch->SetProcessed(false);
				ch->Update();
				dcpi->sprite->Update();
				++dcpi;
			} else {
				dcpi = decltype(dcpi)(dc_players.erase(dcpi.base() - 1));
			}
		}

		DrawableMgr::SetLocalList(old_list);
	}

	if (connection.IsConnected())
		connection.FlushQueue();
}
