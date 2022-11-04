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

#include <string>
#include <algorithm>

#include "window_keyboard.h"
#include "game_system.h"
#include "input.h"
#include "bitmap.h"
#include "font.h"

const char* const Window_Keyboard::DONE = "<Done>";
const char* const Window_Keyboard::SPACE = "SPACE";
const char* const Window_Keyboard::NEXT_PAGE = "<Page>";
const char* const Window_Keyboard::DONE_JP = "<決定>";
const char* const Window_Keyboard::DONE_RU = "<OK>";
const char* const Window_Keyboard::DONE_KO = "<결정>";
const char* const Window_Keyboard::DONE_ZH_CN = "<确定>";
const char* const Window_Keyboard::DONE_ZH_TW = "<確定>";

/*
 * Hiragana <-> Katakana; Hangul 1 <-> Hangul 2; Simp. Chinese 1 <-> Simp. Chinese 2;
 * Trad. Chinese 1 <-> Trad. Chinese 2; Rus.Cyrillic <-> Rus.Latin; letter <-> symbol
 */

Keyboard_Layout Window_Keyboard::layouts[Window_Keyboard::MODE_END] = {
	{
		"<かな>",
		{ // Hiragana
			{"あ", "い", "う", "え", "お", "が", "ぎ", "ぐ", "げ", "ご"},
			{"か", "き", "く", "け", "こ", "ざ", "じ", "ず", "ぜ", "ぞ"},
			{"さ", "し", "す", "せ", "そ", "だ", "ぢ", "づ", "で", "ど"},
			{"た", "ち", "つ", "て", "と", "ば", "び", "ぶ", "べ", "ぼ"},
			{"な", "に", "ぬ", "ね", "の", "ぱ", "ぴ", "ぷ", "ぺ", "ぽ"},
			{"は", "ひ", "ふ", "へ", "ほ", "ぁ", "ぃ", "ぅ", "ぇ", "ぉ"},
			{"ま", "み", "む", "め", "も", "っ", "ゃ", "ゅ", "ょ", "ゎ"},
			{"や", "ゆ", "よ", "わ", "ん", "ー", "～", "・", "＝", "☆"},
			{"ら", "り", "る", "れ", "ろ", "ヴ", NEXT_PAGE, "", DONE}
		}
	},
	{
		"<カナ>",
		{ // Katakana
			{"ア", "イ", "ウ", "エ", "オ", "ガ", "ギ", "グ", "ゲ", "ゴ"},
			{"カ", "キ", "ク", "ケ", "コ", "ザ", "ジ", "ズ", "ゼ", "ゾ"},
			{"サ", "シ", "ス", "セ", "ソ", "ダ", "ヂ", "ヅ", "デ", "ド"},
			{"タ", "チ", "ツ", "テ", "ト", "バ", "ビ", "ブ", "ベ", "ボ"},
			{"ナ", "ニ", "ヌ", "ネ", "ノ", "パ", "ピ", "プ", "ペ", "ポ"},
			{"ハ", "ヒ", "フ", "ヘ", "ホ", "ァ", "ィ", "ゥ", "ェ", "ォ"},
			{"マ", "ミ", "ム", "メ", "モ", "ッ", "ャ", "ュ", "ョ", "ヮ"},
			{"ヤ", "ユ", "ヨ", "ワ", "ン", "ー", "～", "・", "＝", "☆"},
			{"ラ", "リ", "ル", "レ", "ロ", "ヴ", NEXT_PAGE, "", DONE}
		}
	},
	{
		"<앞P>",
		{ // Hangul 1
			{"가", "갸", "거", "겨", "고", "교", "구", "계", "그", "기"},
			{"나", "냐", "너", "녀", "노", "뇨", "누", "뉴", "느", "녹"},
			{"다", "댜", "더", "뎌", "도", "됴", "두", "듀", "드", "디"},
			{"라", "랴", "러", "려", "로", "료", "루", "류", "르", "리"},
			{"마", "먀", "머", "며", "모", "묘", "무", "물", "므", "미"},
			{"바", "뱌", "버", "벼", "보", "뵤", "부", "뷰", "비", "밤"},
			{"사", "색", "서", "세", "소", "쇼", "수", "슈", "신", "심"},
			{"아", "야", "어", "여", "오", "요", "우", "유", "으", "이"},
			{"〜", "·", ".", "☆", SPACE, "", NEXT_PAGE, "", DONE}
		}

	},
	{
		"<뒤P>",
		{ // Hangul 2
			{"자", "쟈", "저", "져", "조", "죠", "주", "쥬", "즈", "지"},
			{"차", "챠", "처", "쳐", "초", "쵸", "추", "츄", "츠", "치"},
			{"카", "캬", "커", "켜", "코", "쿄", "쿠", "큐", "크", "키"},
			{"타", "탸", "터", "텨", "토", "툐", "투", "튜", "트", "티"},
			{"파", "퍄", "퍼", "펴", "포", "표", "푸", "퓨", "프", "피"},
			{"하", "햐", "허", "혀", "호", "효", "후", "휴", "흐", "해"},
			{"1", "2", "3", "4", "5", "6", "7", "8", "9", "0"},
			{"진", "녘", "의", "민", "예", "건", "현", "운", "걔", "임"},
			{"영", "은", "성", "준", SPACE, "", NEXT_PAGE, "", DONE}
		},
	},
	{
		"<翻页>",
		{ // Simp. Chinese 1
			{"阿", "艾", "安", "奥", "巴", "拜", "班", "邦", "贝", "本"},
			{"比", "宾", "波", "伯", "布", "查", "达", "丹", "当", "道"},
			{"德", "登", "迪", "蒂", "丁", "度", "杜", "顿", "多", "厄"},
			{"尔", "恩", "法", "凡", "菲", "费", "芬", "佛", "弗", "夫"},
			{"盖", "格", "戈", "冈", "古", "哈", "海", "汉", "豪", "赫"},
			{"华", "霍", "基", "吉", "加", "杰", "捷", "金", "卡", "凯"},
			{"科", "克", "肯", "拉", "莱", "兰", "朗", "劳", "勒", "雷"},
			{"里", "利", "立", "丽", "莉", "林", "琳", "留", "隆", "鲁"},
			{"路", "伦", "罗", "洛", "律", "", NEXT_PAGE, "", DONE}
		}
	},
	{
		"<前页>",
		{ // Simp. Chinese 2
			{"玛", "迈", "曼", "梅", "美", "门", "米", "密", "明", "缪"},
			{"摩", "莫", "姆", "穆", "那", "娜", "纳", "奈", "南", "尼"},
			{"宁", "纽", "奴", "诺", "欧", "帕", "派", "佩", "皮", "普"},
			{"奇", "琪", "琼", "丘", "萨", "撒", "赛", "桑", "瑟", "森"},
			{"沙", "山", "珊", "史", "世", "斯", "丝", "司", "苏", "所"},
			{"索", "塔", "泰", "坦", "汤", "特", "提", "汀", "统", "瓦"},
			{"威", "维", "韦", "卫", "温", "沃", "乌", "西", "希", "夏"},
			{"辛", "修", "休", "雅", "亚", "林", "琳", "留", "隆", "鲁"},
			{"伊", "英", "尤", "则", "扎", "", NEXT_PAGE, "", DONE}
		}
	},
	{
		"<翻頁>",
		{ // Trad. Chinese 1
			{"泉", "聲", "咽", "危", "石", "日", "色", "冷", "青", "松"},
			{"薄", "暮", "空", "潭", "曲", "安", "禪", "制", "毒", "龍"},
			{"海", "內", "存", "知", "己", "天", "涯", "若", "比", "鄰"},
			{"滿", "階", "芳", "草", "綠", "一", "片", "杏", "花", "香"},
			{"恨", "無", "千", "日", "酒", "空", "斷", "九", "迴", "腸"},
			{"當", "路", "誰", "相", "假", "知", "音", "世", "所", "稀"},
			{"只", "應", "守", "寂", "寞", "還", "掩", "故", "園", "扉"},
			{"坐", "觀", "垂", "釣", "者", "－", "～", "’", "＝", "☆"},
			{"徒", "有", "羨", "魚", "情", "的", NEXT_PAGE, "", DONE}
		}
	},
	{
		"<前頁>",
		{ // Trad. Chinese 2
			{"幾", "行", "歸", "塞", "盡", "念", "爾", "獨", "何", "之"},
			{"暮", "雨", "相", "呼", "失", "寒", "塘", "欲", "下", "遲"},
			{"霞", "光", "連", "碧", "彩", "湖", "岸", "水", "連", "灘"},
			{"天", "地", "丹", "青", "繪", "人", "間", "錦", "秀", "看"},
			{"返", "景", "入", "深", "林", "復", "照", "青", "苔", "上"},
			{"中", "歲", "頗", "好", "道", "晚", "家", "南", "山", "睡"},
			{"興", "來", "每", "獨", "往", "勝", "事", "空", "自", "知"},
			{"遙", "夜", "泛", "清", "瑟", "－", "～", "’", "＝", "☆"},
			{"西", "風", "生", "翠", "蘿", "的", NEXT_PAGE, "", DONE}
		}
	},
	{
		"<Абвг>",
		{ // Cp1251 Russian Cyrillic (+ Bel. and Ukr. letters in the last row)
			{"А", "Б", "В", "Г", "Д", "а", "б", "в", "г", "д"},
			{"Е", "Ё", "Ж", "З", "И", "е", "ё", "ж", "з", "и"},
			{"Й", "К", "Л", "М", "Н", "й", "к", "л", "м", "н"},
			{"О", "П", "Р", "С", "Т", "о", "п", "р", "с", "т"},
			{"У", "Ф", "Х", "Ц", "Ч", "у", "ф", "х", "ц", "ч"},
			{"Ш", "Щ", "Ъ", "Ы", "Ь", "ш", "щ", "ъ", "ы", "ь"},
			{"Э", "Ю", "Я",  "",  "", "э", "ю", "я",  "",  ""},
			{"Ґ", "Є", "І", "Ї", "Ў", "ґ", "є", "і", "ї", "ў"},
			{"ʼ",  "",  "",  "",  "",  "", NEXT_PAGE, "", DONE}
		}
	},
	{
		"<Abcd>",
		{ // Letter
			{"A", "B", "C", "D", "E", "a", "b", "c", "d", "e"},
			{"F", "G", "H", "I", "J", "f", "g", "h", "i", "j"},
			{"K", "L", "M", "N", "O", "k", "l", "m", "n", "o"},
			{"P", "Q", "R", "S", "T", "p", "q", "r", "s", "t"},
			{"U", "V", "W", "X", "Y", "u", "v", "w", "x", "y"},
			{"Z", "" , "" , "" , "" , "z"},
			{"0", "1", "2", "3", "4", "5", "6", "7", "8", "9"},
			{SPACE},
			{"" , "" , "" , "" , "" , "" , NEXT_PAGE, "", DONE}
		}
	},
	{
		"<$A$B>",
		{ // Symbol
			{"$A", "$B", "$C", "$D", "$E", "$a", "$b", "$c", "$d", "$e"},
			{"$F", "$G", "$H", "$I", "$J", "$f", "$g", "$h", "$i", "$j"},
			{"$K", "$L", "$M", "$N", "$O", "$k", "$l", "$m", "$n", "$o"},
			{"$P", "$Q", "$R", "$S", "$T", "$p", "$q", "$r", "$s", "$t"},
			{"$U", "$V", "$W", "$X", "$Y", "$u", "$v", "$w", "$x", "$y"},
			{"$Z",  "" ,  "" ,  "" ,  "" , "$z"},
			{},
			{},
			{ "" ,  "" ,  "" ,  "" ,  "" ,  "" , NEXT_PAGE, "", DONE}
		}
	}
};

