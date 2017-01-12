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
#include <map>

#include "dynrpg_textplugin.h"
#include "baseui.h"
#include "bitmap.h"
#include "drawable.h"
#include "drawable_mgr.h"
#include "game_actors.h"
#include "game_map.h"
#include "game_party.h"
#include "game_pictures.h"
#include "game_screen.h"
#include "game_variables.h"
#include "graphics.h"
#include "main_data.h"
#include "player.h"

class DynRpgText;

namespace {
	std::map<std::string, std::unique_ptr<DynRpgText>> graphics;
}

class DynRpgText : public Drawable {
public:
	DynRpgText(int pic_id, int x, int y, const std::string& text) : Drawable(0), pic_id(pic_id), x(x), y(y) {
		DrawableMgr::Register(this);

		AddLine(text);
	}

	DynRpgText(int pic_id, int x, int y, const std::vector<std::string>& text) : Drawable(0), pic_id(pic_id), x(x), y(y) {
		DrawableMgr::Register(this);

		for (auto& s : text) {
			AddLine(s);
		}
	}

	void AddLine(const std::string& text) {
		texts.push_back(text);

		Refresh();
	}

	void AddText(const std::string& text) {
		if (texts.empty()) {
			texts.push_back(text);
		} else {
			texts.back() += text;
		}

		Refresh();
	}

	void ClearText() {
		texts.clear();

		Refresh();
	}

	void SetPosition(int new_x, int new_y) {
		x = new_x;
		y = new_y;
	}

	void SetColor(int new_color) {
		color = new_color;

		Refresh();
	}

	void SetPictureId(int new_pic_id) {
		pic_id = new_pic_id;

		Refresh();
	}

	void SetFixed(bool fixed) {
		this->fixed = fixed;
	}

	void Draw(Bitmap& dst) override {
		if (!bitmap) {
			return;
		}

		const Sprite* sprite = Main_Data::game_pictures->GetPicture(pic_id).GetSprite();
		if (!sprite) {
			return;
		}

		if (fixed) {
			dst.Blit(x - Game_Map::GetDisplayX() / 16, y - Game_Map::GetDisplayY() / 16, *bitmap, bitmap->GetRect(), sprite->GetOpacity());
		} else {
			dst.Blit(x, y, *bitmap, bitmap->GetRect(), sprite->GetOpacity());
		}
	};

	void Update() {
		const Sprite *sprite = Main_Data::game_pictures->GetPicture(pic_id).GetSprite();

		if (sprite) {
			if (z != sprite->GetZ()) {
				z = sprite->GetZ() + 1;
				SetZ(z);
			}
		}
	}

	std::vector<uint8_t> Save(const std::string& id) {
		std::stringstream ss;
		ss << x << "," << y << ",";
		for (int i = 0; i < texts.size(); ++i) {
			std::string t = texts[i];
			// Replace , with a sentinel 0x01 to not mess up the tokenizer
			std::replace(t.begin(), t.end(), ',', '\1');
			ss << t;
			if (i < texts.size() - 1) {
				ss << "\n";
			}

		}
		ss << "," << color << "," << id;

		ss << "," << 255 << "," << (fixed ? "1" : "0") << "," << pic_id;

		std::vector<uint8_t> data;

		std::string s = ss.str();
		size_t slen = s.size();

		data.resize(slen);
		data.insert(data.end(), s.begin(), s.end());

		return data;
	}

