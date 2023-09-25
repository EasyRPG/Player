#include "chatui.h"
#include <memory>
#include <vector>
#include "../window_base.h"
#include "../scene.h"
#include "../bitmap.h"
#include "../output.h"
#include "../drawable_mgr.h"
#include "../font.h"
#include "../cache.h"
#include "../input.h"
#include "../utils.h"
#include "../player.h"
#include "../compiler.h"
#include "../baseui.h"
#include "../graphics.h"
#include "../statustext_overlay.h"
#include "game_multiplayer.h"
#include "server.h"
#include "strfnd.h"
#include "cryptopp/pwdbased.h"
#include "cryptopp/sha.h"
#include "cryptopp/crc.h"
#include "cryptopp/base64.h"
#include "cryptopp/osrng.h"
#include "cryptopp/filters.h"
#include "cryptopp/aes.h"
#include "cryptopp/gcm.h"

class DrawableOnlineStatus : public Drawable {
	Rect bounds;

	// design parameters
	const unsigned int padding_horz = 4; // padding between box edges and content (left)
	const unsigned int padding_vert = 6; // padding between box edges and content (top)

	BitmapRef conn_status;
	BitmapRef room_status;
public:
	DrawableOnlineStatus(int x, int y, int w, int h)
		: Drawable(Priority::Priority_Maximum, Drawable::Flags::Global),
		bounds(x, y, w, h)
	{
		DrawableMgr::Register(this);

		// initialize
		SetConnectionStatus(false);
		SetRoomStatus(0);
	}

	void Draw(Bitmap& dst) {
		dst.Blit(bounds.x+padding_horz, bounds.y+padding_vert,
			*conn_status, conn_status->GetRect(), Opacity::Opaque());
		auto r_rect = room_status->GetRect();
		dst.Blit(bounds.x+bounds.width-padding_horz-r_rect.width, bounds.y+padding_vert,
			*room_status, r_rect, Opacity::Opaque());
	};

	void RefreshTheme() { }

	void SetConnectionStatus(bool status, bool connecting = false) {
		std::string conn_label = "";
		if (connecting)
			conn_label = "Connecting";
		else
			conn_label = status ? "Connected" : "Disconnected";

		auto c_rect = Text::GetSize(*Font::Default(), conn_label);
		conn_status = Bitmap::Create(c_rect.width+1, c_rect.height+1, true);
		Text::Draw(*conn_status, 0, 0, *Font::Default(), *Cache::SystemOrBlack(), 2, conn_label);
	}

	void SetRoomStatus(unsigned int room_id) {
		std::string room_label = "";
		room_label = "Room #"+std::to_string(room_id);
		auto r_rect = Text::GetSize(*Font::Default(), room_label);
		room_status = Bitmap::Create(r_rect.width+1, r_rect.height+1, true);
		Text::Draw(*room_status, 0, 0, *Font::Default(), *Cache::SystemOrBlack(), 1, room_label);
	}
};

using VisibilityType = Messages::VisibilityType;

struct ChatEntry {
	std::string color_a; // text a
	std::string color_b; // text b
	std::string color_c; // text c
	std::string color_d; // text d
	std::string color_e; // text e
	int8_t _color_a; // color selection _a of text a
	int8_t _color_b; // color selection _b of text b
	int8_t _color_c; // color selection _c of text c
	int8_t _color_d; // color selection _d of text d
	int8_t _color_e; // color selection _e of text e
	VisibilityType visibility;
	std::string sys_name;
	bool break_word;
	ChatEntry(std::string a, std::string b, std::string c, std::string d, std::string e,
			int8_t _a, int8_t _b, int8_t _c, int8_t _d, int8_t _e,
			VisibilityType v, std::string sys, bool bw) {
		color_a = a;
		color_b = b;
		color_c = c;
		color_d = d;
		color_e = e;
		_color_a = _a;
		_color_b = _b;
		_color_c = _c;
		_color_d = _d;
		_color_e = _e;
		visibility = v;
		sys_name = sys;
		break_word = bw;
	}
};

class DrawableChatLog : public Drawable {
	struct DrawableChatEntry {
		ChatEntry* message_data;
		BitmapRef render_graphic;
		bool dirty; // need to redraw? (for when UI skin changes)
		DrawableChatEntry(ChatEntry* msg) {
			message_data = msg;
			render_graphic = nullptr;
			dirty = true;
		}
	};

	Rect bounds;
	// design parameters
	const unsigned int message_margin = 2; // horizontal margin between panel edges and content
	const unsigned int scroll_frame = 8; // width of scroll bar's visual frame (on right side)
	const unsigned int scroll_bleed = 2; // how much to stretch right edge of scroll box offscreen (so only left frame shows)

	bool overlay_flag = false;
	float counter = 0;
	std::vector<DrawableChatEntry> messages;
	Window_Base scroll_box; // box used as rendered design for a scrollbar
	int scroll_position = 0;
	unsigned int scroll_content_height = 0; // total height of scrollable message log
	unsigned short visibility_flags = Messages::CV_LOCAL | Messages::CV_GLOBAL | Messages::CV_CRYPT;
	BitmapRef default_theme; // system graphic for the default theme
	BitmapRef current_theme; // system graphic for the current theme