Window_Keyboard::Window_Keyboard(int ix, int iy, int iwidth, int iheight, const char* ndone_text)
		: Window_Base(ix, iy, iwidth, iheight)
		, done_text(ndone_text)
		, play_cursor(false)
{
	row = 0;
	col = 0;

	SetContents(Bitmap::Create(width - 16, height - 16));

	row_spacing = 16;
	col_spacing = (contents->GetWidth() - 2 * border_x) / col_max;

	mode = Letter;
	next_mode = Symbol;

	Refresh();
	UpdateCursorRect();
}

void Window_Keyboard::SetMode(Window_Keyboard::Mode nmode, Window_Keyboard::Mode nnext_mode) {
	mode = nmode;
	next_mode = nnext_mode;
	Refresh();
	UpdateCursorRect();
}

std::string const& Window_Keyboard::GetKey(int row, int col) const {
	std::string const& str = layouts[mode].items[row][col];
	if (str == NEXT_PAGE) {
		return layouts[next_mode].key_text;
	}
	else if (str == DONE) {
		return done_text;
	}
	else {
		return str;
	}
}

std::string const& Window_Keyboard::GetSelected() const {
	return layouts[mode].items[row][col];
}

Rect Window_Keyboard::GetItemRect(int row, int col) const {
	return Rect(col * col_spacing + border_x,
				row * row_spacing + border_y,
				Text::GetSize(*Font::Default(), GetKey(row, col)).width + 8,
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
			contents->TextDraw(r.x + 4, r.y, Font::ColorDefault, GetKey(j, i));
		}
	}
}

void Window_Keyboard::Update() {
	Window_Base::Update();

	// move left on wide fields
	int skip_dir = -1;

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
			col = (col + 1) % col_max;
			skip_dir = 1;
		}
		if (Input::IsRepeated(Input::LEFT)) {
			play_cursor = true;
			col = (col + col_max - 1) % col_max;
		}
	}

	// Special handling for wide fields
	if (col > 0) {
		// page switch and done are always in the bottom right corner
		if ((row == row_max - 1 && (col == col_max - 3 || col == col_max - 1))
			|| GetKey(row, col - 1) == SPACE)
			col = std::min(col + skip_dir, col_max - 1);
	}

	// Skip empty cells
	if (GetSelected().empty()) {
		Update();
		return;
	}

	if (play_cursor) {
		Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cursor));
		play_cursor = false;
	}
	UpdateCursorRect();
}
