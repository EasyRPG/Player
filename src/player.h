/////////////////////////////////////////////////////////////////////////////
// This file is part of EasyRPG Player.
//
// EasyRPG Player is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// EasyRPG Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
/////////////////////////////////////////////////////////////////////////////

#ifndef _PLAYER_H_
#define _PLAYER_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "baseui.h"

////////////////////////////////////////////////////////////
/// Player namespace.
////////////////////////////////////////////////////////////
namespace Player {
	enum EngineType {
		EngineRpg2k,
		EngineRpg2k3
	};

	////////////////////////////////////////////////////////
	/// Initialize EasyRPG player.
	////////////////////////////////////////////////////////
	void Init(int argc, char *argv[]);

	////////////////////////////////////////////////////////
	/// Run the game engine.
	////////////////////////////////////////////////////////
	void Run();

	////////////////////////////////////////////////////////
	/// Pause the game engine.
	////////////////////////////////////////////////////////
	void Pause();

	////////////////////////////////////////////////////////
	/// Resume the game engine.
	////////////////////////////////////////////////////////
	void Resume();

	////////////////////////////////////////////////////////
	/// Update EasyRPG player.
	////////////////////////////////////////////////////////
	void Update();

	////////////////////////////////////////////////////////
	/// Exit EasyRPG player.
	////////////////////////////////////////////////////////
	void Exit();

	/// Exit flag, if true will exit application on next Player::Update.
	extern bool exit_flag;

	/// Reset flag, if true will restart game on next Player::Update.
	extern bool reset_flag;

	/// Debug flag, if true will run game in debug mode.
	extern bool debug_flag;

	/// Hide Title flag, if true title scene will run without image and music.
	extern bool hide_title_flag;

	/// Window flag, if true will run in window mode instead of full screen.
	extern bool window_flag;

	/// Battle Test flag, if true will run battle test.
	extern bool battle_test_flag;

	/// Battle Test Troop ID to fight with if battle test is run.
	extern int battle_test_troop_id;

	/// Currently interpreted engine
	extern EngineType engine;
}

#endif
