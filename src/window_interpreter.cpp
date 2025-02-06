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
#include <vector>
#include "window_interpreter.h"
#include "bitmap.h"
#include "game_map.h"
#include "game_message.h"
#include "game_system.h"
#include "game_variables.h"
#include "input.h"
#include "player.h"
#include "lcf/reader_util.h"
#include <cache.h>
#include "feature.h"

#ifdef ENABLE_DYNAMIC_INTERPRETER_CONFIG
namespace {
	bool GlobalIsRPG2k3Commands() { return Player::game_config.patch_rpg2k3_commands.Get(); }
	bool GlobalIsPatchDynRpg() { return Player::game_config.patch_dynrpg.Get(); }
	bool GlobalIsPatchManiac() { return Player::game_config.patch_maniac.Get(); }
	bool GlobalIsPatchKeyPatch() { return Player::game_config.patch_key_patch.Get(); }
	bool GlobalIsPatchDestiny() { return Player::game_config.patch_destiny.Get(); }
	bool GlobalIsPatchCommonThisEvent() { return Player::game_config.patch_common_this_event.Get(); }
	bool GlobalIsPatchUnlockPics() { return Player::game_config.patch_unlock_pics.Get(); }
	bool GlobalHasRpg2kBattleSystem() { return lcf::Data::system.easyrpg_use_rpg2k_battle_system; }
}

struct RuntimeFlagInfo {
	bool (*config_fn)();
	Game_Interpreter_Shared::StateRuntimeFlagRef field_on, field_off;
	std::string_view name;
};

using StateFlags = lcf::rpg::SaveEventExecState::EasyRpgStateRuntime_Flags;
constexpr std::array <RuntimeFlagInfo, 8> runtime_flags = { {
	{ &GlobalIsRPG2k3Commands, &StateFlags::patch_rpg2k3_cmds_on, &StateFlags::patch_rpg2k3_cmds_off, "rpg2k3_cmds"},
	{ &GlobalIsPatchDynRpg, &StateFlags::patch_dynrpg_on, &StateFlags::patch_dynrpg_off, "dynrpg" },
	{ &GlobalIsPatchManiac, &StateFlags::patch_maniac_on, &StateFlags::patch_maniac_off, "maniac" },
	{ &GlobalIsPatchKeyPatch, &StateFlags::patch_keypatch_on, &StateFlags::patch_keypatch_off, "keypatch" },
	{ &GlobalIsPatchDestiny, &StateFlags::patch_destiny_on, &StateFlags::patch_destiny_off, "destiny" },
	{ &GlobalIsPatchCommonThisEvent, &StateFlags::patch_common_this_event_on, &StateFlags::patch_common_this_event_off, "common_this_event" },
	{ &GlobalIsPatchUnlockPics, &StateFlags::patch_unlock_pics_on, &StateFlags::patch_unlock_pics_off, "unlock_pics" },
	{ &GlobalHasRpg2kBattleSystem, &StateFlags::use_rpg2k_battle_system_on, &StateFlags::use_rpg2k_battle_system_off, "rpg2k_battle_system" }
} };
#endif

Window_Interpreter::Window_Interpreter(int ix, int iy, int iwidth, int iheight) :
	Window_Selectable(ix, iy, iwidth, iheight) {
	column_max = 1;

	sub_actions = UiSubActionLine(
		{ UiAction::ShowRuntimeFlags,		UiAction::ShowMovementInfo },
		{ "[Flags]",						"WAITING for EV movement" },
		{ Font::SystemColor::ColorHeal,		Font::SystemColor::ColorCritical },
		{ [&] { return (this->state.easyrpg_runtime_flags.conf_override_active); }, [&] { return this->state.wait_movement; } }
	);

	sub_window_flags.reset(new Window_Selectable(Player::menu_offset_x + 15, Player::menu_offset_y + 16, 288, 208));
	sub_window_flags->SetVisible(false);
	sub_window_flags->SetActive(false);
	sub_window_flags->SetIndex(-1);
}