	void BuildMessageGraphic(DrawableChatEntry& msg) {
		struct Glyph {
			Utils::TextRet data;
			Rect dims;
			int8_t color;
		};
		using GlyphLine = std::vector<Glyph>;
		auto ExtractGlyphs = [](StringView str, int8_t color, GlyphLine& line, unsigned int& width) {
			const auto* iter = str.data();
			const auto* end = str.data() + str.size();
			while(iter != end) {
				auto resp = Utils::TextNext(iter, end, 0);
				iter = resp.next;

				Rect ch_rect;
				if(resp.is_exfont)
					ch_rect = Text::GetSize(*Font::exfont, " ");
				else
					ch_rect = Font::Default()->GetSize(resp.ch);

				line.push_back({resp, ch_rect, color});
				width += ch_rect.width;
			}
		};
		auto FindFirstGlyphOf = [](GlyphLine& line, char32_t ch) -> int {
			unsigned int n_glyphs = line.size();
			for(int i = 0; i < n_glyphs; i++) {
				if(line[i].data.ch == ch) {
					return i;
				}
			}
			return -1;
		};
		auto FindLastGlyphOf = [](GlyphLine& line, char32_t ch) -> int {
			unsigned int n_glyphs = line.size();
			for(int i = n_glyphs-1; i >= 0; i--) {
				if(line[i].data.ch == ch) {
					return i;
				}
			}
			return -1;
		};
		auto MoveGlyphsToNext = [](GlyphLine& curr, GlyphLine& next,
				unsigned int& curr_width, unsigned int& next_width, unsigned int amount) {
			for(int i = 0; i < amount; i++) {
				auto& glyph = curr.back();
				unsigned int delta_width = glyph.dims.width;
				next.insert(next.begin(), glyph);
				next_width += delta_width;
				curr.pop_back();
				curr_width -= delta_width;
			}
		};
		auto GetLineHeight = [](GlyphLine& line) -> unsigned int {
			unsigned int height = 0;
			unsigned int n_glyphs = line.size();
			for(int i = 0; i < n_glyphs; i++) {
				height = std::max<unsigned int>(height, line[i].dims.height);
			}
			return height;
		};

		// manual text wrapping
		const unsigned int max_width = bounds.width-scroll_frame-message_margin;

		// individual lines saved so far, along with their y offset
		std::vector<std::pair<GlyphLine, unsigned int>> lines;
		unsigned int total_width = 0; // maximum width between all lines
		unsigned int total_height = 0; // accumulated height from all lines

		GlyphLine glyphs_current; // list of glyphs and their dimensions on current line
		GlyphLine glyphs_next; // stores glyphs moved down to line below
		unsigned int width_current = 0; // total width of glyphs on current line
		unsigned int width_next = 0; // total width of glyphs on next line

		BitmapRef graphic;
		if (msg.message_data->sys_name == "") {
			graphic = current_theme;
		} else {
			graphic = Cache::System(msg.message_data->sys_name);
		}

		// break down whole message string into glyphs for processing.
		// glyph lookup is performed only at this stage, and their dimensions are saved for subsequent line width recalculations.
		ExtractGlyphs(msg.message_data->color_a, msg.message_data->_color_a, glyphs_current, width_current);
		ExtractGlyphs(msg.message_data->color_b, msg.message_data->_color_b, glyphs_current, width_current);
		ExtractGlyphs(msg.message_data->color_c, msg.message_data->_color_c, glyphs_current, width_current);
		ExtractGlyphs(msg.message_data->color_d, msg.message_data->_color_d, glyphs_current, width_current);
		ExtractGlyphs(msg.message_data->color_e, msg.message_data->_color_e, glyphs_current, width_current);

		bool break_word = msg.message_data->break_word;

		// break down message into fitting lines
		do {
			while(width_current > max_width) {
				// as long as current line exceeds maximum width,
				// move one word from this line down to the next one
				int last_space = FindLastGlyphOf(glyphs_current, ' ');
				if(break_word && last_space != -1 && last_space < glyphs_current.size()-1) {
					// there is a word that can be moved down
					MoveGlyphsToNext(glyphs_current, glyphs_next,
						width_current, width_next, glyphs_current.size()-last_space-1);
				} else {
					// there is not a whole word that can be moved down, so move individual characters.
					// this case happens when last character in current line is a space,
					// or when there are no spaces in the current line
					MoveGlyphsToNext(glyphs_current, glyphs_next, width_current, width_next, 1);
				}
			}
			// once line fits, check for line breaks
			int line_break = FindFirstGlyphOf(glyphs_current, '\n');
			if(line_break != -1) {
				MoveGlyphsToNext(glyphs_current, glyphs_next,
					width_current, width_next, glyphs_current.size()-line_break-1);
			}
			// save line
			lines.push_back(std::make_pair(glyphs_current, total_height));
			total_width = std::max<unsigned int>(total_width, width_current);
			total_height += GetLineHeight(glyphs_current);
			// repeat this work on the exceeding portion moved down to the line below
			glyphs_current = glyphs_next;
			glyphs_next.clear();
			width_current = width_next;
			width_next = 0;
			if (overlay_flag && glyphs_current.size() > 0) {
				// use the '>' as the truncated signs
				auto& last_glyph = lines.front().first.back();
				last_glyph.data.ch = '>';
				last_glyph.color = 2;
				break;
			}
		} while(glyphs_current.size() > 0);

		unsigned int message_padding = 1;
		if (overlay_flag)
			message_padding = 2;

		// once all lines have been saved
		// render them into a bitmap
		BitmapRef text_img = Bitmap::Create(total_width+message_padding*2, total_height+message_padding*2, true);
		// add background
		if (overlay_flag)
			text_img->Fill(Color(0, 0, 0, 102));
		int n_lines = lines.size();
		for(int i = 0; i < n_lines; i++) {
			auto& line = lines[i];
			int glyph_offset = 0;
			for(int j = 0; j < line.first.size(); j++) {
				auto& glyph = line.first[j];
				auto ret = glyph.data;
				if(EP_UNLIKELY(!ret)) continue;
				if (glyph.color > -1) {
					glyph_offset += Text::Draw(*text_img, glyph_offset+message_padding, line.second+message_padding,
						*Font::Default(), *graphic, glyph.color, ret.ch, ret.is_exfont).x;
				} else {
					glyph_offset += Text::Draw(*text_img, glyph_offset+message_padding, line.second+message_padding,
						*Font::Default(), *default_theme, 0, ret.ch, ret.is_exfont).x;
				}
			}
		}
		msg.render_graphic = text_img;
		msg.dirty = false;
	}

	void AssertScrollBounds() {
		// maximum value for scroll_position (= amount of height escaping from the top)
		const unsigned int max_scroll = std::max<int>(0, scroll_content_height-bounds.height);
		scroll_position = std::max<int>(scroll_position, 0);
		scroll_position = std::min<int>(scroll_position, max_scroll);
	}

	void UpdateScrollBar() {
		if(scroll_content_height <= bounds.height) {
			// hide scrollbar if content isn't large enough for scroll
			scroll_box.SetX(bounds.x+bounds.width);
			return;
		}
		scroll_box.SetX(bounds.x+bounds.width-scroll_frame); // show scrollbar
		// position scrollbar
		const float ratio = bounds.height/float(scroll_content_height);
		const unsigned int bar_height = bounds.height*ratio;
		// clamp the scroll_box minimum height
		const unsigned int bar_height_safe = std::max<unsigned int>(bar_height, 16);
		const unsigned int bar_y = scroll_position*ratio;
		const unsigned int bar_offset_safe = (bar_height_safe-bar_height)*
			(1.0f-float(bar_y)/bounds.height);
		scroll_box.SetHeight(bar_height_safe);
		scroll_box.SetY(bounds.y+bounds.height-bar_y-bar_height-bar_offset_safe);
	}

