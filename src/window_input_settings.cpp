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
#include "window_input_settings.h"
#include "bitmap.h"
#include "input_buttons.h"
#include "window_selectable.h"


Window_InputSettings::Window_InputSettings(Scene* parent, int ix, int iy, int iwidth, int iheight) :
	Window_Selectable(parent, ix, iy, iwidth, iheight) {
	column_max = 2;

	SetContents(Bitmap::Create(width - 16, height - 16));
}

Input::InputButton Window_InputSettings::GetInputButton() const {
	return button;
}

void Window_InputSettings::SetInputButton(Input::InputButton button) {
	this->button = button;
	Refresh();
}

bool Window_InputSettings::RemoveMapping() {
	int index = GetIndex();

	int i = 0;
	auto& mappings = Input::GetInputSource()->GetButtonMappings();
	for (auto ki = mappings.LowerBound(button); ki != mappings.end() && ki->first == button; ++ki) {
		if (i == index) {
			auto key = static_cast<Input::Keys::InputKey>(ki->second);

			mappings.Remove({button, key});
			Refresh();
			break;
		}

		++i;
	}

	return true;
}

void Window_InputSettings::ResetMapping() {
	Input::ResetDefaultMapping(button);
	Refresh();
}

void Window_InputSettings::Refresh() {
	contents->Clear();

	auto& mappings = Input::GetInputSource()->GetButtonMappings();
	auto custom_names = Input::GetInputKeyNames();

	std::vector<std::string> items;

	std::stringstream ss;
	for (auto ki = mappings.LowerBound(button); ki != mappings.end() && ki->first == button;++ki) {
		auto key = static_cast<Input::Keys::InputKey>(ki->second);

		auto custom_name = std::find_if(custom_names.begin(), custom_names.end(), [&](auto& key_pair) {
			return key_pair.first == key;
		});

		if (custom_name != custom_names.end()) {
			items.push_back(custom_name->second);
		} else {
			items.push_back(Input::Keys::kInputKeyNames.tag(key));
		}
	}

	if (items.size() > mapping_limit) {
		items.resize(mapping_limit);
	}

	item_max = static_cast<int>(items.size());

	for (int i = 0; i < item_max; ++i) {
		Rect rect = GetItemRect(i);
		contents->ClearRect(rect);

		const std::string& text = items[i];

		bool enabled = true;

		Font::SystemColor color = enabled ? Font::ColorDefault : Font::ColorDisabled;
		contents->TextDraw(rect.x, rect.y, color, text);
	}
}
