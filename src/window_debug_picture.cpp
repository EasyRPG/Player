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

#include "window_debug_picture.h"
#include "game_pictures.h"
#include "game_windows.h"
#include "main_data.h"
#include "bitmap.h"
#include "font.h"
#include "utils.h"
#include <iomanip>
#include <sstream>
#include <fmt/format.h>
#include <lcf/rpg/savepicture.h>
#include <lcf/rpg/saveeasyrpgwindow.h>

Window_DebugPictureList::Window_DebugPictureList(int x, int y, int w, int h) :
	Window_Selectable(x, y, w, h)
{
	SetMenuItemHeight(16);
	SetColumnMax(1);
	SetContents(Bitmap::Create(width, height));
	Refresh();
}

void Window_DebugPictureList::Refresh() {
	picture_ids.clear();

	// Scan all allocated pictures
	for (int i = 1; ; ++i) {
		auto* pic = Main_Data::game_pictures->GetPicturePtr(i);
		if (!pic) {
			break;
		}

		if (pic->Exists() || pic->IsWindowAttached()) {
			picture_ids.push_back(i);
		}
	}

	item_max = picture_ids.size();

	int required_height = std::max(height - 32, (int)(item_max * menu_item_height));
	if (contents->GetHeight() != required_height) {
		SetContents(Bitmap::Create(contents->GetWidth(), required_height));
	}

	contents->Clear();

	if (item_max == 0) {
		contents->TextDraw(0, 0, Font::ColorDisabled, "No Pic");
	}
	else {
		for (int i = 0; i < item_max; ++i) {
			Rect rect = GetItemRect(i);
			int id = picture_ids[i];
			auto* pic = Main_Data::game_pictures->GetPicturePtr(id);

			std::string suffix = "";
			if (pic->IsWindowAttached()) suffix = " T"; // Text/String

			std::string text = fmt::format("{:04d}{}", id, suffix);
			contents->TextDraw(rect.x, rect.y, Font::ColorDefault, text);
		}
	}
}

int Window_DebugPictureList::GetPictureId() const {
	if (index >= 0 && index < static_cast<int>(picture_ids.size())) {
		return picture_ids[index];
	}
	return 0;
}

// ---------------------------------------------------------------------------

Window_DebugPictureInfo::Window_DebugPictureInfo(int x, int y, int w, int h) :
	Window_Base(x, y, w, h)
{
	SetContents(Bitmap::Create(width, height));
}

void Window_DebugPictureInfo::SetPictureId(int id) {
	if (picture_id != id) {
		picture_id = id;
		Refresh();
	}
}

int Window_DebugPictureInfo::DrawLine(int y, std::string_view label, std::string_view value) {
	contents->TextDraw(0, y, Font::ColorDefault, label);
	int val_x = 40;
	contents->TextDraw(val_x, y, Font::ColorHeal, value);
	return y + 16;
}

int Window_DebugPictureInfo::DrawDualLine(int y, std::string_view l1, std::string_view v1, std::string_view l2, std::string_view v2) {
	contents->TextDraw(0, y, Font::ColorDefault, l1);
	contents->TextDraw(40, y, Font::ColorHeal, v1);

	contents->TextDraw(110, y, Font::ColorDefault, l2);
	contents->TextDraw(150, y, Font::ColorHeal, v2);
	return y + 16;
}

int Window_DebugPictureInfo::DrawSeparator(int y) {
	// Draw a dim line
	Color col;
	col.alpha = 128;
	contents->FillRect(Rect(0, y + 7, contents->GetWidth(), 1), col);
	return y + 16;
}

int Window_DebugPictureInfo::DrawFlags(int y, const std::vector<FlagInfo>& flags) {
	int x = 0;
	int row_start_y = y;

	for (const auto& flag : flags) {
		int w = Text::GetSize(*Font::Default(), flag.name).width + 4;
		if (x + w > contents->GetWidth()) {
			x = 0;
			y += 16;
		}

		// Draw bracketed flag like [FlipX]
		std::string text = fmt::format("[{}]", flag.name);
		contents->TextDraw(x, y, flag.active ? Font::ColorHeal : Font::ColorDisabled, text);

		x += w + 12;
	}

	return y + 16;
}