	// called when:
	// - scroll position changes
	// - content height changes
	// - visible height (bounds.height) changes
	void RefreshScroll() {
		AssertScrollBounds();
		UpdateScrollBar();
	}

	bool MessageVisible(DrawableChatEntry& msg, unsigned short v) {
		return (msg.message_data->visibility & v) > 0;
	}
public:
	DrawableChatLog(int x, int y, int w, int h)
		: Drawable(Priority::Priority_Maximum, Drawable::Flags::Global),
		bounds(x, y, w, h),
		scroll_box(0, 0, scroll_frame+scroll_bleed, 0, Drawable::Flags::Global)
	{
		DrawableMgr::Register(this);

		scroll_box.SetZ(Priority::Priority_Maximum);
		scroll_box.SetVisible(false);

		current_theme = Cache::SystemOrBlack();
		default_theme = current_theme;
	}

	void SetOverlayMode(bool enabled) {
		overlay_flag = enabled;
	}

	void SetHeight(unsigned int h) {
		bounds.height = h;
		RefreshScroll();
	}

	void Draw(Bitmap& dst) {
		// y offset to draw next message, from bottom of log panel
		int next_height = -scroll_position;
		unsigned int n_messages = messages.size();
		for(int i = n_messages-1; i >= 0; i--) {
			DrawableChatEntry& dmsg = messages[i];
			// skip drawing hidden messages
			if(!MessageVisible(dmsg, visibility_flags))
				continue;
			auto rect = dmsg.render_graphic->GetRect();
			// accumulate y offset
			next_height += rect.height;
			// skip drawing offscreen messages, but still accumulate y offset (bottom offscreen)
			if(next_height <= 0)
				continue;
			// cutoff message graphic so text does not bleed out of bounds
			const unsigned int top_offscreen = std::max<int>(next_height-bounds.height, 0);
			Rect cutoff_rect = Rect(rect.x, rect.y+top_offscreen,
				rect.width, std::min<unsigned int>(rect.height, next_height)-top_offscreen);
			// rebuild message graphic if needed
			if(messages[i].dirty)
				BuildMessageGraphic(dmsg);
			// draw
			unsigned int margin = message_margin;
			if (overlay_flag)
				margin = 0;
			dst.Blit(bounds.x+margin, bounds.y+bounds.height-next_height+top_offscreen,
				*(dmsg.render_graphic), cutoff_rect, Opacity::Opaque());
			// stop drawing offscreen messages (top offscreen)
			if(next_height > bounds.height)
				break;
		}

		// automatically remove messages
		if (overlay_flag && messages.size() > 0) {
			++counter;
			// the delay is 10 seconds
			if (Game_Clock::GetFPS() > 0.0f && counter > Game_Clock::GetFPS()*10.0f) {
				counter = 0.0f;
				RemoveFirstChatEntry();
			}
		}
	};

	void RefreshTheme() {
		auto new_theme = Cache::SystemOrBlack();

		// do nothing if theme hasn't changed
		if(new_theme == current_theme)
			return;

		current_theme = new_theme;
		scroll_box.SetWindowskin(current_theme);
		for(int i = 0; i < messages.size(); i++) {
			// all messages now need to be redrawn with different UI skin
			messages[i].dirty = true;
		}
	}

	void AddChatEntry(ChatEntry* message_data) {
		DrawableChatEntry d_msg = DrawableChatEntry(message_data);
		BuildMessageGraphic(d_msg);
		messages.push_back(d_msg);

		if(MessageVisible(d_msg, visibility_flags)) {
			scroll_content_height += d_msg.render_graphic->GetRect().height;
			RefreshScroll();
		}
	}

	void RemoveChatEntry(ChatEntry* message_data) {
		unsigned int n_msgs = messages.size();
		for(int i = 0; i < n_msgs; i++) {
			DrawableChatEntry& d_msg = messages[i];
			if(d_msg.message_data == message_data) {
				if(MessageVisible(d_msg, visibility_flags)) {
					scroll_content_height -= d_msg.render_graphic->GetRect().height;
					RefreshScroll();
				}
				messages.erase(messages.begin()+i);
				break;
			}
		}
	}

	void RemoveFirstChatEntry() {
		if (messages.size() > 0) {
			RemoveChatEntry(messages.front().message_data);
		}
	}

	size_t GetSize() {
		return messages.size();
	}

	void SetScroll(int s) {
		scroll_position = s;
		RefreshScroll();
	}

	void Scroll(int delta) {
		SetScroll(scroll_position+delta);
	}

	void ShowScrollBar(bool v) {
		scroll_box.SetVisible(v);
	}

	unsigned short GetVisibilityFlags() {
		return visibility_flags;
	}

	void ToggleVisibilityFlag(VisibilityType v) {
		// Expands/collapses messages in-place,
		// so you don't get lost if you've scrolled far up.
		//
		// Finds the bottommost (before the change) message that is visible both before and after changing visibility flags, and
		// anchors it into place so it stays at the same visual location before and
		// after expanding/collapsing.

		unsigned short new_visibility_flags = visibility_flags ^ v;
		// calculate new total content height for new visibility flags.
		unsigned int new_content_height = 0;
		// save anchor position before and after
		int pre_anchor_y = -scroll_position;
		int post_anchor_y = -scroll_position;
		// if true, anchor has been found, so stop accumulating message heights
		bool anchored = false;

		for(int i = messages.size()-1; i >= 0; i--) {
			// is message visible with previous visibility mask?
			bool pre_vis = MessageVisible(messages[i], visibility_flags);
			// is message visible with new visibility mask?
			bool post_vis = MessageVisible(messages[i], new_visibility_flags);
			unsigned int msg_height = messages[i].render_graphic->GetRect().height;
			// accumulate total content height for new visibility flags
			if(post_vis) new_content_height += msg_height;

			if(!anchored) {
				if(pre_vis)
					pre_anchor_y += msg_height;
				if(post_vis)
					post_anchor_y += msg_height;
				// this message is an anchor candidate since it is visible both before and after
				bool valid_anchor = pre_vis && post_vis;
				if(valid_anchor && pre_anchor_y > 0) {
					// this is the bottommost anchorable message
					anchored = true;
				}
			}
		}
		// updates scroll content height
		scroll_content_height = new_content_height;
		// adjusts scroll position so anchor stays in place
		int scroll_delta = post_anchor_y-pre_anchor_y;
		scroll_position += scroll_delta;
		RefreshScroll();
		// set new visibility mask
		visibility_flags = new_visibility_flags;
	}
};

