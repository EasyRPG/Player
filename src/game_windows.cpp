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

#include "game_windows.h"
#include "game_message.h"
#include "main_data.h"
#include "compiler.h"
#include "text.h"
#include "cache.h"
#include "pending_message.h"
#include "filefinder.h"
#include "output.h"
#include "player.h"

Game_Windows::Window_User::Window_User(lcf::rpg::SaveEasyRpgWindow save)
	: data(std::move(save))
{

}

void Game_Windows::SetSaveData(std::vector<lcf::rpg::SaveEasyRpgWindow> save)
{
	windows.clear();

	int num_windows = static_cast<int>(save.size());
	windows.reserve(num_windows);
	for (int i = 0; i < num_windows; ++i) {
		windows.emplace_back(std::move(save[i]));
	}
}

std::vector<lcf::rpg::SaveEasyRpgWindow> Game_Windows::GetSaveData() const {
	std::vector<lcf::rpg::SaveEasyRpgWindow> save;

	auto data_size = static_cast<int>(windows.size());
	save.reserve(data_size);

	for (auto& win: windows) {
		save.push_back(win.data);
	}

	// RPG_RT Save game data always has a constant number of pictures
	// depending on the engine version. We replicate this, unless we have even
	// more pictures than that.
	while (data_size > static_cast<int>(save.size())) {
		lcf::rpg::SaveEasyRpgWindow data;
		data.ID = static_cast<int>(save.size()) + 1;
		save.push_back(std::move(data));
	}

	return save;
}

Game_Windows::Window_User& Game_Windows::GetWindow(int id) {
	if (EP_UNLIKELY(id > static_cast<int>(windows.size()))) {
		windows.reserve(id);
		while (static_cast<int>(windows.size()) < id) {
			windows.emplace_back(windows.size() + 1);
		}
	}
	return windows[id - 1];
}

Game_Windows::Window_User* Game_Windows::GetWindowPtr(int id) {
	return id <= static_cast<int>(windows.size())
		? &windows[id - 1] : nullptr;
}

bool Game_Windows::Window_User::Create(const WindowParams& params) {
	data.width = params.width;
	data.height = params.height;
	data.system_name = lcf::DBString(params.system_name);
	data.message_stretch = params.message_stretch;
	data.flags.draw_frame = params.draw_frame;
	data.flags.border_margin = params.border_margin;

	for (const auto& text: params.texts) {
		lcf::rpg::SaveEasyRpgText data_text;

		data_text.text = lcf::DBString(text.text);
		data_text.position_x = text.position_x;
		data_text.position_y = text.position_y;
		data_text.font_name = lcf::DBString(text.font_name);
		data_text.font_size = text.font_size;
		data_text.letter_spacing = text.letter_spacing;
		data_text.line_spacing = text.line_spacing;
		data_text.flags.draw_gradient = text.draw_gradient;
		data_text.flags.draw_shadow = text.draw_shadow;
		data_text.flags.bold = text.bold;
		data_text.flags.italic = text.italic;

		data.texts.push_back(data_text);
	}

	Refresh();

	return true;
}

bool Game_Windows::Create(int id, const WindowParams& params) {
	auto& window = GetWindow(id);

	if (window.Create(params)) {
		if (Main_Data::game_pictures->Show(id, params)) {
			Main_Data::game_pictures->AttachWindow(id);
			auto& pic = Main_Data::game_pictures->GetPicture(id);
			pic.CreateEmptyBitmap(window.window->GetWidth(), window.window->GetHeight());
			return true;
		} else {
			window.Erase();
			return false;
		}
	}

	return false;
}

void Game_Windows::Window_User::Erase() {
	data = {};
}

