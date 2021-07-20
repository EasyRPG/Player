#include <map>
#include <memory>
#include <queue>
#include <emscripten/emscripten.h>
#include <emscripten/websocket.h>

#include "game_multiplayer.h"
#include "output.h"
#include "game_player.h"
#include "sprite_character.h"
#include "window_base.h"
#include "drawable_mgr.h"
#include "scene.h"
#include "bitmap.h"
#include "font.h"
#include "input.h"

namespace {
	std::unique_ptr<Window_Base> conn_status_window;
	const std::string server_url = "wss://dry-lowlands-62918.herokuapp.com/";
	EMSCRIPTEN_WEBSOCKET_T socket;
	bool connected = false;
	int myid = -1;
	//std::mutex plrs_mx;
	std::map<int,std::queue<std::pair<int,int>>> player_mvq; //queue of move commands bc sometimes we may receive a move command while the player is being animated
	std::map<int,std::unique_ptr<Game_PlayerOther>> players;
	std::map<int,std::unique_ptr<Sprite_Character>> player_sprites;

	void SetConnStatusWindowText(std::string s) {
		conn_status_window->GetContents()->Clear();
		conn_status_window->GetContents()->TextDraw(0, 0, Font::ColorDefault, s);
	}

	void SpawnOtherPlayer(int id) {
		auto& player = Main_Data::game_player;
		auto& nplayer = players[id];
		nplayer = std::make_unique<Game_PlayerOther>();
		nplayer->SetX(player->GetX());
		nplayer->SetY(player->GetY());
		nplayer->SetSpriteGraphic(player->GetSpriteName(), player->GetSpriteIndex());
		nplayer->SetMoveSpeed(player->GetMoveSpeed());
		nplayer->SetMoveFrequency(player->GetMoveFrequency());
		nplayer->SetThrough(true);
		nplayer->SetLayer(player->GetLayer());

		auto scene_map = Scene::Find(Scene::SceneType::Map);
		if (scene_map == nullptr) Output::Debug("unexpected");
		auto old_list = &DrawableMgr::GetLocalList();
		DrawableMgr::SetLocalList(&scene_map->GetDrawableList());
		player_sprites[id] = std::make_unique<Sprite_Character>(nplayer.get());
		Output::Debug("Newp z={}", player_sprites[id]->GetZ());
		DrawableMgr::SetLocalList(old_list);
	}
	void SendMainPlayerPos() {
		unsigned short ready;
		emscripten_websocket_get_ready_state(socket, &ready);
		if (connected && ready == 1) { //1 means OPEN
			auto& player = Main_Data::game_player;
			std::string msg = "m " + std::to_string(player->GetX()) + " " + std::to_string(player->GetY());
			emscripten_websocket_send_binary(socket, (void*)msg.c_str(), msg.length());
		}
	}

