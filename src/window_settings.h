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

#ifndef EP_WINDOW_SETTINGS_H
#define EP_WINDOW_SETTINGS_H

// Headers
#include <vector>
#include "input.h"
#include "window_numberinput.h"
#include "window_selectable.h"

/**
 * Window_Settings class.
 */
class Window_Settings : public Window_Selectable {
public:
	enum UiMode {
		eNone,
		eMain,
		eInput,
		eInputButtonCategory,
		eInputListButtonsGame,
		eInputListButtonsEngine,
		eInputListButtonsDeveloper,
		eInputButtonOption,
		eInputButtonAdd,
		eInputButtonRemove,
		eVideo,
		eAudio,
		eAudioMidi,
		eAudioSoundfont,
		eLicense,
		eEngine,
		eEngineFont1,
		eEngineFont2,
		eSave,
		eEnd,
		eAbout,
		eLanguage,
		eLastMode
	};

	enum OptionMode {
		eOptionNone,
		eOptionRangeInput,
		eOptionPicker
	};

	struct Option {
		std::string text;
		std::string value_text;
		std::string help;
		std::string help2;
		std::function<void(void)> action;
		OptionMode mode;
		Font::SystemColor color = Font::ColorDefault;
		int current_value;
		int original_value;
		int min_value;
		int max_value;
		std::vector<int> options_index;
		std::vector<std::string> options_text;
		std::vector<std::string> options_help;
	};

	struct StackFrame {
		UiMode uimode = eNone;
		int arg = -1;
		int scratch = 0;
		int scratch2 = 0;
		std::vector<Option> options;
	};

	/** Constructor  */
	Window_Settings(int ix, int iy, int iwidth, int iheight);

	/** @return true if the index points to an enabled action */
	bool IsCurrentActionEnabled() const {
		return (index >= 0
				&& index < static_cast<int>(GetFrame().options.size())
				&& static_cast<bool>(GetFrame().options[index].action));
	}

	/** Execute the action pointed to by index */
	Option& GetCurrentOption() {
		return GetFrame().options[index];
	}

	UiMode GetMode() const;

	void Push(UiMode ui, int arg = -1);
	void Pop();

	StackFrame& GetFrame(int n = 0);
	const StackFrame& GetFrame(int n = 0) const;

	/**
	 * Refreshes the item list.
	 */
	void Refresh();

	/**
	 * Draws an item.
	 *
	 * @param index index of item to draw.
	 */
	void DrawOption(int index);

	Window_Help* help_window2 = nullptr;
	RangeConfigParam<int> font_size { "<Font Size>", "Font size to use. Not supported for the built-in font.", "", "", 12, 6, 16};
private:

	template <typename Param, typename Action>
	void AddOption(const Param& p,
			Action&& action);

	template <typename T, typename Action>
	void AddOption(const RangeConfigParam<T>& p,
			Action&& action
	);

	template <typename T, typename Action, size_t S>
	void AddOption(const EnumConfigParam<T, S>& p,
			Action&& action
	);

	void RefreshInput();
	void RefreshButtonCategory();
	void RefreshButtonList();
	void RefreshVideo();
	void RefreshAudio();
	void RefreshAudioMidi();
	void RefreshAudioSoundfont();
	void RefreshEngine();
	void RefreshEngineFont(bool mincho);
	void RefreshLicense();

	void UpdateHelp() override;

	struct Memory {
		int index = 0;
		int top_row = 0;
	};
	Memory memory[eLastMode] = {};

	std::array<StackFrame,8> stack;
	int stack_index = 0;
	std::vector<std::string> picker_options;

	void SavePosition();
	void RestorePosition();

	// The sample text for the font preview. This option is not saved.
	enum class SampleText {
		English,
		French,
		German,
		Spanish,
		Chinese,
		Japanese_Hiragana,
		Japanese_Kanji,
		Korean,
		Diacritics
	};

	EnumConfigParam<SampleText, 9> sample_text{
		"<Sample Text>", "Text to show in the preview", "", "", SampleText::English,
		Utils::MakeSvArray("English", "French", "German", "Spanish", "Chinese", "Japanese (Hiragana)", "Japanese (Kanji)", "Korean", "Various Diacritics"),
		Utils::MakeSvArray("", "", "", "", "", "", "", "", ""),
		Utils::MakeSvArray(
			"The quick brown fox jumps over the lazy dog 1234567890.?!",
			"Voix ambiguë d'un cœur qui au zéphyr préfère les jattes de kiwis",
			"Victor jagt zwölf Boxkämpfer quer über den großen Sylter Deich",
			"Jovencillo emponzoñado de whisky, ¡qué figurota exhibe!",
			"天地玄黃宇宙洪荒。日月盈昃辰宿列張。寒來暑往秋收冬藏。閏餘成歲律呂調陽。雲騰致雨露結為霜。金生麗水玉出崑岡。劍號巨闕珠稱夜光。果珍李柰菜重芥薑。海鹹河淡鱗潛羽翔。",
			"いろはにほへと ちりぬるを わかよたれそ つねならむ うゐのおくやま けふこえて あさきゆめみし ゑひもせす",
			"色は匂えど散りぬるを我が世誰ぞ常ならん有為の奥山今日越えて浅き夢見じ酔いもせず",
			"키스의 고유조건은 입술끼리 만나야 하고 특별한 기술은 필요치 않다",
			"ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõöøùúûüýþÿ"
		)
	};
};

#endif
