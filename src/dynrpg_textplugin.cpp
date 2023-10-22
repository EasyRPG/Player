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
#include <memory>
#include <lcf/reader_util.h>

#include "dynrpg_textplugin.h"
#include "baseui.h"
#include "bitmap.h"
#include "drawable.h"
#include "drawable_mgr.h"
#include "game_map.h"
#include "game_message.h"
#include "game_pictures.h"
#include "game_variables.h"
#include "main_data.h"
#include "pending_message.h"
#include "text.h"

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

		const Game_Pictures::Picture* pic = Main_Data::game_pictures->GetPicturePtr(pic_id);
		if (!pic) {
			return;
		}
		const Sprite_Picture* sprite = pic->sprite.get();
		if (!sprite) {
			return;
		}

		SetZ(sprite->GetZ() + 1);
	}

	void SetFixed(bool fixed) {
		this->fixed = fixed;
	}

	void Draw(Bitmap& dst) override {
		if (!bitmap) {
			return;
		}

		const Game_Pictures::Picture* pic = Main_Data::game_pictures->GetPicturePtr(pic_id);
		if (!pic) {
			return;
		}
		const Sprite_Picture* sprite = pic->sprite.get();
		if (!sprite) {
			return;
		}

		// For unknown reasons the official plugin has an y-offset of 2
		if (fixed) {
			dst.Blit(x - Game_Map::GetDisplayX() / 16, y - Game_Map::GetDisplayY() / 16 + 2, *bitmap, bitmap->GetRect(), sprite->GetOpacity());
		} else {
			dst.Blit(x, y + 2, *bitmap, bitmap->GetRect(), sprite->GetOpacity());
		}
	};

	void Update() {
		if (GetZ() == 0) {
			SetPictureId(pic_id);
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

	static DynRpgText* GetTextHandle(const std::string& id, bool silent = false) {
		PendingMessage pm(CommandCodeInserter);
		pm.PushLine(id);
		std::string new_id = pm.GetLines().front();

		auto it = graphics.find(new_id);
		if (it == graphics.end()) {
			if (!silent) {
				Output::Warning("No text with ID %s found", new_id.c_str());
			}
			return nullptr;
		}

		return (*it).second.get();
	}

	static std::optional<std::string> CommandCodeInserter(char ch, const char** iter, const char* end, uint32_t escape_char) {
		if (ch == 'I' || ch == 'i') {
			auto parse_ret = Game_Message::ParseParam('I', 'i', *iter, end, escape_char, true);
			*iter = parse_ret.next;
			int value = parse_ret.value;
			const auto* item = lcf::ReaderUtil::GetElement(lcf::Data::items, value);
			if (!item) {
				Output::Warning("Invalid Item Id {} in DynTextPlugin text", value);
				return "";
			} else{
				return ToString(ch == 'i' ? item->name : item->description);
			}
		} else if (ch == 'T' || ch == 't') {
			auto parse_ret = Game_Message::ParseParam('T', 't', *iter, end, escape_char, true);
			*iter = parse_ret.next;
			int value = parse_ret.value;
			const auto* skill = lcf::ReaderUtil::GetElement(lcf::Data::skills, value);
			if (!skill) {
				Output::Warning("Invalid Item Id {} in DynTextPlugin text", value);
				return "";
			} else{
				return ToString(ch == 't' ? skill->name : skill->description);
			}
		} else if (ch == 'x' || ch == 'X') {
			auto parse_ret = Game_Message::ParseStringParam('X', 'x', *iter, end, escape_char, true);
			*iter = parse_ret.next;
			std::string value = parse_ret.value;
			auto* handle = GetTextHandle(value);
			if (handle) {
				return handle->texts[0];
			} else {
				return "";
			}
		}

		return PendingMessage::DefaultCommandInserter(ch, iter, end, escape_char);
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
			PendingMessage pm(CommandCodeInserter);
			pm.PushLine(t);
			t = pm.GetLines().front();

			Rect r = Text::GetSize(*font, t);
			width = std::max(width, r.width);
			height += r.height + 2;
		}

		bitmap = Bitmap::Create(width, height, true);

		height = 0;
		for (auto& t : texts) {
			bitmap->TextDraw(0, height, color, t);
			height += Text::GetSize(*font, t).height + 2;
		}

		SetPictureId(pic_id);
	}

	std::vector<std::string> texts;
	BitmapRef bitmap;
	int x = 0;
	int y = 0;
	int pic_id = 1;
	int color = 0;
	bool fixed = false;
};

