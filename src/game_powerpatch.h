/*
 * This file is part of EasyRPG Player.
 *
 * EasyRPG Player is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * EasyRPG Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef EP_GAME_POWERPATCH_H
#define EP_GAME_POWERPATCH_H

// Headers
#include <string>

#include "async_op.h"
#include "game_ineluki.h"
#include "string_view.h"
#include "span.h"

namespace Game_PowerPatch {
	enum class PPC_CommandType {
		Debug,
		Quit,
		Restart,
		Save,
		Load,
		CheckSave,
		CopySave,
		DeleteSave,
		GetSaveDateTime,
		GetSystemDateTime,
		SetGlobalBrightness,
		PauseTimer,
		ChangeScene,
		CallLoadMenu,
		CallSaveMenu,
		CallGameMenu,
		CallTitleScreen,
		SimulateKeyPress,
		ChangeFunctionKey,
		SetTitleBGM,
		SetTitleScreen,
		SetGameOverScreen,
		UnlockPictures,
		SetMsgBoxColor,
		PauseGame,
		SetVar,
		LAST
	};

	AsyncOp ExecutePPC(std::string_view pcc_cmd, Span<std::string const> args);

	bool Execute(PPC_CommandType command, Span<std::string const> args, AsyncOp& async_op);

	struct PPC_Command {
		PPC_CommandType type;
		uint8_t min_args;
		const char* name;
	};

	static constexpr std::array<PPC_Command, static_cast<size_t>(PPC_CommandType::LAST)> PPC_commands = {{
		{ PPC_CommandType::Debug,               0, "DEBUG" },
		{ PPC_CommandType::Quit,                0, "QUIT" },
		{ PPC_CommandType::Restart,             0, "RESTART" },
		{ PPC_CommandType::Save,                0, "SAVE" },
		{ PPC_CommandType::Load,                0, "LOAD" },
		{ PPC_CommandType::CheckSave,           2, "CHECKSAVE" },
		{ PPC_CommandType::CopySave,            2, "COPYSAVE" },
		{ PPC_CommandType::DeleteSave,          1, "DELETESAVE" },
		{ PPC_CommandType::GetSaveDateTime,     2, "GETSAVEDATETIME" },
		{ PPC_CommandType::GetSystemDateTime,   1, "GETSYSTEMDATETIME" },
		{ PPC_CommandType::SetGlobalBrightness, 1, "SETGLOBALBRIGHTNESS" },
		{ PPC_CommandType::PauseTimer,          1, "PAUSETIMER" },
		{ PPC_CommandType::ChangeScene,         1, "CHANGESCENE" },
		{ PPC_CommandType::CallLoadMenu,        0, "CALLLOADMENU" },
		{ PPC_CommandType::CallSaveMenu,        0, "CALLSAVEMENU" },
		{ PPC_CommandType::CallGameMenu,        0, "CALLGAMEMENU" },
		{ PPC_CommandType::CallTitleScreen,     0, "CALLTITLESCREEN" },
		{ PPC_CommandType::SimulateKeyPress,    2, "SIMULATEKEYPRESS" },
		{ PPC_CommandType::ChangeFunctionKey,   0, "CHANGEFUNCTIONKEY" },
		{ PPC_CommandType::SetTitleBGM,         1, "SETTITLEBGM" },
		{ PPC_CommandType::SetTitleScreen,      1, "SETTITLESCREEN" },
		{ PPC_CommandType::SetGameOverScreen,   1, "SETGAMEOVERSCREEN" },
		{ PPC_CommandType::UnlockPictures,      1, "UNLOCKPICTURES" },
		{ PPC_CommandType::SetMsgBoxColor,      4, "SETMSGBOXCOLOR" },
		{ PPC_CommandType::PauseGame,           0, "PAUSEGAME" },
		{ PPC_CommandType::SetVar,              0, "SETVAR" }
	}};

	/**
	* Map of simulated keypresses handled via ppcomp
	* 'int' value refers to remaining frames
	*/
	extern std::map<Input::Keys::InputKey, int> simulate_keypresses;
};

#endif