Window_Interpreter::~Window_Interpreter() {

}

void Window_Interpreter::SetStackState(std::string interpreter_desc, lcf::rpg::SaveEventExecState state) {
	this->display_item = { interpreter_desc };
	this->state = state;
}

void Window_Interpreter::Refresh() {

	if (sub_window_flags->GetActive()) {
#ifdef ENABLE_DYNAMIC_INTERPRETER_CONFIG
		DrawRuntimeFlagsWindow();
#endif
		return;
	}

	stack_display_items.clear();

	int max_cmd_count = 0, max_evt_id = 10, max_page_id = 0;

	stack_display_items = Debug::CreateCallStack(state);

	for (auto it = stack_display_items.begin(); it < stack_display_items.end(); ++it) {
		auto& item = *it;

		if (item.evt_id > max_evt_id)
			max_evt_id = item.evt_id;
		if (item.page_id > max_page_id)
			max_page_id = item.page_id;
		if (item.cmd_count > max_cmd_count)
			max_cmd_count = item.cmd_count;
	}

	item_max = stack_display_items.size() + lines_without_stack;
	lines_without_stack = lines_without_stack_fixed;

	if (sub_actions.IsVisible()) {
		item_max++;
		lines_without_stack++;
	}

	digits_stackitemno = std::log10(stack_display_items.size()) + 1;
	digits_evt_id = max_evt_id == 0 ? 2 : std::log10(max_evt_id) + 1;
	digits_page_id = max_page_id == 0 ? 0 : std::log10(max_page_id) + 1;
	digits_cmdcount = std::log10(max_cmd_count) + 1;

	digits_evt_combined_id = digits_evt_id + 3;
	if (digits_page_id > 0)
		digits_evt_combined_id += digits_page_id + 2;

	CreateContents();
	contents->Clear();

	if (!IsValid())
		return;

	DrawDescriptionLines();

	for (int i = 0; i < static_cast<int>(stack_display_items.size()); ++i) {
		DrawStackLine(i);
	}
}

bool Window_Interpreter::IsValid() {
	return !display_item.desc.empty();
}

void Window_Interpreter::Update() {
	if (sub_window_flags->GetActive()) {
		if (Input::IsTriggered(Input::InputButton::CANCEL)) {
			sub_window_flags->SetActive(false);
			sub_window_flags->SetVisible(false);
			sub_window_flags->SetIndex(-1);
		} else {
			sub_window_flags->Update();
		}
		return;
	}
	Window_Selectable::Update();

	if (IsHoveringSubActionLine()) {
		sub_actions.Update(*this);

		if (Input::IsTriggered(Input::InputButton::DECISION)) {
			if (GetSelectedAction() == UiAction::ShowRuntimeFlags) {
				sub_window_flags->SetActive(true);
				sub_window_flags->SetVisible(true);
				this->Refresh();

				Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));
			} else if (GetSelectedAction() == UiAction::ShowMovementInfo) {
				Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Buzzer));
			}
		}
	} else {
		sub_actions.ClearIndex();
	}
}

bool Window_Interpreter::IsHoveringSubActionLine() const {
	return GetIndex() == 1 && sub_actions.IsVisible();
}

void Window_Interpreter::DrawDescriptionLines() {
	int i = 0;
	Rect rect = GetItemRect(i++);
	contents->ClearRect(rect);

	contents->TextDraw(rect.x, rect.y, Font::ColorDefault, display_item.desc);

	rect = GetItemRect(i++);
	contents->ClearRect(rect);

	if (sub_actions.IsVisible()) {
		sub_actions.Draw(contents, rect);
	}

	rect = GetItemRect(i++);
	contents->ClearRect(rect);

	auto str_ex_type = std::string(Game_Interpreter_Shared::kExecutionType.tag(static_cast<int>(stack_display_items[0].type_ex)));
	contents->TextDraw(rect.x, rect.y, Font::ColorDefault, "(");
	contents->TextDraw(rect.x + 6, rect.y, Font::ColorHeal, str_ex_type);
	contents->TextDraw(rect.x + 6 * (str_ex_type.length() + 1), rect.y, Font::ColorDefault, ")");
	contents->TextDraw(rect.x + rect.width / 2, rect.y, Font::ColorDefault, "Stack Size: ");
	contents->TextDraw(GetWidth() - 16, rect.y, Font::ColorCritical, std::to_string(state.stack.size()), Text::AlignRight);
}