class DrawableTypeBox : public Drawable {
	Rect bounds;
	// design parameters
	const unsigned int label_padding_horz = 6; // left margin between label text and bounds
	const unsigned int label_padding_vert = 3; // top margin between label text and bounds
	const unsigned int label_margin = 35; // left margin for type box to make space for the label
	// amount that is visible outside the padded bounds (so text can be seen beyond a left or rightmost placed caret)
	const unsigned int type_bleed = 0;
	const unsigned int type_padding_horz = 12; // padding between type box edges and content (left)
	const unsigned int type_padding_vert = 3; // padding between type box edges and content (top)

	BitmapRef label;
	BitmapRef type_text;
	BitmapRef caret;
	unsigned int caret_index_tail = 0;
	unsigned int caret_index_head = 0;
	// cumulative x offsets for each character in the type box. Used for rendering the caret
	std::vector<unsigned int> type_char_offsets;
	unsigned int scroll = 0; // horizontal scrolling of type box

	unsigned int get_label_margin() {
		return (label->GetRect().width>1) ? label_margin : 0;
	}
public:
	DrawableTypeBox(int x, int y, int w, int h)
		: Drawable(Priority::Priority_Maximum, Drawable::Flags::Global),
		bounds(x, y, w, h)
	{
		DrawableMgr::Register(this);

		// create caret (type cursor) graphic
		std::string caret_char = "｜";
		const unsigned int caret_left_kerning = 6;
		auto c_rect = Text::GetSize(*Font::Default(), caret_char);
		caret = Bitmap::Create(c_rect.width+1-caret_left_kerning, c_rect.height+1, true);
		Text::Draw(*caret, -caret_left_kerning, 0, *Font::Default(), *Cache::SystemOrBlack(), 0, caret_char);

		// initialize
		UpdateTypeText(std::u32string());
		SetLabel("");
	}

	void Draw(Bitmap& dst) {
		const unsigned int label_pad = get_label_margin();
		const unsigned int type_visible_width = bounds.width-type_padding_horz*2-label_pad;
		auto rect = type_text->GetRect();
		// crop type text to stay within padding
		Rect cutoff_rect = Rect(scroll-type_bleed, rect.y,
			std::min<int>(type_visible_width, rect.width-scroll)+type_bleed*2, rect.height);

		// draw label
		dst.Blit(bounds.x+label_padding_horz, bounds.y+label_padding_vert,
			*label, label->GetRect(), Opacity::Opaque());
		// draw contents
		dst.Blit(bounds.x+label_pad+type_padding_horz-type_bleed, bounds.y+type_padding_vert,
			*type_text, cutoff_rect, Opacity::Opaque());
		// draw caret
		dst.Blit(bounds.x+label_pad+type_padding_horz+type_char_offsets[caret_index_head]-scroll,
			bounds.y+type_padding_vert, *caret, caret->GetRect(), Opacity::Opaque());
		// draw selection
		const unsigned int caret_start = std::min<unsigned int>(caret_index_tail, caret_index_head);
		const unsigned int caret_end = std::max<unsigned int>(caret_index_tail, caret_index_head);
		const unsigned int select_start = bounds.x+label_pad+type_padding_horz
			+std::max<int>(type_char_offsets[caret_start]-scroll, -type_bleed);
		const unsigned int select_end = bounds.x+label_pad+type_padding_horz
			+std::min<int>(type_char_offsets[caret_end]-scroll, type_visible_width+type_bleed);
		Rect selected_rect = Rect(select_start, bounds.y+type_padding_vert,
			select_end-select_start, bounds.height-type_padding_vert*2);
		dst.FillRect(selected_rect, Color(255, 255, 255, 100));
	};

	void RefreshTheme() { }

	void UpdateTypeText(std::u32string text) {
		// get char offsets for each character in type box, for caret positioning
		type_char_offsets.clear();
		Rect accumulated_rect;
		const unsigned int n_chars = text.size();
		for(int k = 0; k <= n_chars; k++) {
			// for every substring of sizes 0 to N, inclusive, starting at the left
			std::u32string text_so_far = text.substr(0, k);
			// absolute offset of character at this point (considering kerning of all previous ones)
			accumulated_rect = Text::GetSize(*Font::Default(), Utils::EncodeUTF(text_so_far));
			type_char_offsets.push_back(accumulated_rect.width);
		}

		// final value assigned to accumulated_rect is whole type string
		// create Bitmap graphic for text
		type_text = Bitmap::Create(accumulated_rect.width+1, accumulated_rect.height+1, true);
		Text::Draw(*type_text, 0, 0, *Font::Default(), *Cache::SystemOrBlack(), 0, Utils::EncodeUTF(text));
	}

	const int GetCaretRelativeOffset() {
		return type_char_offsets[caret_index_head]-scroll;
	}

	void SeekCaret(unsigned int seek_tail, unsigned int seek_head) {
		caret_index_tail = seek_tail;
		caret_index_head = seek_head;
		// adjust type box horizontal scrolling based on caret position (always keep it in-bounds)
		const unsigned int label_pad = get_label_margin();
		const unsigned int type_visible_width = bounds.width-type_padding_horz*2-label_pad;
		// absolute offset of caret in relation to type text contents
		const unsigned int caret_offset = type_char_offsets[caret_index_head];
		// caret's position relative to viewable portion of type box
		const int relative_offset = caret_offset-scroll;
		if(relative_offset < 0) {
			// caret escapes from left side. adjust
			scroll += relative_offset;
		} else if(relative_offset >= type_visible_width) {
			// caret escapes from right side. adjust
			scroll += relative_offset-type_visible_width;
		}
	}

	void MoveScrollToRight() {
		const unsigned int caret_offset = type_char_offsets[caret_index_head];
		const int relative_offset = caret_offset-scroll;
		scroll = scroll-(caret_offset-relative_offset);
	}

	void SetLabel(std::string l) {
		auto l_rect = Text::GetSize(*Font::Default(), l);
		label = Bitmap::Create(l_rect.width+1, l_rect.height+1, true);
		Text::Draw(*label, 0, 0, *Font::Default(), *Cache::SystemOrBlack(), 1, l);
	}

	Rect GetFormBounds() {
		const unsigned int label_pad = get_label_margin();
		return Rect(bounds.x+label_pad+type_padding_horz, bounds.y,
			bounds.width-label_pad-type_padding_horz, bounds.height);
	}
};