	//this assumes that the player is stopped
	void MovePlayerToPos(std::unique_ptr<Game_PlayerOther> &player, int x, int y) {
		if (!player->IsStopping()) {
			Output::Debug("MovePlayerToPos unexpected error: the player is busy being animated");
		}
		int dx = x - player->GetX();
		int dy = y - player->GetY();
		if (dx == 0 && dy == 1) {
			player->Move(Game_Character::Direction::Down);
		}
		else if (dx == 0 && dy == -1) {
			player->Move(Game_Character::Direction::Up);
		}
		else if (dy == 0 && dx == 1) {
			player->Move(Game_Character::Direction::Right);
		}
		else if (dy == 0 && dx == -1) {
			player->Move(Game_Character::Direction::Left);
		}
		else { //if we can't animate the player
			player->SetX(x);
			player->SetY(y);
		}
	}
	EM_BOOL onopen(int eventType, const EmscriptenWebSocketOpenEvent *websocketEvent, void *userData) {
		SetConnStatusWindowText("Connected");
		//puts("onopen");
		connected = true;
		SendMainPlayerPos();

		return EM_TRUE;
	}
	EM_BOOL onclose(int eventType, const EmscriptenWebSocketCloseEvent *websocketEvent, void *userData) {
		SetConnStatusWindowText("Disconnected");
		//puts("onclose");
		connected = false;

		return EM_TRUE;
	}
	EM_BOOL onmessage(int eventType, const EmscriptenWebSocketMessageEvent *websocketEvent, void *userData) {
		//puts("onmessage");
		if (websocketEvent->isText) {
			// For only ascii chars.
			//printf("message: %s\n", websocketEvent->data);
			std::string s = (const char*)websocketEvent->data;
			Output::Debug("msg={}", s);
			//split by space
			std::stringstream ss(s);
			std::vector<std::string> v;
			//Output::Debug("msg={}", s);
			while (ss >> s) {
				v.push_back(s);
			}
			//Output::Debug("msg flagsize {}", v.size());
			if (v[0] == "s") { //set your id command
				myid = std::stoi(v[1]);
			}
			if (std::stoi(v[1]) != myid) {
				//plrs_mx.lock();
				if (v[0] == "c") { //connect command
					SpawnOtherPlayer(std::stoi(v[1]));
				}
				else if (v[0] == "d") { //disconnect command
					players.erase(std::stoi(v[1]));
					player_sprites.erase(std::stoi(v[1]));
					player_mvq.erase(std::stoi(v[1]));
				}
				else if (v[0] == "m") { //move command
					//if we get a move command for a player that we don't know of, probably packet loss occurred so spawn the player
					if (players.count(std::stoi(v[1])) == 0) {
						SpawnOtherPlayer(std::stoi(v[1]));
					}
					player_mvq[std::stoi(v[1])].push(std::make_pair(std::stoi(v[2]), std::stoi(v[3])));
				}
				//plrs_mx.unlock();
			}
		}

		return EM_TRUE;
	}
}

void Game_Multiplayer::Connect(int map_id) {
	Game_Multiplayer::Quit();
	//if the window doesn't exist (first map loaded) then create it
	//else, if the window is visible recreate it
	if (conn_status_window.get() == nullptr || conn_status_window->IsVisible()) {
		conn_status_window = std::make_unique<Window_Base>(0, SCREEN_TARGET_HEIGHT-30, 100, 30);
		conn_status_window->SetContents(Bitmap::Create(100, 30));
		conn_status_window->SetZ(2106632960);
	}
	SetConnStatusWindowText("Disconnected");
	std::string room_url = server_url + std::to_string(map_id);
	Output::Debug(room_url);
	EmscriptenWebSocketCreateAttributes ws_attrs = {
		room_url.c_str(),
		"binary",
		EM_TRUE
	};

	socket = emscripten_websocket_new(&ws_attrs);
	emscripten_websocket_set_onopen_callback(socket, NULL, onopen);
	//emscripten_websocket_set_onerror_callback(socket, NULL, onerror);
	emscripten_websocket_set_onclose_callback(socket, NULL, onclose);
	emscripten_websocket_set_onmessage_callback(socket, NULL, onmessage);
}

void Game_Multiplayer::Quit() {
	emscripten_websocket_deinitialize(); //kills every socket for this thread
	players.clear();
	player_sprites.clear();
	player_mvq.clear();
}

void Game_Multiplayer::MainPlayerMoved(int dir) {
	SendMainPlayerPos();
}

void Game_Multiplayer::MainPlayerChangedMoveSpeed(int spd) {
	Output::Debug("main player newspd={}", spd);
}

void Game_Multiplayer::Update() {
	for (auto& p : players) {
		auto& q = player_mvq[p.first];
		if (!q.empty() && p.second->IsStopping()) {
			MovePlayerToPos(p.second, q.front().first, q.front().second);
			q.pop();
		}
		p.second->SetProcessed(false);
		p.second->Update();
	}
	for (auto& p : player_sprites) {
		p.second->Update();
	}
	//plrs_mx.unlock();
	if (Input::IsReleased(Input::InputButton::N3)) {
		conn_status_window->SetVisible(!conn_status_window->IsVisible());
	}
}
