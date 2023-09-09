#ifndef EP_GAME_MULTIPLAYER_H
#define EP_GAME_MULTIPLAYER_H

#include <string>
#include <bitset>
#include "../string_view.h"
#include "../game_config.h"
#include "../game_pictures.h"
#include "../tone.h"
#include <lcf/rpg/sound.h>
#include "client_connection.h"

class PlayerOther;

class Game_Multiplayer {
public:
	static Game_Multiplayer& Instance();

	Game_Multiplayer();

	void SetConfig(const Game_ConfigMultiplayer& cfg);
	Game_ConfigMultiplayer GetConfig() const;
	void SetChatName(std::string chat_name);
	std::string GetChatName();
	void SetRemoteAddress(std::string address);
	void Connect();
	void Disconnect();
	void SwitchRoom(int map_id, bool room_switch = false);
	void Reset();
	void MapQuit();
	void Quit();
	void Update();
	void MapUpdate();
	void SendChatMessage(int visibility, std::string message);
	void SendBasicData();
	void MainPlayerMoved(int dir);
	void MainPlayerFacingChanged(int dir);
	void MainPlayerChangedMoveSpeed(int spd);
	void MainPlayerChangedSpriteGraphic(std::string name, int index);
	void MainPlayerJumped(int x, int y);
	void MainPlayerFlashed(int r, int g, int b, int p, int f);
	void MainPlayerChangedSpriteHidden(bool hidden);
	void MainPlayerTeleported(int map_id, int x, int y);
	void MainPlayerTriggeredEvent(int event_id, bool action);
	void SystemGraphicChanged(StringView sys);
	void SePlayed(const lcf::rpg::Sound& sound);
	bool IsPictureSynced(int pic_id, Game_Pictures::ShowParams& params);
	void PictureShown(int pic_id, Game_Pictures::ShowParams& params);
	void PictureMoved(int pic_id, Game_Pictures::MoveParams& params);
	void PictureErased(int pic_id);
	bool IsBattleAnimSynced(int anim_id);
	void PlayerBattleAnimShown(int anim_id);
	void ApplyPlayerBattleAnimUpdates();
	void ApplyFlash(int r, int g, int b, int power, int frames);
	void ApplyRepeatingFlashes();
	void ApplyTone(Tone tone);
	void ApplyScreenTone();
	void SwitchSet(int switch_id, int value);
	void VariableSet(int var_id, int value);

	Game_ConfigMultiplayer cfg;

	struct {
		bool enable_sounds{ true };
		bool mute_audio{ false };
		int moving_queue_limit{ 4 };
	} settings;

	ClientConnection connection;
	bool reconnect_wait{ false };
	bool active{ false }; // if true, it will automatically reconnect when disconnected
	bool switching_room{ true }; // when client enters new room, but not synced to server
	bool switched_room{ false }; // determines whether new connected players should fade in
	int room_id{-1};
	int frame_index{-1};

	enum class NametagMode {
		NONE,
		CLASSIC,
		COMPACT,
		SLIM
	};

	NametagMode GetNametagMode() { return nametag_mode; }
	void SetNametagMode(int mode) {
		nametag_mode = static_cast<NametagMode>(mode);
	}
	NametagMode nametag_mode{NametagMode::CLASSIC};

	std::map<int, std::string> global_players_system;
	std::map<int, PlayerOther> players;
	std::vector<PlayerOther> dc_players; // disconnect and player fade
	std::vector<int> sync_switches;
	std::vector<int> sync_vars;
	std::vector<int> sync_events;
	std::vector<int> sync_action_events;
	std::vector<std::string> sync_picture_names; // for badge conditions
	std::vector<std::string> global_sync_picture_names;
	std::vector<std::string> global_sync_picture_prefixes;
	std::map<int, bool> sync_picture_cache;
	std::vector<int> sync_battle_anim_ids;
	int last_flash_frame_index{-1};
	std::unique_ptr<std::array<int, 5>> last_frame_flash;
	std::map<int, std::array<int, 5>> repeating_flashes;
	std::shared_ptr<int> sys_graphic_request_id;

	void SpawnOtherPlayer(int id);
	void ResetRepeatingFlash();
	void InitConnection();
};

inline Game_Multiplayer& GMI() { return Game_Multiplayer::Instance(); }

#endif
