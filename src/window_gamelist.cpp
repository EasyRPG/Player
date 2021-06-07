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

// Headers
#include <iomanip>
#include <sstream>
#include "window_gamelist.h"
#include "game_party.h"
#include "bitmap.h"
#include "font.h"

Window_GameList::Window_GameList(int ix, int iy, int iwidth, int iheight) :
	Window_Selectable(ix, iy, iwidth, iheight) {
	column_max = 1;
}

bool Window_GameList::Refresh(FilesystemView filesystem_base, bool show_dotdot) {
	base_fs = filesystem_base;
	if (!base_fs) {
		return false;
	}

	game_directories.clear();

	this->show_dotdot = show_dotdot;

	auto files = base_fs.ListDirectory();

	// Find valid game diectories
	for (auto& dir : *files) {
		assert(!dir.second.name.empty() && "VFS BUG: Empty filename in the folder");

		if (StringView(dir.second.name).ends_with(".save")) {
			continue;
		}
		if (dir.second.type == DirectoryTree::FileType::Regular) {
			auto sv = StringView(dir.second.name);
			if (sv.ends_with(".zip") || sv.ends_with(".easyrpg")) {
				game_directories.emplace_back(dir.second.name);
			}
		} else if (dir.second.type == DirectoryTree::FileType::Directory) {
			game_directories.emplace_back(dir.second.name);
		}
	}

	// Sort game list in place
	std::sort(game_directories.begin(), game_directories.end(),
			  [](const std::string& s, const std::string& s2) {
				  return strcmp(Utils::LowerCase(s).c_str(), Utils::LowerCase(s2).c_str()) <= 0;
			  });

	if (show_dotdot) {
		game_directories.insert(game_directories.begin(), "..");
	}

	if (HasValidEntry()) {
		item_max = game_directories.size();

		CreateContents();

		contents->Clear();

		for (int i = 0; i < item_max; ++i) {
			DrawItem(i);
		}
	}
	else {
		item_max = 1;

		SetContents(Bitmap::Create(width - 16, height - 16));

		if (show_dotdot) {
			DrawItem(0);
		}

		DrawErrorText();
	}

	return true;
}

void Window_GameList::DrawItem(int index) {
	Rect rect = GetItemRect(index);
	contents->ClearRect(rect);

	std::string text;

	if (HasValidEntry()) {
		text = game_directories[index];
	}

	contents->TextDraw(rect.x, rect.y, Font::ColorDefault, game_directories[index]);
}

void Window_GameList::DrawErrorText() {
	std::vector<std::string> error_msg = {
#ifdef EMSCRIPTEN
		"Did you type in a wrong URL?",
		"",
		"If you followed a link and stranded here",
		"please notify us (see About page)."
#else
		"Games must be in a direct subdirectory",
		"and must have the files RPG_RT.ldb and",
		"RPG_RT.lmt in their main directory.",
		"",
		"This engine only supports RPG Maker 2000",
		"and 2003 games.",
		"",
		"RPG Maker XP, VX, VX Ace and MV are NOT",
		"supported."
#endif
	};

#ifdef EMSCRIPTEN
	contents->TextDraw(0, 0, Font::ColorKnockout, "The game was not found.");
#else
	contents->TextDraw(0, 4 + 14, Font::ColorKnockout, "No games found in the current directory.");
#endif

	for (size_t i = 0; i < error_msg.size(); ++i) {
		contents->TextDraw(0, 4 + 14 * (i + 3), Font::ColorCritical, error_msg[i]);
	}
}

bool Window_GameList::HasValidEntry() {
	size_t minval = show_dotdot ? 1 : 0;
	return game_directories.size() > minval;
}

std::pair<FilesystemView, std::string> Window_GameList::GetGameFilesystem() const {
	return { base_fs.Create(game_directories[GetIndex()]), game_directories[GetIndex()] };
}
