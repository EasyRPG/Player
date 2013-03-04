/////////////////////////////////////////////////////////////////////////////
// This file is part of EasyRPG Player.
//
// EasyRPG Player is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// EasyRPG Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <algorithm>
#include <string>
#include "window_keyboard.h"
#include "game_system.h"
#include "input.h"
#include "bitmap.h"
#include "font.h"

#include <boost/regex/pending/unicode_iterator.hpp>


const char* const Window_Keyboard::TO_SYMBOL = "Symbol";
const char* const Window_Keyboard::TO_LETTER = "Letter";
const char* const Window_Keyboard::DONE = "Done";

std::string Window_Keyboard::TO_KATAKANA;
std::string Window_Keyboard::TO_HIRAGANA;
std::string Window_Keyboard::DONE_JP;

namespace {
	void write_chars(std::string& out, unsigned const* in_beg, unsigned const* in_end) {
		typedef boost::u32_to_u8_iterator<unsigned const*> iterator;
		out.resize(std::distance(iterator(in_beg), iterator(in_end)));
		std::copy(iterator(in_beg), iterator(in_end), out.begin());
	}
	void write_char(std::string& out, unsigned const in) {
		write_chars(out, &in, &in + 1);
	}

	void write_range(std::string (&k)[9][10], unsigned num,
					 unsigned y, unsigned x, unsigned base, unsigned diff) {
		for(unsigned i = 0; i < num; ++i) { write_char(k[y][x + i], base + diff*i); }
	}

	void write_kana(std::string (&k)[9][10], unsigned const base) {
		// left half
		write_range(k, 5, 0, 0, base + 0x02, 2); //  a -  o
		write_range(k, 5, 1, 0, base + 0x0B, 2); // ka - ko
		write_range(k, 5, 2, 0, base + 0x15, 2); // sa - so
		write_range(k, 2, 3, 0, base + 0x1F, 2); // ta - ti
		write_range(k, 3, 3, 0 + 2, base + 0x24, 2); // tu - to
		write_range(k, 5, 4, 0, base + 0x2A, 1); // na - no
		write_range(k, 5, 5, 0, base + 0x2F, 3); // ha - ho
		write_range(k, 5, 6, 0, base + 0x3E, 1); // ma - mo
		write_range(k, 3, 7, 0, base + 0x44, 2); // ya - yo
		write_char(k[7][3], base + 0x4F); // wa
		write_char(k[7][4], base + 0x53); // nn
		write_range(k, 5, 8, 0, base + 0x49, 1); // ra - ro

		// right half
		write_range(k, 5, 0, 5, base + 0x0C, 2); // ga - go
		write_range(k, 5, 1, 5, base + 0x16, 2); // za - zo
		write_range(k, 2, 2, 5, base + 0x20, 2); // da - di
		write_range(k, 3, 2, 5 + 2, base + 0x25, 2); // du - do
		write_range(k, 5, 3, 5, base + 0x30, 3); // ba - bo
		write_range(k, 5, 4, 5, base + 0x31, 3); // pa - po
		write_range(k, 5, 5, 5, base + 0x01, 2); // small a - o
		// small other
		write_char(k[6][5], base + 0x23); // small tu
		write_range(k, 3, 6, 5 + 1, base + 0x43, 2); // small ya - yo
		write_char(k[6][9], base + 0x4E); // small wa
		// Symbol
		write_char(k[7][5], 0x30FC); // cho-on
		write_char(k[7][6], 0x301C); // wave dash
		write_char(k[7][7], 0x30FB); // dot
		write_char(k[7][8], 0xFF1D); // equal
		write_char(k[7][9], 0x2606); // star
		write_char(k[8][5], 0x30F4); // va
	}
} // anonymous namespace

/*
 * hiragana -> katakana -> letter -> symbol -> hiragana -> ...
 */