	static int ParseParameter(bool& is_valid, std::u32string::iterator& text_index, std::u32string::iterator& end) {
		++text_index;

		if (text_index == end ||
			*text_index != '[') {
			--text_index;
			is_valid = false;
			return 0;
		}

		++text_index; // Skip the [

		bool null_at_start = false;
		std::stringstream ss;
		for (;;) {
			if (text_index == end) {
				break;
			} else if (*text_index == '\n') {
				--text_index;
				break;
			}
			else if (*text_index == '0') {
				// Truncate 0 at the start
				if (!ss.str().empty()) {
					ss << "0";
				} else {
					null_at_start = true;
				}
			}
			else if (*text_index >= '1' &&
					 *text_index <= '9') {
				ss << std::string(text_index, std::next(text_index));
			} else if (*text_index == ']') {
				break;
			} else {
				// End of number
				// Search for ] or line break
				while (text_index != end) {
					if (*text_index == '\n') {
						--text_index;
						break;
					} else if (*text_index == ']') {
						break;
					}
					++text_index;
				}
				break;
			}
			++text_index;
		}

		if (ss.str().empty()) {
			if (null_at_start) {
				ss << "0";
			} else {
				is_valid = false;
				return 0;
			}
		}

		int num;
		ss >> num;
		is_valid = true;
		return num;
	}

	static std::string ParseCommandCode(bool& success, std::u32string::iterator& text_index, std::u32string::iterator& end) {
		int parameter;
		bool is_valid;
		uint32_t cmd_char = *text_index;
		success = true;

		switch (cmd_char) {
			case 'n':
			case 'N':
				// Output Hero name
				parameter = ParseParameter(is_valid, text_index, end);
				if (is_valid) {
					Game_Actor* actor = NULL;
					if (parameter == 0) {
						// Party hero
						actor = Main_Data::game_party->GetActors()[0];
					} else {
						actor = Game_Actors::GetActor(parameter);
					}
					if (actor != NULL) {
						return actor->GetName();
					}
				}
				break;
			case 'v':
			case 'V':
				// Show Variable value
				parameter = ParseParameter(is_valid, text_index, end);
				if (is_valid && Main_Data::game_variables->IsValid(parameter)) {
					std::stringstream ss;
					ss << Main_Data::game_variables->Get(parameter);
					return ss.str();
				} else {
					// Invalid Var is always 0
					return "0";
				}
			case 'i':
				parameter = ParseParameter(is_valid, text_index, end);
				if (is_valid && parameter > 0 && parameter <= Data::items.size()) {
					return Data::items[parameter - 1].name;
				}
				return "";
			case 'I':
				parameter = ParseParameter(is_valid, text_index, end);
				if (is_valid && parameter > 0 && parameter <= Data::items.size()) {
					return Data::items[parameter - 1].description;
				}
				return "";
			case 't':
				parameter = ParseParameter(is_valid, text_index, end);
				if (is_valid && parameter > 0 && parameter <= Data::skills.size()) {
					return Data::skills[parameter - 1].name;
				}
				return "";
			case 'T':
				parameter = ParseParameter(is_valid, text_index, end);
				if (is_valid && parameter > 0 && parameter <= Data::skills.size()) {
					return Data::skills[parameter - 1].description;
				}
				return "";
			case 'x':
			case 'X':
				// Take text of ID referenced by X (if exists) TODO
				{

				}
				return "";
			default:;
				// When this happens text_index was not on a \ during calling
		}
		success = false;
		return "";
	}

	static std::string Substitute(const std::string& text) {
		std::u32string::iterator text_index, end;
		std::u32string utext;

		utext = Utils::DecodeUTF32(text);
		text_index = utext.end();
		end = utext.end();

		uint32_t escape_char = Utils::DecodeUTF32(Player::escape_symbol).front();

		if (!utext.empty()) {
			// Move on first valid char
			--text_index;

			// Apply commands that insert text
			while (std::distance(text_index, utext.begin()) <= -1) {
				switch (tolower(*text_index--)) {
					case 'n':
					case 'v':
					case 'i':
					case 't':
					case 'x':
					{
						if (*text_index != escape_char) {
							continue;
						}
						++text_index;

						auto start_code = text_index - 1;
						bool success;
						std::u32string command_result = Utils::DecodeUTF32(ParseCommandCode(success, text_index, end));
						if (!success) {
							text_index = start_code - 2;
							continue;
						}
						utext.replace(start_code, text_index + 1, command_result);
						// Start from the beginning, the inserted text might add new commands
						text_index = utext.end();

						// Move on first valid char
						--text_index;

						break;
					}
					default:
						break;
				}
			}
		}

		return Utils::EncodeUTF(utext);
	}

private:
	void Refresh() {
		if (texts.empty()) {
			bitmap.reset();
			return;
		}

		int width = 0;
		int height = 0;

		const FontRef& font = Font::Default();

		for (auto& t : texts) {
			t = Substitute(t);

			Rect r = font->GetSize(t);
			width = std::max(width, r.width);
			height += r.height + 2;
		}

		bitmap = Bitmap::Create(width, height, true);

		height = 0;
		for (auto& t : texts) {
			bitmap->TextDraw(0, height, color, t);
			height += font->GetSize(t).height + 2;
		}
	}