void Window_DebugPictureInfo::Refresh() {
	contents->Clear();

	if (picture_id <= 0) {
		contents->TextDraw(0, 0, Font::ColorDisabled, "No Selection");
		return;
	}

	auto* pic = Main_Data::game_pictures->GetPicturePtr(picture_id);
	if (!pic) {
		contents->TextDraw(0, 0, Font::ColorCritical, "Invalid ID");
		return;
	}

	bool is_str = pic->IsWindowAttached();
	if (!pic->Exists() && !is_str) {
		contents->TextDraw(0, 0, Font::ColorDisabled, "Empty");
		return;
	}

	const auto& d = pic->data;
	int y = 0;

	// === COMMON PROPERTIES ===

	// ID & Type
	std::string type_str = is_str ? "String" : "Image";
	DrawDualLine(y, "ID", fmt::format("{}", picture_id), "Type", type_str);
	y += 16;

	// Position & Movement
	std::string pos_str = fmt::format("{:.0f},{:.0f}", d.current_x, d.current_y);
	y = DrawLine(y, "Pos", pos_str);

	if (d.time_left > 0 || d.current_x != d.finish_x || d.current_y != d.finish_y) {
		std::string goal_str = fmt::format("{:.0f},{:.0f} ({}f)", d.finish_x, d.finish_y, d.time_left);
		y = DrawLine(y, "Goal", goal_str);
	}

	// Scale
	std::string scale_str = fmt::format("{:.0f}", d.current_magnify);
	if (d.maniac_current_magnify_height != d.current_magnify) {
		scale_str += fmt::format(" / {:.0f}", d.maniac_current_magnify_height);
	}
	

	// Transparency
	std::string trans_str;
	if (d.current_top_trans == d.current_bot_trans) {
		trans_str = fmt::format("{:.0f}", d.current_top_trans);
	}
	else {
		trans_str = fmt::format("{:.0f}/{:.0f}", d.current_top_trans, d.current_bot_trans);
	}
	

	y = DrawDualLine(y, "Scale", scale_str + "%", "Trans", trans_str + "%");

	// Blend & Layer
	std::string blend = "None";
	if (d.easyrpg_blend_mode == 1) blend = "Multiply";
	if (d.easyrpg_blend_mode == 2) blend = "Addition";
	if (d.easyrpg_blend_mode == 3) blend = "Overlay";

	std::string layer = fmt::format("M:{} B:{}", d.map_layer, d.battle_layer);
	y = DrawDualLine(y, "Blend", blend, "Layer", layer);

	// Tone (R,G,B,S)
	std::string tone_str = fmt::format("{:.0f},{:.0f},{:.0f},{:.0f}", d.current_red, d.current_green, d.current_blue, d.current_sat);
	y = DrawLine(y, "Tone", tone_str);

	// Effects
	if (d.effect_mode != lcf::rpg::SavePicture::Effect_none) {
		std::string effect;
		switch (d.effect_mode) {
		case lcf::rpg::SavePicture::Effect_rotation: effect = "Rot"; break;
		case lcf::rpg::SavePicture::Effect_wave: effect = "Wave"; break;
		case lcf::rpg::SavePicture::Effect_maniac_fixed_angle: effect = "Ang"; break;
		}
		effect += fmt::format(" {:.1f}", d.current_effect_power);
		if (d.effect_mode == lcf::rpg::SavePicture::Effect_rotation || d.effect_mode == lcf::rpg::SavePicture::Effect_maniac_fixed_angle) {
			effect += fmt::format(" ({:.1f})", d.current_rotation);
		}
		y = DrawLine(y, "FX", effect);
	}

	y = DrawSeparator(y);
	// Common Flags
	std::vector<FlagInfo> flags = {
		{ "Fixed", d.fixed_to_map },
		{ "Chroma", d.use_transparent_color },
		{ "Tint", d.flags.affected_by_tint },
		{ "Flash", d.flags.affected_by_flash },
		{ "Shake", d.flags.affected_by_shake },
		{ "EraseOnMapChange", d.flags.erase_on_map_change },
		{ "EraseAfterBattle", d.flags.erase_on_battle_end },
		{ "FlipX", (bool)(d.easyrpg_flip & lcf::rpg::SavePicture::EasyRpgFlip_x) },
		{ "FlipY", (bool)(d.easyrpg_flip & lcf::rpg::SavePicture::EasyRpgFlip_y) }
	};
	y = DrawFlags(y, flags);

	y = DrawSeparator(y);

	// === SPECIFIC PROPERTIES ===

	if (!is_str) {
		// FILE PICTURE
		std::string name_str = std::string(d.name);
		if (name_str.length() > 18) name_str = "..." + name_str.substr(name_str.length() - 15);
		y = DrawLine(y, "File", name_str);

		if (d.spritesheet_cols > 1 || d.spritesheet_rows > 1) {
			std::string cell_str = fmt::format("#{} ({}x{})", d.spritesheet_frame, d.spritesheet_cols, d.spritesheet_rows);
			y = DrawLine(y, "Cell", cell_str);

			if (d.spritesheet_speed > 0) {
				y = DrawLine(y, "Anim", fmt::format("Spd: {} {}", d.spritesheet_speed, d.spritesheet_play_once ? "[Once]" : "[Loop]"));
			}
		}
	}
	else {
		// STRING PICTURE
		auto& win = Main_Data::game_windows->GetWindow(picture_id);
		const auto& wd = win.data;

		std::string dims = fmt::format("{}x{}", wd.width, wd.height);
		y = DrawLine(y, "Size", dims);

		std::string skin = std::string(wd.system_name);
		if (skin.empty()) skin = "Default";
		y = DrawLine(y, "Skin", skin);

		if (!wd.texts.empty()) {
			const auto& txt = wd.texts[0]; // Usually only one text chunk for string pics

			std::string font_info = std::string(txt.font_name);
			if (font_info.empty()) font_info = "Sys";
			font_info += fmt::format(" {}pt", txt.font_size);
			y = DrawLine(y, "Font", font_info);

			y = DrawDualLine(y, "LSpc", std::to_string(txt.letter_spacing), "HSpc", std::to_string(txt.line_spacing));

			// String Flags
			std::vector<FlagInfo> str_flags = {
				{ "Frame", wd.flags.draw_frame },
				{ "Grad", txt.flags.draw_gradient },
				{ "Shdw", txt.flags.draw_shadow },
				{ "Bold", txt.flags.bold },
				{ "Ital", txt.flags.italic },
				{ "Marg", wd.flags.border_margin }
			};

			// Background type (Stretch/Tile/None)
			std::string bg_type = "Stretch";
			if (wd.message_stretch == 0) bg_type = "Tile";
			if (wd.message_stretch == 2) bg_type = "None"; // easyrpg_none

			y = DrawLine(y, "BG", bg_type);
			y = DrawFlags(y, str_flags);

			// Content preview
			y = DrawSeparator(y);
			std::string content = ToString(txt.text);
			// Simple replace newlines for preview
			content = Utils::ReplaceAll(content, "\n", "\\n");
			if (content.length() > 22) content = content.substr(0, 20) + "...";

			contents->TextDraw(0, y, Font::ColorDefault, "Text:");
			contents->TextDraw(40, y, Font::ColorDefault, content);
		}
	}
}