void Game_Windows::Window_User::Refresh() {
	std::vector<FontRef> fonts;
	std::vector<PendingMessage> messages;

	for (const auto& text: data.texts) {
		// TODO: Async requests
		FontRef font;

		Filesystem_Stream::InputStream font_file;
		std::string font_name = ToString(text.font_name);
		// Try to find best fitting font
		if (text.flags.bold && text.flags.italic) {
			font_file = FileFinder::OpenFont(font_name + "-BoldItalic");
		}

		if (!font_file && text.flags.bold) {
			font_file = FileFinder::OpenFont(font_name + "-Bold");
		}

		if (!font_file && text.flags.italic) {
			font_file = FileFinder::OpenFont(font_name + "-Italic");
		}

		if (!font_file) {
			font_file = FileFinder::OpenFont(font_name+ "-Regular");
		}

		if (!font_file) {
			font_file = FileFinder::OpenFont(font_name);
		}

		if (!font_file) {
			Output::Warning("Font not found: {}", text.font_name);
			font = Font::Default();
		} else {
			font = Font::CreateFtFont(std::move(font_file), text.font_size, text.flags.bold, text.flags.italic);
		}

		fonts.emplace_back(font);

		std::stringstream ss(ToString(text.text));
		std::string out;
		PendingMessage pm;
		while (Utils::ReadLine(ss, out)) {
			pm.PushLine(out);
		}
		messages.emplace_back(pm);
	}

	if (data.width == 0 && data.height == 0) {
		// Automatic window size
		int x_max = 0;
		int y_max = 0;

		for (size_t i = 0; i < data.texts.size(); ++i) {
			// Lots of duplication with the rendering code below but cannot be easily reduced more
			const auto& font = fonts[i];
			const auto& pm = messages[i];
			const auto& text = data.texts[i];

			int x = text.position_x;
			int y = text.position_y;
			for (const auto& line: pm.GetLines()) {
				std::u32string line32;
				auto* text_index = line.data();
				const auto* end = line.data() + line.size();

				while (text_index != end) {
					auto tret = Utils::TextNext(text_index, end, Player::escape_char);
					text_index = tret.next;

					if (EP_UNLIKELY(!tret)) {
						continue;
					}

					const auto ch = tret.ch;

					if (Utils::IsControlCharacter(ch)) {
						// control characters not handled
						continue;
					}

					if (tret.is_escape && ch != Player::escape_char) {
						if (!line32.empty()) {
							x += Text::GetSize(*font, Utils::EncodeUTF(line32)).width;
							line32.clear();
						}

						// Special message codes
						switch (ch) {
							case 'c':
							case 'C':
							{
								// Color
								Game_Message::ParseColor(text_index, end, Player::escape_char, true);
							}
							break;
						}
						continue;
					}

					line32 += static_cast<char32_t>(ch);
				}

				if (!line32.empty()) {
					x += Text::GetSize(*font, Utils::EncodeUTF(line32)).width;
				}

				x_max = std::max(x, x_max);

				x = 0;
				y += text.font_size + text.line_spacing;
			}

			y -= text.line_spacing;

			y_max = std::max(y, y_max);
		}

		// Border size
		x_max += 16;
		y_max += 20; // 16 looks better but this matches better with Maniac Patch

		data.width = x_max;
		data.height = y_max;
	}

	window = std::make_unique<Window_Selectable>(0, 0, data.width, data.height);
	window->CreateContents();
	window->SetVisible(false);

	if (data.message_stretch == lcf::rpg::System::Stretch_easyrpg_none) {
		window->SetWindowskin(nullptr);
	} else {
		BitmapRef system;
		if (!data.system_name.empty()) {
			// TODO: Async request
			system = Cache::System(data.system_name);
		} else {
			system = Cache::SystemOrBlack();
		}

		window->SetWindowskin(system);
		window->SetStretch(data.message_stretch == lcf::rpg::System::Stretch_stretch);
	}

	if (!data.flags.draw_frame) {
		window->SetFrameOpacity(0);
	}

	if (!data.flags.border_margin) {
		window->SetBorderX(0);
		window->SetBorderY(0);
	}

	for (size_t i = 0; i < data.texts.size(); ++i) {
		// Lots of duplication with the rendering code below but cannot be easily reduced more
		const auto& font = fonts[i];
		const auto& pm = messages[i];
		const auto& text = data.texts[i];

		int x = text.position_x;
		int y = text.position_y;
		int text_color = 0;
		for (const auto& line: pm.GetLines()) {
			std::u32string line32;
			auto* text_index = line.data();
			const auto* end = line.data() + line.size();

			while (text_index != end) {
				auto tret = Utils::TextNext(text_index, end, Player::escape_char);
				text_index = tret.next;

				if (EP_UNLIKELY(!tret)) {
					continue;
				}

				const auto ch = tret.ch;

				if (Utils::IsControlCharacter(ch)) {
					// control characters not handled
					continue;
				}

				if (tret.is_escape && ch != Player::escape_char) {
					if (!line32.empty()) {
						x += Text::Draw(*window->GetContents(), x, y, *font, *Cache::System(), text_color, Utils::EncodeUTF(line32)).x;
						line32.clear();
					}

					// Special message codes
					switch (ch) {
						case 'c':
						case 'C':
						{
							// Color
							auto pres = Game_Message::ParseColor(text_index, end, Player::escape_char, true);
							auto value = pres.value;
							text_index = pres.next;
							text_color = value > 19 ? 0 : value;
						}
						break;
					}
					continue;
				}

				line32 += static_cast<char32_t>(ch);
			}

			if (!line32.empty()) {
				Text::Draw(*window->GetContents(), x, y, *font, *Cache::System(), text_color, Utils::EncodeUTF(line32));
			}

			x = 0;
			y += text.font_size + text.line_spacing;
		}
	}
}

void Game_Windows::Erase(int id) {
	auto* window = GetWindowPtr(id);
	if (EP_LIKELY(window)) {
		window->Erase();
	}
}