	std::vector<std::string> texts;
	BitmapRef bitmap;
	int x = 0;
	int y = 0;
	int z = 0;
	int pic_id = 1;
	int color = 0;
	bool fixed = false;
};

DynRpgText* get_text(const std::string& id, bool silent = false) {
	std::string new_id = DynRpgText::Substitute(id);

	auto it = graphics.find(new_id);
	if (it == graphics.end()) {
		if (!silent) {
			Output::Warning("No text with ID %s found", new_id.c_str());
		}
		return nullptr;
	}

	return (*it).second.get();
}

static bool WriteText(const dyn_arg_list& args) {
	DYNRPG_FUNCTION("write_text")

	DYNRPG_CHECK_ARG_LENGTH(4);

	DYNRPG_GET_STR_ARG(0, id)
	DYNRPG_GET_INT_ARG(1, x)
	DYNRPG_GET_INT_ARG(2, y)
	DYNRPG_GET_STR_ARG(3, text)

	const std::string new_id = DynRpgText::Substitute(id);
	graphics[new_id] = std::unique_ptr<DynRpgText>(new DynRpgText(1, x, y + 2, text));

	if (args.size() > 4) {
		DYNRPG_GET_STR_ARG(4, fixed)
		graphics[new_id]->SetFixed(fixed == "fixed");
	}

	if (args.size() > 5) {
		DYNRPG_GET_INT_ARG(5, color)
		graphics[new_id]->SetColor(color);
	}

	if (args.size() > 6) {
		DYNRPG_GET_INT_ARG(6, pic_id)
		graphics[new_id]->SetPictureId(pic_id);
	}

	return true;
}

static bool AppendLine(const dyn_arg_list& args) {
	DYNRPG_FUNCTION("append_line")

	DYNRPG_CHECK_ARG_LENGTH(2);

	DYNRPG_GET_STR_ARG(0, id)
	DYNRPG_GET_STR_ARG(1, text)

	DynRpgText* handle = get_text(id);

	if (!handle) {
		return true;
	}

	handle->AddLine(text);

	return true;
}

static bool AppendText(const dyn_arg_list& args) {
	DYNRPG_FUNCTION("append_text")

	DYNRPG_CHECK_ARG_LENGTH(2);

	DYNRPG_GET_STR_ARG(0, id)
	DYNRPG_GET_STR_ARG(1, text)

	DynRpgText* handle = get_text(id);

	if (!handle) {
		return true;
	}

	handle->AddText(text);

	return true;
}

static bool ChangeText(const dyn_arg_list& args) {
	DYNRPG_FUNCTION("change_text")

	DYNRPG_CHECK_ARG_LENGTH(3);

	DYNRPG_GET_STR_ARG(0, id)
	DYNRPG_GET_STR_ARG(1, text)
	DYNRPG_GET_INT_ARG(2, color)

	DynRpgText* handle = get_text(id);

	if (!handle) {
		return true;
	}

	handle->ClearText();
	handle->SetColor(color);
	handle->AddText(text);

	return true;
}