void Window_Interpreter::DrawStackLine(int index) {

	Rect rect = GetItemRect(index + lines_without_stack);
	contents->ClearRect(rect);

	Debug::CallStackItem& item = stack_display_items[index];

	contents->TextDraw(rect.x, rect.y, Font::ColorDisabled, fmt::format("[{:0" + std::to_string(digits_stackitemno) + "d}]", state.stack.size() - index));

	std::string formatted_id;
	Font::SystemColor color = Font::ColorDefault;

	switch (item.type_ev) {
		case InterpreterEventType::MapEvent:
			if (item.page_id > 0) {
				formatted_id = fmt::format("EV{:0" + std::to_string(digits_evt_id) + "d}[{:0" + std::to_string(digits_page_id) + "d}]", item.evt_id, item.page_id);
			} else {
				formatted_id = fmt::format("EV{:0" + std::to_string(digits_evt_id) + "d}", item.evt_id);
			}
			if (item.map_has_changed) {
				color = Font::ColorKnockout;
			}
			break;
		case InterpreterEventType::CommonEvent:
			formatted_id = fmt::format("CE{:0" + std::to_string(digits_evt_id) + "d}", item.evt_id);
			break;
		case InterpreterEventType::BattleEvent:
			formatted_id = fmt::format("BE{:0" + std::to_string(digits_evt_id) + "d}", item.evt_id);
			break;
		default:
			formatted_id = fmt::format("{:0" + std::to_string(digits_evt_id + 2) + "d}", 0);
			color = Font::ColorKnockout;
			break;
	}
	contents->TextDraw(rect.x + (digits_stackitemno * 6) + 16, rect.y, color, formatted_id);

	std::string name = item.name;
	int max_length = 28;
	max_length -= digits_stackitemno;
	max_length -= digits_evt_combined_id;
	max_length -= digits_cmdcount * 2;
	if (static_cast<int>(name.length()) > max_length) {
		name = name.substr(0, max_length - 3) + "...";
	}
	contents->TextDraw(rect.x + ((digits_stackitemno + digits_evt_combined_id) * 6) + 16, rect.y, Font::ColorDefault, name, Text::AlignLeft);
	contents->TextDraw(GetWidth() - 16, rect.y, Font::ColorDefault, fmt::format("{:0" + std::to_string(digits_cmdcount) + "d}/{:0" + std::to_string(digits_cmdcount) + "d}", item.cmd_current, item.cmd_count), Text::AlignRight);
}

Window_Interpreter::UiAction Window_Interpreter::GetSelectedAction() const {
	if (GetIndex() >= lines_without_stack) {
		return UiAction::ShowStackItem;
	}
	if (GetIndex() == 1 && sub_actions.IsVisible()) {
		return sub_actions.GetSelectedAction();
	}
	return UiAction::None;
}

int Window_Interpreter::GetSelectedStackFrameLine() const {
	if (GetIndex() < lines_without_stack) {
		return -1;
	}
	return state.stack.size() - (GetIndex() - lines_without_stack) - 1;
}

bool Window_Interpreter::UiSubActionLine::IsVisible() const {
	for (auto v : this->visibility_delegates) {
		if (v())
			return true;
	}
	return false;
}