class DrawableChatUi : public Drawable {
	// design parameters
	const unsigned int chat_width = Player::screen_width*0.725;
	const unsigned int chat_left = Player::screen_width-chat_width;
	const unsigned int notification_log_height = Player::screen_height*0.275;
	const unsigned int panel_frame_left_top = 4; // width of panel's visual frame (border width is missing)
	const unsigned int panel_frame_right_bottom = 6; // on right and bottom side (including border width)
	const unsigned int status_height = 19; // height of status region on top of chatlog
	const unsigned int log_scroll_delta = (Player::screen_height-status_height)/16;
	const unsigned int type_height = 19; // height of type box
	const unsigned int type_border_offset = 8; // width of type border offset

	DrawableChatLog d_notification_log;
	bool notification_log_shown = true;

	Window_Base back_panel; // background pane
	DrawableOnlineStatus d_status;
	DrawableChatLog d_log;
	DrawableTypeBox d_type;

	void UpdateTypePanel() {
		if(d_type.IsVisible()) {
			// SetCursorRect for some reason already has a padding of 8px relative to the window, so we fix it
			const unsigned int f = -8;
			const auto form_rect = d_type.GetFormBounds();
			back_panel.SetCursorRect(
				Rect((f-type_border_offset)+form_rect.x-chat_left, f+form_rect.y,
					form_rect.width+type_border_offset, form_rect.height));
		} else {
			back_panel.SetCursorRect(Rect(0, 0, 0, 0));
		}
	}
public:
	DrawableChatUi()
		: Drawable(Priority::Priority_Maximum, Drawable::Flags::Global),
		d_notification_log(0, Player::screen_height-notification_log_height,
			Player::screen_width, notification_log_height),
		back_panel(chat_left, 0, chat_width, Player::screen_height, Drawable::Flags::Global),
		d_status(chat_left+panel_frame_left_top, 0, chat_width-panel_frame_right_bottom, status_height),
		d_log(chat_left+panel_frame_left_top, status_height,
			chat_width-panel_frame_right_bottom, Player::screen_height-status_height),
		d_type(chat_left+panel_frame_left_top, Player::screen_height-type_height-panel_frame_left_top,
			chat_width-panel_frame_right_bottom-type_border_offset, type_height)
	{
		DrawableMgr::Register(this);

		back_panel.SetZ(Priority::Priority_Maximum-1);
		back_panel.SetOpacity(240);

		d_notification_log.SetOverlayMode(true);

		SetFocus(false);
	}

	void Draw(Bitmap& dst) { }

	void AddLogEntry(ChatEntry* msg) {
		d_log.AddChatEntry(msg);
	}

	void AddNotificationLogEntry(ChatEntry* msg, unsigned int limit) {
		if (d_notification_log.GetSize() > limit-1)
			d_notification_log.RemoveFirstChatEntry();
		d_notification_log.AddChatEntry(msg);
	}

	void RemoveLogEntry(ChatEntry* msg) {
		d_log.RemoveChatEntry(msg);
	}

	void SetStatusConnection(bool conn, bool connecting) {
		d_status.SetConnectionStatus(conn, connecting);
	}

	void SetStatusRoom(unsigned int room_id) {
		d_status.SetRoomStatus(room_id);
	}

	void RefreshTheme() {
		d_notification_log.RefreshTheme();
		back_panel.SetWindowskin(Cache::SystemOrBlack());
		d_status.RefreshTheme();
		d_log.RefreshTheme();
		d_type.RefreshTheme();
	}

	void UpdateTypeText(std::u32string text, unsigned int caret_seek_tail, unsigned int caret_seek_head) {
		d_type.UpdateTypeText(text);
		d_type.SeekCaret(caret_seek_tail, caret_seek_head);
	}

	void UpdateTypeCaret(unsigned int caret_seek_tail, unsigned int caret_seek_head) {
		d_type.SeekCaret(caret_seek_tail, caret_seek_head);
	}

	void UpdateTypeScroll() {
		d_type.MoveScrollToRight();
	}

	void UpdateTypeTextInputRect() {
		const auto form_rect = d_type.GetFormBounds();
		Rect metrics = DisplayUi->GetWindowMetrics();
		int scale_factor_width = metrics.width/Player::screen_width;
		int scale_factor_height = metrics.height/Player::screen_height;
		DisplayUi->SetTextInputRect(
			(form_rect.x+d_type.GetCaretRelativeOffset())*scale_factor_width,
			(form_rect.y+form_rect.height-6)*scale_factor_height
		);
	}

	void ShowTypeLabel(std::string label) {
		d_type.SetLabel(label);
		UpdateTypePanel();
	}

	void ScrollUp() {
		d_log.Scroll(+log_scroll_delta);
	}

	void ScrollDown() {
		d_log.Scroll(-log_scroll_delta);
	}

	void SetFocus(bool focused) {
		if (notification_log_shown)
			d_notification_log.SetVisible(!focused);
		this->SetVisible(focused);
		back_panel.SetVisible(focused);
		d_status.SetVisible(focused);
		d_log.SetVisible(focused);
		d_type.SetVisible(focused);
		UpdateTypePanel();
		d_log.ShowScrollBar(focused);
		if(focused) {
			d_log.SetHeight(Player::screen_height-status_height-type_height-panel_frame_right_bottom);
			DisplayUi->StartTextInput();
			UpdateTypeTextInputRect();
		} else {
			d_log.SetHeight(Player::screen_height-status_height-panel_frame_right_bottom);
			d_log.SetScroll(0);
			DisplayUi->StopTextInput();
		}
	}

	unsigned short GetVisibilityFlags() {
		return d_log.GetVisibilityFlags();
	}

	void ToggleVisibilityFlag(VisibilityType v) {
		d_notification_log.ToggleVisibilityFlag(v);
		d_log.ToggleVisibilityFlag(v);
	}

	void ToggleNotificationLog() {
		notification_log_shown = !notification_log_shown;
		d_notification_log.SetVisible(notification_log_shown);
		Graphics::GetStatusTextOverlay().ShowText(
			notification_log_shown ? "Notifications shown" : "Notifications hidden"
		);
	}
};

const unsigned int MAXCHARSINPUT_MESSAGE = 200;
const unsigned int MAXMESSAGES = 500;
const unsigned int MAXNOTIFICATIONS = 3;

std::u32string type_text;
unsigned int type_caret_index_tail = 0; // anchored when SHIFT-selecting text
unsigned int type_caret_index_head = 0; // moves when SHIFT-selecting text
std::mutex chat_mutex;
std::unique_ptr<DrawableChatUi> chat_box; // chat renderer
std::vector<std::unique_ptr<ChatEntry>> chat_log;
std::vector<std::unique_ptr<ChatEntry>> chat_notification_log;
VisibilityType chat_visibility = Messages::CV_LOCAL;