static bool ChangePosition(const dyn_arg_list& args) {
	DYNRPG_FUNCTION("change_position")

	DYNRPG_CHECK_ARG_LENGTH(3);

	DYNRPG_GET_STR_ARG(0, id)
	DYNRPG_GET_INT_ARG(1, x)
	DYNRPG_GET_INT_ARG(2, y)

	DynRpgText* handle = get_text(id);

	if (!handle) {
		return true;
	}

	// Offset is somehow wrong compared to RPG_RT
	handle->SetPosition(x, y + 2);

	return true;
}

static bool RemoveText(const dyn_arg_list& args) {
	DYNRPG_FUNCTION("remove_text")

	DYNRPG_CHECK_ARG_LENGTH(2);

	DYNRPG_GET_STR_ARG(0, id)
	DYNRPG_GET_STR_ARG(1, nothing)

	DynRpgText* handle = get_text(id, true);

	if (!handle) {
		return true;
	}

	handle->ClearText();

	return true;
}

static bool RemoveAll(const dyn_arg_list& args) {
	DYNRPG_FUNCTION("remove_all")

	DYNRPG_CHECK_ARG_LENGTH(0);

	graphics.clear();

	return true;
}

std::string DynRpg::TextPlugin::GetIdentifier() {
	return "DynTextPlugin";
}

void DynRpg::TextPlugin::RegisterFunctions() {
	DynRpg::RegisterFunction("write_text", WriteText);
	DynRpg::RegisterFunction("append_line", AppendLine);
	DynRpg::RegisterFunction("append_text", AppendText);
	DynRpg::RegisterFunction("change_text", ChangeText);
	DynRpg::RegisterFunction("change_position", ChangePosition);
	DynRpg::RegisterFunction("remove_text", RemoveText);
	DynRpg::RegisterFunction("remove_all", RemoveAll);
}

void DynRpg::TextPlugin::Update() {
	for (auto& g : graphics) {
		g.second->Update();
	}
}

DynRpg::TextPlugin::~TextPlugin() {
	graphics.clear();
}

void DynRpg::TextPlugin::Load(const std::vector<uint8_t>& in_buffer) {
	size_t counter = 0;

	std::string str((char*)in_buffer.data(), in_buffer.size());

	std::vector<std::string> tokens = Utils::Tokenize(str, [&] (char32_t c) { return c == ','; });

	int x = 0;
	int y = 0;
	std::vector<std::string> texts;
	int color = 0;
	std::string id = "";
	bool fixed = false;
	int pic_id = 1;

	for (auto& t : tokens) {
		switch (counter) {
			case 0:
				x = atoi(t.c_str());
				break;
			case 1:
				y = atoi(t.c_str());
				break;
			case 2:
			{
				// Replace sentinel \1 with ,
				std::replace(t.begin(), t.end(), '\1', ',');

				texts = Utils::Tokenize(t, [&] (char32_t c) { return c == '\n'; });
			}
				break;
			case 3:
				color = atoi(t.c_str());
				break;
			case 4:
				// transparency, but isn't that from the picture?
				break;
			case 5:
				fixed = t == "1";
				break;
			case 6:
				pic_id = atoi(t.c_str());
				break;
			case 7:
				id = t.c_str();
				break;
			default:
				break;
		}

		++counter;

		if (counter == 8) {
			counter = 0;

			graphics[id] = std::unique_ptr<DynRpgText>(new DynRpgText(pic_id, x, y + 2, texts));
			texts.clear();
			graphics[id]->SetColor(color);
			graphics[id]->SetFixed(fixed);
		}
	}
}

std::vector<uint8_t> DynRpg::TextPlugin::Save() {
	std::vector<uint8_t> save_data;
	std::stringstream ss;

	for (auto& g : graphics) {
		std::vector<uint8_t> res = g.second->Save(g.first);
		save_data.reserve(save_data.size() + res.size() + 1);
		save_data.insert(save_data.end(), res.begin(), res.end());

		save_data.push_back(',');
	}
	save_data.pop_back();

	return DynRpgPlugin::Save();
}
