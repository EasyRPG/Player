#include "mock_game.h"
#include "game_actors.h"
#include "game_system.h"

static lcf::rpg::Terrain MakeTerrain() {
	return {};
}

static lcf::rpg::Chipset MakeChipset() {
	lcf::rpg::Chipset chipset;
	chipset.passable_data_lower.resize(162, 0xF);
	chipset.passable_data_lower[BLOCK_E_INDEX + 1] = 0;
	chipset.passable_data_upper.resize(162, 0xF);
	chipset.terrain_data.resize(144, 1);

	return chipset;
}

MockGame::MockGame(MockMap maptag) {
	Input::ResetKeys();

	lcf::Data::terrains.push_back(MakeTerrain());
	lcf::Data::chipsets.push_back(MakeChipset());

	Main_Data::game_actors = std::make_unique<Game_Actors>();
	Main_Data::game_party = std::make_unique<Game_Party>();

	auto& treemap = lcf::Data::treemap;
	treemap = {};
	treemap.maps.push_back(lcf::rpg::MapInfo());
	treemap.maps.back().type = lcf::rpg::TreeMap::MapType_root;
	for (int i = 1; i < int(MockMap::eMapCount); ++i) {
		treemap.maps.push_back(lcf::rpg::MapInfo());
		treemap.maps.back().ID = i;
		treemap.maps.back().type = lcf::rpg::TreeMap::MapType_map;
	}

	Game_Map::Init();
	Main_Data::game_system = std::make_unique<Game_System>();
	Main_Data::game_switches = std::make_unique<Game_Switches>();
	Main_Data::game_variables = std::make_unique<Game_Variables>(Game_Variables::min_2k3, Game_Variables::max_2k3);
	Main_Data::game_pictures = std::make_unique<Game_Pictures>();
	Main_Data::game_screen = std::make_unique<Game_Screen>();
	Main_Data::game_player = std::make_unique<Game_Player>();
	Main_Data::game_player->SetMapId(1);

	Game_Map::Setup(MakeMockMap(maptag));
}

Game_Player* MockGame::GetPlayer() {
	return Main_Data::game_player.get();
}

Game_Vehicle* MockGame::GetVehicle(Game_Vehicle::Type veh) {
	return Game_Map::GetVehicle(veh);
}

Game_Event* MockGame::GetEvent(int id) {
	return Game_Map::GetEvent(id);
}

void MockGame::Reset() {
	if (_active) {
		DoReset();
		_active = false;
	}
}

void MockGame::DoReset() {
	Main_Data::game_switches = {};
	Main_Data::game_variables = {};
	Main_Data::game_player = {};
	Main_Data::game_screen = {};
	Main_Data::game_pictures = {};
	Game_Map::Quit();
	lcf::Data::data = {};

	Main_Data::game_party.reset();
	Input::ResetKeys();
}

std::unique_ptr<lcf::rpg::Map> MakeMockMap(MockMap maptag) {
	auto map = std::make_unique<lcf::rpg::Map>();

	auto w = 20;
	auto h = 15;

	switch (maptag) {
		case MockMap::ePass40x30:
			w = 40;
			h = 30;
		default:
			break;
	}

	// FIXME: Add a SetSize(w, h) method?
	map->width = w;
	map->height = h;
	map->upper_layer.resize(w * h, BLOCK_F);
	map->lower_layer.resize(w * h, BLOCK_E);

	map->events.push_back({});
	map->events.back().ID = 1;
	map->events.back().pages.push_back({});
	map->events.back().pages.back().ID = 1;
	map->events.back().pages.back().move_type = lcf::rpg::EventPage::MoveType_stationary;
	map->events.back().pages.back().character_pattern = 1;

	switch (maptag) {
		case MockMap::eNone:
		case MockMap::eMapCount:
		case MockMap::ePass40x30:
			break;
		case MockMap::ePassBlock20x15:
			for (int y = 0; y < h; ++y) {
				for (int x = 0; x < w; ++x) {
					auto& tile = map->lower_layer[y * w + x];
					tile = (x < w / 2) ? BLOCK_E : BLOCK_E + 1;
				}
			}
			break;
	}

	return map;
}
