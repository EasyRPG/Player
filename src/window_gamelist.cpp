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

void Window_GameList::Refresh() {
	tree = FileFinder::CreateDirectoryTree(Main_Data::project_path, false);
	game_directories.clear();

	// Find valid game diectories
	for (auto dir : tree.get()->directories) {
		EASYRPG_SHARED_PTR<FileFinder::DirectoryTree> subtree = FileFinder::CreateDirectoryTree(FileFinder::MakePath(Main_Data::project_path, dir.second), false);
		if (FileFinder::IsValidProject(*subtree)) {
			game_directories.push_back(dir.second);
		}
	}

	if (HasValidGames()) {
		item_max = game_directories.size();

		CreateContents();

		contents->Clear();

		for (int i = 0; i < item_max; ++i) {
			DrawItem(i);
		}
	}
	else {
		SetContents(Bitmap::Create(width - 16, height - 16));

		DrawErrorText();
	}
}

void Window_GameList::DrawItem(int index) {
	Rect rect = GetItemRect(index);
	contents->ClearRect(rect);
	
	std::string text;

	if (HasValidGames()) {
		text = game_directories[index];
	}

	contents->TextDraw(rect.x, rect.y, Font::ColorDefault, game_directories[index]);
}

void Window_GameList::DrawErrorText() {
	std::vector<std::string> error_msg = {
		"Games must be in a direct subdirectory",
		"and must have the files RPG_RT.ldb and",
		"RPG_RT.lmt in their main directory.",
		"",
		"This engine only supports RPG Maker 2000",
		"and 2003 games.",
		"",
		"RPG Maker XP, VX, VX Ace and MV are NOT",
		"supported."
	};

	contents->TextDraw(0, 0, Font::ColorKnockout, "No games found in the current directory");
	
	for (size_t i = 0; i < error_msg.size(); ++i) {
		contents->TextDraw(0, 12 * (i + 2), Font::ColorCritical, error_msg[i]);
	}
}

bool Window_GameList::HasValidGames()
{
	return !game_directories.empty();
}

std::string Window_GameList::GetGamePath() {
	return FileFinder::MakePath(tree.get()->directory_path, game_directories[GetIndex()]);
}