void AddLogEntry(
		std::string a, std::string b, std::string c, std::string d, std::string e,
		int8_t _a, int8_t _b, int8_t _c, int8_t _d, int8_t _e,
		VisibilityType v, std::string sys_name) {
	const std::lock_guard<std::mutex> lock(chat_mutex);
	chat_log.push_back(std::make_unique<ChatEntry>(
			a, b, c, d, e, _a, _b, _c, _d, _e, v, sys_name, true));
	chat_box->AddLogEntry(chat_log.back().get());
	if(chat_log.size() > MAXMESSAGES) {
		chat_box->RemoveLogEntry(chat_log.front().get());
		chat_log.erase(chat_log.begin());
	}
}

void AddLogEntry(std::string a, std::string b, std::string c, VisibilityType v) {
	AddLogEntry(a, b, c, "", "", 1, 2, 0, 0, 0, v, "");
}

void AddNotificationLogEntry(
		std::string a, std::string b, std::string c, std::string d, std::string e,
		int8_t _a, int8_t _b, int8_t _c, int8_t _d, int8_t _e,
		VisibilityType v, std::string sys_name) {
	const std::lock_guard<std::mutex> lock(chat_mutex);
	chat_notification_log.push_back(std::make_unique<ChatEntry>(
			a, b, c, d, e, _a, _b, _c, _d, _e, v, sys_name, false));
	chat_box->AddNotificationLogEntry(chat_notification_log.back().get(), MAXNOTIFICATIONS);
	if(chat_notification_log.size() > MAXNOTIFICATIONS)
		chat_notification_log.erase(chat_notification_log.begin());
}

void AddNotificationLogEntry(std::string a, std::string b, std::string c, VisibilityType v) {
	AddNotificationLogEntry(a, b, c, "", "", 1, 2, 0, 0, 0, v, "");
}

void AddClientInfo(std::string message) {
	AddLogEntry("[Client]: ", message, "", Messages::CV_LOCAL);
}

// https://www.cryptopp.com/wiki/PKCS5_PBKDF2_HMAC
std::string GetPasswordHash(const std::string& password, unsigned int iterations = 600000) {
	std::string salt = "";

	CryptoPP::byte derived[CryptoPP::SHA256::DIGESTSIZE];

	CryptoPP::PKCS5_PBKDF2_HMAC<CryptoPP::SHA256> pbkdf;
	CryptoPP::byte unused = 0;

	pbkdf.DeriveKey(derived, sizeof(derived), unused,
		(CryptoPP::byte*)password.c_str(), password.length(),
		(CryptoPP::byte*)salt.c_str(), salt.length(),
		iterations, 0.0f);

	std::string result;
	CryptoPP::Base64Encoder encoder(new CryptoPP::StringSink(result), false);
	encoder.Put(derived, sizeof(derived));
	encoder.MessageEnd();

	return std::move(result);
}

uint32_t StringToCRC32(std::string& value) {
	std::string digest;
	CryptoPP::CRC32C crc;
	crc.Update((CryptoPP::byte*)&value[0], value.size());
	digest.resize(crc.DigestSize());
	crc.Final((CryptoPP::byte*)&digest[0]);
	return *((uint32_t*)digest.data());
}

// https://cryptopp.com/wiki/Hash_Functions
// https://cryptopp.com/wiki/Authenticated_Encryption
std::string EncryptMessage(const std::string& password, const std::string& plain) {
	CryptoPP::byte key[CryptoPP::AES::MAX_KEYLENGTH];
	CryptoPP::SHA256().CalculateDigest(key, (CryptoPP::byte*)password.c_str(), password.length());

	CryptoPP::AutoSeededRandomPool prng;
	CryptoPP::byte iv[CryptoPP::AES::BLOCKSIZE];
	prng.GenerateBlock(iv, CryptoPP::AES::BLOCKSIZE);

	std::string cipher;
	try {
		CryptoPP::GCM<CryptoPP::AES>::Encryption encryptor;
		encryptor.SetKeyWithIV(key, CryptoPP::AES::MAX_KEYLENGTH, iv, CryptoPP::AES::BLOCKSIZE);

		CryptoPP::StringSource(plain, true,
			new CryptoPP::AuthenticatedEncryptionFilter(encryptor,
				new CryptoPP::StringSink(cipher)
			)
		);
	} catch(const CryptoPP::Exception& e) {
		Output::Debug("EncryptMessage exception: {}", e.what());
	}

	std::string result;
	CryptoPP::Base64Encoder encoder(new CryptoPP::StringSink(result), false);
	encoder.Put(iv, sizeof(iv));
	encoder.Put((CryptoPP::byte*)cipher.c_str(), cipher.length());
	encoder.MessageEnd();

	return std::move(result);
}

std::string DecryptMessage(const std::string& password, const std::string& data, std::string& recovered) {
	CryptoPP::byte key[CryptoPP::AES::MAX_KEYLENGTH];
	CryptoPP::SHA256().CalculateDigest(key, (CryptoPP::byte*)password.c_str(), password.length());

	CryptoPP::byte iv[CryptoPP::AES::BLOCKSIZE];
	std::string cipher;

	CryptoPP::Base64Decoder decoder;
	decoder.Put((CryptoPP::byte*)data.data(), data.size());
	decoder.MessageEnd();

	CryptoPP::word64 size = decoder.MaxRetrievable();
	decoder.Get(iv, CryptoPP::AES::BLOCKSIZE);
	size_t cipher_size = size-CryptoPP::AES::BLOCKSIZE;
	cipher.resize(cipher_size);
	decoder.Get((CryptoPP::byte*)cipher.data(), cipher_size);

	std::string exception_what;
	try {
		CryptoPP::GCM<CryptoPP::AES>::Decryption decryptor;
		decryptor.SetKeyWithIV(key, CryptoPP::AES::MAX_KEYLENGTH, iv, CryptoPP::AES::BLOCKSIZE);

		CryptoPP::StringSource(cipher, true,
			new CryptoPP::AuthenticatedDecryptionFilter(decryptor,
				new CryptoPP::StringSink(recovered)
			)
		);
	} catch(const CryptoPP::Exception& e) {
		recovered = "";
		exception_what = e.what();
		Output::Debug("DecryptMessage exception: {}", exception_what);
	}

	return std::move(exception_what);
}

bool SetChatVisibility(std::string visibility_name) {
	auto it = Messages::VisibilityValues.find(visibility_name);
	if (it != Messages::VisibilityValues.end()) {
		chat_visibility = it->second;
		return true;
	}
	return false;
}