void Window_Interpreter::UiSubActionLine::Update(Window_Selectable& parent) {
	if (actions.size() == 0 || !visibility_delegates[this->index]())
		return;

	int i = this->index;

	if (Input::IsRepeated(Input::RIGHT)) {
		i++;

		while (i != this->index) {
			if (i >= static_cast<int>(actions.size()))
				i = 0;
			if (visibility_delegates[i]())
				break;
			i++;
		}
	}
	if (Input::IsRepeated(Input::LEFT)) {
		i--;
		while (i != this->index) {
			if (i < 0)
				i = actions.size() - 1;
			if (visibility_delegates[i]())
				break;
			i--;
		}
	}

	if (i != this->index) {
		this->index = i;
		Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cursor));
	}
	int offset_x = 0;
	for (i = 0; i < this->index; i++) {
		offset_x += (this->texts[i].length() + 1) * 6;
	}
	auto cursor_rect = parent.GetCursorRect();
	parent.SetCursorRect(Rect(cursor_rect.x + offset_x, cursor_rect.y, (this->texts[this->index].length() + 1) * 6 + 2, cursor_rect.height));
}

namespace {
	void TextDrawUnderlined(BitmapRef contents, int x, int y, int color, std::string_view text) {
		auto sys = Cache::SystemOrBlack();
		auto rect = Rect(x, y + 12, text.length() * 6, 1);
		// Draw shadow first
		contents->FillRect(Rect(rect.x + 1, rect.y + 1, rect.width, rect.height), sys->GetColorAt(18, 34));

		//Draw the actual text
		contents->TextDraw(x, y, color, text);

		// Draw underline
		contents->FillRect(rect, sys->GetColorAt(color % 10 * 16 + 2, color / 10 * 16 + 48 + 15));
	}
}

void Window_Interpreter::UiSubActionLine::Draw(BitmapRef contents, Rect rect) const {
	int offset_x = 0;

	for (int i = 0; i < static_cast<int>(this->actions.size()); i++) {
		if (!visibility_delegates[i]())
			continue;
		TextDrawUnderlined(contents, rect.x + offset_x, rect.y, colors[i], texts[i]);
		offset_x += (texts[i].length() + 1) * 6;
	}
}

void Window_Interpreter::UiSubActionLine::ClearIndex() {
	this->index = 0;
}

Window_Interpreter::UiAction Window_Interpreter::UiSubActionLine::GetSelectedAction() const {
	return this->actions[this->index];
}

#ifdef ENABLE_DYNAMIC_INTERPRETER_CONFIG
void Window_Interpreter::DrawRuntimeFlagsWindow() const {
	sub_window_flags->CreateContents();
	auto contents = sub_window_flags->GetContents();
	Rect rect = sub_window_flags->GetItemRect(0);


	contents->TextDraw(rect.x, rect.y, Font::ColorHeal, "Interpreter Flags:");
	contents->TextDraw(rect.x + 19 * 6, rect.y, Font::ColorDefault, display_item.desc);

	int i = 0;
	for (auto info : runtime_flags) {
		auto flag = Game_Interpreter_Shared::GetRuntimeFlag(state, info.field_on, info.field_off);;
		if (!flag) {
			continue;
		}

		rect = sub_window_flags->GetItemRect(i + 1);
		contents->ClearRect(rect);

		bool is_active = info.config_fn();
		bool is_overridden = *flag;
		contents->TextDraw(rect.x, rect.y, Font::ColorDefault, fmt::format("{}:", info.name));

		int r = 16;
		contents->TextDraw(sub_window_flags->GetWidth() - r, rect.y, Font::ColorCritical, is_overridden ? "[ON]" : "[OFF]", Text::AlignRight);
		r += 3 * 8;
		if (!is_overridden)
			r += 8;
		contents->TextDraw(sub_window_flags->GetWidth() - r, rect.y, Font::ColorDefault, "->", Text::AlignRight);
		r += 16;
		contents->TextDraw(sub_window_flags->GetWidth() - r, rect.y, Font::ColorDisabled, is_active ? "[ON]" : "[OFF]", Text::AlignRight);

		++i;
	}
}
#endif