static bool WriteText(dyn_arg_list args) {
	auto func = "write_text";
	bool okay;
	std::string id, text;
	int x, y;

	std::tie(id, x, y, text) = DynRpg::ParseArgs<std::string, int, int, std::string>(func, args, &okay);
	if (!okay)
		return true;

	PendingMessage pm(DynRpgText::CommandCodeInserter);
	pm.PushLine(id);
	std::string new_id = pm.GetLines().front();
	graphics[new_id] = std::make_unique<DynRpgText>(1, x, y, text);

	if (args.size() > 4) {
		std::string fixed = std::get<0>(DynRpg::ParseArgs<std::string>(func, args.subspan(4), &okay));
		if (!okay)
			return true;
		graphics[new_id]->SetFixed(fixed == "fixed");
	}

	if (args.size() > 5) {
		int color = std::get<0>(DynRpg::ParseArgs<int>(func, args.subspan(5), &okay));
		if (!okay)
			return true;
		graphics[new_id]->SetColor(color);
	}

	if (args.size() > 6) {
		int pic_id = std::get<0>(DynRpg::ParseArgs<int>(func, args.subspan(6), &okay));
		if (!okay)
			return true;
		graphics[new_id]->SetPictureId(pic_id);
	}

	return true;
}

static bool AppendLine(dyn_arg_list args) {
	auto func = "append_line";
	bool okay;
	std::string id, text;

	std::tie(id, text) = DynRpg::ParseArgs<std::string, std::string>(func, args, &okay);
	if (!okay)
		return true;

	DynRpgText* handle = DynRpgText::GetTextHandle(id);
	if (!handle) {
		return true;
	}

	handle->AddLine(text);
	return true;
}

static bool AppendText(dyn_arg_list args) {
	auto func = "append_line";
	bool okay;
	std::string id, text;

	std::tie(id, text) = DynRpg::ParseArgs<std::string, std::string>(func, args, &okay);
	if (!okay)
		return true;

	DynRpgText* handle = DynRpgText::GetTextHandle(id);
	if (!handle) {
		return true;
	}

	handle->AddText(text);
	return true;
}

static bool ChangeText(dyn_arg_list args) {
	auto func = "change_text";
	bool okay;
	std::string id, text, color;

	// Color can be a string (usually "end") or a number
	std::tie(id, text, color) = DynRpg::ParseArgs<std::string, std::string, std::string>(func, args, &okay);
	if (!okay)
		return true;

	DynRpgText* handle = DynRpgText::GetTextHandle(id);
	if (!handle) {
		return true;
	}

	handle->ClearText();
	if (color != "end") {
		handle->SetColor(atoi(color.c_str()));
	}
	handle->AddText(text);
	return true;
}

static bool ChangePosition(dyn_arg_list args) {
	auto func = "change_position";
	bool okay;
	std::string id;
	int x, y;

	std::tie(id, x, y) = DynRpg::ParseArgs<std::string, int, int>(func, args, &okay);
	if (!okay)
		return true;

	DynRpgText* handle = DynRpgText::GetTextHandle(id);
	if (!handle) {
		return true;
	}

	handle->SetPosition(x, y);
	return true;
}

static bool RemoveText(dyn_arg_list args) {
	auto func = "remove_text";
	bool okay;
	std::string id;

	std::tie(id) = DynRpg::ParseArgs<std::string>(func, args, &okay);
	if (!okay)
		return true;

	DynRpgText* handle = DynRpgText::GetTextHandle(id, true);
	if (!handle) {
		return true;
	}

	handle->ClearText();
	return true;
}

static bool RemoveAll(dyn_arg_list) {
	graphics.clear();
	return true;
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
				// ignore transparency, the picture defines this
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

			graphics[id] = std::make_unique<DynRpgText>(pic_id, x, y, texts);
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