void SendKeyHash() {
	if (chat_visibility == Messages::CV_CRYPT) {
		std::string key = GMI().GetConfig().client_chat_crypt_key.Get();
		// send a hash integer to help the server to search for clients with the same key
		GMI().SendChatMessage(static_cast<int>(chat_visibility), "", StringToCRC32(key));
	}
}

void InitHello() {
	AddLogEntry("", "!! • IME input now supported!", "", Messages::CV_LOCAL);
	AddLogEntry("", "!!   (for Japanese, etc.)", "", Messages::CV_LOCAL);
	AddLogEntry("", "!! • You can now copy and", "", Messages::CV_LOCAL);
	AddLogEntry("", "!!   paste from type box.", "", Messages::CV_LOCAL);
	AddLogEntry("", "!! • SHIFT+[←, →] to select text.", "", Messages::CV_LOCAL);
	AddLogEntry("", "", "―――", Messages::CV_LOCAL);
	AddLogEntry("[F9]: ", "hide/show notifications.", "", Messages::CV_LOCAL);
	AddLogEntry("[F10]: ", "focus/unfocus.", "", Messages::CV_LOCAL);
	AddLogEntry("[↑, ↓]: ", "scroll.", "", Messages::CV_LOCAL);
	AddLogEntry("", "", "―――", Messages::CV_LOCAL);
	AddLogEntry("• Type !help to list commands.", "", "", Messages::CV_LOCAL);
}

void ShowUsage() {
	AddLogEntry("", "", "―――", Messages::CV_LOCAL);
	AddLogEntry("", "Usage:", "", Messages::CV_LOCAL);
	AddLogEntry("", "[...] Optional | <...> Required", "", Messages::CV_LOCAL);
	AddLogEntry("", "", "―――", Messages::CV_LOCAL);
	AddLogEntry("!server [on, off]", "", "", Messages::CV_LOCAL);
	AddLogEntry("- ", "turn on/off the server", "", Messages::CV_LOCAL);
	AddLogEntry("!connect [address]", "", "", Messages::CV_LOCAL);
	AddLogEntry("- ", "connect to the server", "", Messages::CV_LOCAL);
	AddLogEntry("!disconnect", "", "", Messages::CV_LOCAL);
	AddLogEntry("- ", "disconnect from server", "", Messages::CV_LOCAL);
	AddLogEntry("!name [text]", "", "", Messages::CV_LOCAL);
	AddLogEntry("- ", "change chat name", "", Messages::CV_LOCAL);
	AddLogEntry("!chat [LOCAL, GLOBAL, CRYPT] [CRYPT Password]", "", "", Messages::CV_LOCAL);
	AddLogEntry("- ", "switch visibility to chat", "", Messages::CV_LOCAL);
	AddLogEntry("!log [LOCAL, GLOBAL, CRYPT]", "", "", Messages::CV_LOCAL);
	AddLogEntry("- ", "toggle visibility", "", Messages::CV_LOCAL);
}

void Initialize() {
	chat_box = std::make_unique<DrawableChatUi>();
	InitHello();
	// initialize saved settings
	SetChatVisibility(GMI().GetConfig().client_chat_visibility.Get());
}

void SetFocus(bool focused) {
	Input::SetGameFocus(!focused);
	chat_box->SetFocus(focused);
}

void InputsFocusUnfocus() {
	if(Input::IsTriggered(Input::InputButton::KEY_F10)) {
		SetFocus(true);
	} else if(Input::IsExternalTriggered(Input::InputButton::KEY_F10) ||
			Input::IsExternalTriggered(Input::InputButton::KEY_ESCAPE)) {
		SetFocus(false);
	}
}

void InputsLog() {
	if(Input::IsExternalPressed(Input::InputButton::KEY_UP)) {
		chat_box->ScrollUp();
	}
	if(Input::IsExternalPressed(Input::InputButton::KEY_DOWN)) {
		chat_box->ScrollDown();
	}
	if(Input::IsTriggered(Input::InputButton::KEY_F9)) {
		chat_box->ToggleNotificationLog();
	}
}

