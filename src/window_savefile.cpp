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
#include <string>
#include <iomanip>
#include <sstream>
#include "window_savefile.h"
#include "game_system.h"
#include "input.h"
#include "bitmap.h"
#include "font.h"
#include "player.h"

Window_SaveFile::Window_SaveFile(Scene* parent, int ix, int iy, int iwidth, int iheight) :
	Window_Base(parent, ix, iy, iwidth, iheight) {

	SetBorderX(4);
	SetContents(Bitmap::Create(width - 8, height - 16));

	Refresh();
	UpdateCursorRect();
}

void Window_SaveFile::UpdateCursorRect() {
	Rect rect = Rect();

	if (GetActive()) {
		if (override_index > 0) {
			rect = Rect(0, 0, Text::GetSize(*Font::Default(), GetSaveFileName()).width + 6, 16);
		} else {
			rect = Rect(0, 0, Text::GetSize(*Font::Default(), GetSaveFileName()).width + Text::GetSize(*Font::Default(), " ").width * 5 / 2 + 8, 16);
		}
	}

	SetCursorRect(rect);
}

std::string Window_SaveFile::GetSaveFileName() const {
	std::ostringstream out;
	if (!override_name.empty()) {
		if (override_name.size() > 14 && has_party) {
			out << override_name.substr(0, 11) << "...";
		} else {
			out << override_name;
		}
	} else {
		out << lcf::Data::terms.file;
	}
	return out.str();
}

void Window_SaveFile::SetIndex(int id) {
	index = id;
}

void Window_SaveFile::SetDisplayOverride(const std::string& name, int index) {
	override_name = name;
	override_index = index;
}

void Window_SaveFile::SetParty(lcf::rpg::SaveTitle title) {
	data = std::move(title);
	has_party = true;
}

void Window_SaveFile::SetCorrupted(bool corrupted) {
	this->corrupted = corrupted;
}

bool Window_SaveFile::IsValid() const {
	return has_save && !corrupted;
}

bool Window_SaveFile::HasParty() const {
	return has_party;
}

void Window_SaveFile::SetHasSave(bool valid) {
	this->has_save = valid;
}

void Window_SaveFile::Refresh() {
	contents->Clear();

	Font::SystemColor fc = has_save ? Font::ColorDefault : Font::ColorDisabled;

	contents->TextDraw(4, 2, fc, GetSaveFileName());
	contents->TextDraw(4 + Text::GetSize(*Font::Default(), GetSaveFileName()).width, 2, fc, " ");

	std::stringstream out;
	out << std::setw(2) << std::setfill(' ') << index + 1;
	contents->TextDraw(4 + Text::GetSize(*Font::Default(), GetSaveFileName()).width + Text::GetSize(*Font::Default(), " ").width / 2, 2, fc, out.str());

	if (corrupted) {
		contents->TextDraw(4, 16 + 2, Font::ColorKnockout, "Savegame corrupted");
		return;
	}

	if (!has_party) {
		return;
	}

	out.str("");
	if (override_index > 0) {
		out << lcf::Data::terms.file << std::setw(3) << std::setfill(' ') << override_index;
		contents->TextDraw(4, 16+2, fc, out.str());
	} else {
		contents->TextDraw(4, 16 + 2, fc, data.hero_name);
	}

	auto lvl_short = ToString(lcf::Data::terms.lvl_short);
	if (lvl_short.size() != 2) {
		lvl_short.resize(2, ' ');
	}

	contents->TextDraw(4, 32 + 2, 1, lvl_short);

	int lx = Text::GetSize(*Font::Default(), lvl_short).width;
	out.str("");
	out << std::setw(2) << std::setfill(' ') << data.hero_level;
	contents->TextDraw(4 + lx, 32 + 2, fc, out.str());

	auto hp_short = ToString(lcf::Data::terms.hp_short);
	if (hp_short.size() != 2) {
		hp_short.resize(2, ' ');
	}

	contents->TextDraw(46, 32 + 2, 1, hp_short);

	int hx = Text::GetSize(*Font::Default(), hp_short).width;
	out.str("");
	out << std::setw(Player::IsRPG2k3() ? 4 : 3) << std::setfill(' ') << data.hero_hp;
	contents->TextDraw(46 + hx, 32 + 2, fc, out.str());

	int i = 0;
	DrawFace(data.face1_name, data.face1_id, 92 + i++ * 56, 0, false);
	DrawFace(data.face2_name, data.face2_id, 92 + i++ * 56, 0, false);
	DrawFace(data.face3_name, data.face3_id, 92 + i++ * 56, 0, false);
	DrawFace(data.face4_name, data.face4_id, 92 + i++ * 56, 0, false);
}

void Window_SaveFile::Update() {
	Window_Base::Update();
	UpdateCursorRect();
}
