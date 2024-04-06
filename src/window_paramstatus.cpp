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
#include "game_actors.h"
#include "window_paramstatus.h"
#include "bitmap.h"
#include "font.h"

Window_ParamStatus::Window_ParamStatus(Scene* parent, int ix, int iy, int iwidth, int iheight, int actor_id) :
	Window_Base(parent, ix, iy, iwidth, iheight),
	actor_id(actor_id)
{

	SetContents(Bitmap::Create(width - 16, height - 16));

	Refresh();
}

void Window_ParamStatus::Refresh() {
	contents->Clear();

	auto* actor = Main_Data::game_actors->GetActor(actor_id);

	auto draw = [this](int y, StringView name, int value) {
		// Draw Term
		contents->TextDraw(0, y, 1, name);

		// Draw Value
		contents->TextDraw(90, y, Font::ColorDefault, std::to_string(value), Text::AlignRight);
		return y + 16;
	};

	int y = 2;
	y = draw(y, lcf::Data::terms.attack, actor->GetAtk());
	y = draw(y, lcf::Data::terms.defense, actor->GetDef());
	y = draw(y, lcf::Data::terms.spirit, actor->GetSpi());
	y = draw(y, lcf::Data::terms.agility, actor->GetAgi());
}