void InputsTyping() {
	// input and paste
	std::string input_text = Input::GetExternalTextInput();
	if(Input::IsExternalTriggered(Input::InputButton::KEY_V) && Input::IsExternalPressed(Input::InputButton::KEY_CTRL))
		input_text = DisplayUi->GetClipboardText();
	if(input_text.size() > 0) {
		unsigned int caret_start = std::min<unsigned int>(type_caret_index_tail, type_caret_index_head);
		unsigned int caret_end = std::max<unsigned int>(type_caret_index_tail, type_caret_index_head);
		// erase selection
		type_text.erase(caret_start, caret_end-caret_start);
		std::u32string input_u32 = Utils::DecodeUTF32(input_text);
		std::u32string fits = input_u32.substr(0, MAXCHARSINPUT_MESSAGE-type_text.size());
		type_text.insert(caret_start, fits);
		type_caret_index_tail = type_caret_index_head = caret_start+fits.size();
	}

	// erase
	if(Input::IsExternalRepeated(Input::InputButton::KEY_BACKSPACE)) {
		unsigned int caret_start = std::min<unsigned int>(type_caret_index_tail, type_caret_index_head);
		unsigned int caret_end = std::max<unsigned int>(type_caret_index_tail, type_caret_index_head);
		auto len = std::max<unsigned int>(1, caret_end-caret_start);
		if (caret_start == caret_end) {
			if (caret_start == 0)
				return;
			type_caret_index_tail = type_caret_index_head = std::max<int>(0, caret_start-1);
		} else
			type_caret_index_tail = type_caret_index_head = caret_start;
		type_text.erase(type_caret_index_tail, len);
		chat_box->UpdateTypeScroll();
	}

	// copy
	if(Input::IsExternalTriggered(Input::InputButton::KEY_C) && Input::IsExternalPressed(Input::InputButton::KEY_CTRL)) {
		if(type_caret_index_tail != type_caret_index_head) {
			unsigned int caret_start = std::min<unsigned int>(type_caret_index_tail, type_caret_index_head);
			unsigned int caret_end = std::max<unsigned int>(type_caret_index_tail, type_caret_index_head);
			std::u32string selected = type_text.substr(caret_start, caret_end-caret_start);
			DisplayUi->SetClipboardText(Utils::EncodeUTF(selected));
		}
	}

	// move caret
	if(Input::IsExternalRepeated(Input::InputButton::KEY_LEFT)) {
		if(Input::IsExternalPressed(Input::InputButton::SHIFT)) {
			if(type_caret_index_head > 0) type_caret_index_head--;
		} else {
			if(type_caret_index_tail == type_caret_index_head) {
				if(type_caret_index_head > 0) type_caret_index_head--;
			} else {
				type_caret_index_head = std::min<unsigned int>(type_caret_index_tail, type_caret_index_head);
			}
			type_caret_index_tail = type_caret_index_head;
		}
	}
	if(Input::IsExternalRepeated(Input::InputButton::KEY_RIGHT)) {
		if(Input::IsExternalPressed(Input::InputButton::SHIFT)) {
			if(type_caret_index_head < type_text.size()) type_caret_index_head++;
		} else {
			if(type_caret_index_tail == type_caret_index_head) {
				if(type_caret_index_head < type_text.size()) type_caret_index_head++;
			} else {
				type_caret_index_head = std::max<unsigned int>(type_caret_index_tail, type_caret_index_head);
			}
			type_caret_index_tail = type_caret_index_head;
		}
	}

	// send
	if(Input::IsExternalTriggered(Input::InputButton::KEY_RETURN)) {
		std::string text = std::move(Utils::EncodeUTF(type_text));
		Strfnd fnd(text);
		std::string command = fnd.next(" ");
		// command: !server
		if (command == "!server" || command == "!srv") {
			std::string option = fnd.next(" ");
			if (option == "on") {
				Server().Start();
				AddClientInfo("Server: on");
			} else if (option == "off") {
				Server().Stop();
				AddClientInfo("Server: off");
			}
		// command: !connect
		} else if (command == "!connect" || command == "!c") {
			std::string address = fnd.next("");
			if (address != "")
				GMI().SetRemoteAddress(address);
			GMI().Connect();
		// command: !disconnect
		} else if (command == "!disconnect" || command == "!d") {
			GMI().Disconnect();
		// command: !name
		} else if (command == "!name") {
			std::string name = fnd.next(" ");
			if (name != "")
				GMI().SetChatName(name == "<unknown>" ? "" : name);
			name = GMI().GetChatName();
			AddClientInfo("Name: " + (name == "" ? "<unknown>" : name));
		// command: !chat
		} else if (command == "!chat") {
			std::string visibility_name = fnd.next(" ");
			if (visibility_name != "") {
				if (SetChatVisibility(visibility_name))
					GMI().GetConfig().client_chat_visibility.Set(visibility_name);
			}
			AddClientInfo("Visibility: " + \
				Messages::VisibilityNames.find(chat_visibility)->second);
			if (visibility_name == "CRYPT") {
				std::string chat_crypt_password = fnd.next(" ");
				if (chat_crypt_password != "") {
					AddClientInfo("CRYPT: Generating encryption key ...");
					std::thread([chat_crypt_password]() {
						GMI().GetConfig().client_chat_crypt_key.Set(GetPasswordHash(chat_crypt_password));
						SendKeyHash();
						AddClientInfo("CRYPT: Done");
					}).detach();
				}
			}
		// command: !log
		} else if (command == "!log") {
			auto it = Messages::VisibilityValues.find(fnd.next(""));
			if (it != Messages::VisibilityValues.end())
				chat_box->ToggleVisibilityFlag(it->second);
			std::string flags = "";
			for (const auto& it : Messages::VisibilityValues)
				if ((chat_box->GetVisibilityFlags() & it.second) > 0)
					flags += it.first + " ";
			if (flags.size() > 0)
				flags.erase(flags.size() - 1);
			AddClientInfo("Flags: " + flags);
		// command: !help
		} else if (command == "!help") {
			ShowUsage();
		} else {
			if (text != "") {
				if (chat_visibility == Messages::CV_CRYPT) {
					std::string key = GMI().GetConfig().client_chat_crypt_key.Get();
					GMI().SendChatMessage(static_cast<int>(chat_visibility),
						EncryptMessage(key, text));
				} else
					GMI().SendChatMessage(static_cast<int>(chat_visibility), text);
			}
		}
		// reset typebox
		type_text.clear();
		type_caret_index_tail = type_caret_index_head = 0;
	}

	// update type box
	chat_box->UpdateTypeText(type_text, type_caret_index_tail, type_caret_index_head);
	chat_box->UpdateTypeTextInputRect();
}

void ProcessInputs() {
	InputsFocusUnfocus();
	InputsLog();
	InputsTyping();
}

/**
 * External access
 */

static ChatUi _instance;

ChatUi& ChatUi::Instance() {
	return _instance;
}

void ChatUi::Refresh() {
	if(chat_box == nullptr) return;
	chat_box->RefreshTheme();
}

void ChatUi::Update() {
	if(chat_box == nullptr) {
		if (Player::debug_flag)
			return;
		for (int e = Scene::SceneType::Title; e < Scene::SceneType::Map; ++e) {
			if (Scene::Find(static_cast<Scene::SceneType>(e)) != nullptr)
				Initialize();
		}
	} else {
		ProcessInputs();
	}
}

void ChatUi::GotMessage(int visibility, int room_id,
		std::string name, std::string message, std::string sys_name) {
	if(chat_box == nullptr)
		return;
	VisibilityType v = static_cast<VisibilityType>(visibility);
	std::string vtext = "?";
	if (v == Messages::CV_CRYPT) {
		std::string decrypted_message;
		std::string key = GMI().GetConfig().client_chat_crypt_key.Get();
		std::string error = DecryptMessage(key, message, decrypted_message);
		if (error != "") {
			AddLogEntry("", error, "", Messages::CV_LOCAL);
		} else
			message = decrypted_message;
	}
	auto it = Messages::VisibilityNames.find(v);
	if (it != Messages::VisibilityNames.end())
		vtext = it->second;
	AddLogEntry("<", name, "> ", vtext, " #" + std::to_string(room_id),
			1, 0, 1, 2, 1, v, sys_name);
	AddLogEntry("\u00A0", message, "", "", "",
			0, -1, 0, 0, 0, v, "");
	AddNotificationLogEntry("<", name, "> ", message, "",
			1, 0, 1, -1, 0, v, sys_name);
	Output::Info("Chat: {} [{}, {}]: {}", name, vtext, room_id, message);
}

void ChatUi::GotInfo(std::string message) {
	if(chat_box == nullptr)
		return;
	AddLogEntry("", message, "", Messages::CV_LOCAL);
	AddNotificationLogEntry("", message, "", Messages::CV_LOCAL);
}

void ChatUi::SetStatusConnection(bool connected, bool connecting) {
	if(chat_box == nullptr)
		return;
	chat_box->SetStatusConnection(connected, connecting);
	if (connected)
		SendKeyHash();
}

void ChatUi::SetStatusRoom(unsigned int room_id) {
	if(chat_box == nullptr)
		return;
	chat_box->SetStatusRoom(room_id);
}