////////////////////////////////////////////////////////////
std::string Window_Keyboard::items[Window_Keyboard::MODE_END][9][10] = {
	{}, // Hiragana

	{ // Katakana
	{},{},{},{},{},{},{},{},
	{"","","","","","",Window_Keyboard::TO_LETTER,},
	},

	{ // Letter
	{"A","B","C","D","E","a","b","c","d","e"},
	{"F","G","H","I","J","f","g","h","i","j"},
	{"K","L","M","N","O","k","l","m","n","o"},
	{"P","Q","R","S","T","p","q","r","s","t"},
	{"U","V","W","X","Y","u","v","w","x","y"},
	{"Z","" ,"" ,"" ,"" ,"z",},
	{"0","1","2","3","4","5","6","7","8","9"},
	{},
	{"","","","","","",Window_Keyboard::TO_SYMBOL,"",Window_Keyboard::DONE},
	},

	{ // Symbol
	{"$A","$B","$C","$D","$E","$a","$b","$c","$d","$e"},
	{"$F","$G","$H","$I","$J","$f","$g","$h","$i","$j"},
	{"$K","$L","$M","$N","$O","$k","$l","$m","$n","$o"},
	{"$P","$Q","$R","$S","$T","$p","$q","$r","$s","$t"},
	{"$U","$V","$W","$X","$Y","$u","$v","$w","$x","$y"},
	{"$Z",""  ,""  ,""  ,""  ,"$z"},
	{},
	{},
	{"","","","","","","","",Window_Keyboard::DONE},
	},
};

////////////////////////////////////////////////////////////
Window_Keyboard::Window_Keyboard(int ix, int iy, int iwidth, int iheight)
		: Window_Base(ix, iy, iwidth, iheight)
		, play_cursor(false)
{
	row = 0;
	col = 0;

	SetContents(Bitmap::Create(width - 16, height - 16));
	contents->SetTransparentColor(windowskin->GetTransparentColor());
	SetZ(9999);

	row_spacing = 16;
	col_spacing = (contents->GetWidth() - 2 * border_x) / col_max;

	mode = Letter;

	Refresh();
	UpdateCursorRect();

	if(items[Hiragana][0][0].empty()) {
		write_kana(items[Hiragana], 0x3040);
		write_kana(items[Katakana], 0x30A0);

		unsigned const to_katakana[] = {0x3C, 0x30AB, 0x30CA, 0x3E};
		unsigned const to_hiragana[] = {0x3C, 0x304B, 0x306A, 0x3E};
		unsigned const done_jp[] = {0x3C, 0x6C7A, 0x5B9A, 0x3E};
		write_chars(TO_KATAKANA, to_katakana, to_katakana + 4);
		write_chars(TO_HIRAGANA, to_hiragana, to_hiragana + 4);
		write_chars(DONE_JP, done_jp, done_jp + 4);

		items[Hiragana][8][8] = items[Katakana][8][8] = DONE_JP;
		items[Symbol][8][6] = TO_HIRAGANA;
		items[Hiragana][8][6] = TO_KATAKANA;
	}
}

void Window_Keyboard::SetMode(Window_Keyboard::Mode nmode) {
	mode = nmode;
	Refresh();
	UpdateCursorRect();
}

std::string const& Window_Keyboard::GetSelected(void) {
	return items[mode][row][col];
}

Rect Window_Keyboard::GetItemRect(int row, int col) const {
	std::string const& str = items[mode][row][col];
	return Rect(col * col_spacing + border_x,
				row * row_spacing + border_y,
				contents->GetFont()->GetSize(str).width + 8,
				row_spacing);
}

void Window_Keyboard::UpdateCursorRect() {
	Rect r = GetItemRect(row, col);
	r.y -= 2;
	SetCursorRect(r);
}

void Window_Keyboard::Refresh() {
	contents->Clear();

	for (int j = 0; j < row_max; j++) {
		for (int i = 0; i < col_max; i++) {
			Rect r = GetItemRect(j, i);
			contents->TextDraw(r.x + 4, r.y, Font::ColorDefault, items[mode][j][i]);
		}
	}
}

void Window_Keyboard::Update() {
	Window_Base::Update();

	if (active) {
		if (Input::IsRepeated(Input::DOWN)) {
			play_cursor = true;
			row = (row + 1) % row_max;
		}
		if (Input::IsRepeated(Input::UP)) {
			play_cursor = true;
			row = (row + row_max - 1) % row_max;
		}
		if (Input::IsRepeated(Input::RIGHT)) {
			play_cursor = true;
			col += 1;
			if (col >= col_max) {
				col = 0;
				if(mode == Letter) { row = (row + 1) % row_max; }
			}
		}
		if (Input::IsRepeated(Input::LEFT)) {
			play_cursor = true;
			col -= 1;
			if (col < 0) {
				col = col_max - 1;
				if(mode == Letter) { row = (row + row_max - 1) % row_max; }
			}
		}

	}

	if(GetSelected().empty()) {
		Update();
		return;
	}

	if(play_cursor) {
		Game_System::SePlay(Data::system.cursor_se);
		play_cursor = false;
	}
	UpdateCursorRect();
}
