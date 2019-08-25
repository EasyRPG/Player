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

#ifndef EP_PENDING_MESSAGE_H
#define EP_PENDING_MESSAGE_H
#include <string>
#include <vector>
#include <bitset>

class PendingMessage {
	public:
		int PushLine(std::string msg);
		int PushChoice(std::string msg, bool enabled = true);
		int PushNumInput(int variable_id, int num_digits);
		void PushPageEnd();

		void SetWordWrapped(bool value);
		void SetChoiceCancelType(int value);
		void SetChoiceResetColors(bool value);

		const std::vector<std::string>& GetLines() const { return texts; }

		int NumLines() const { return texts.size(); }
		bool IsWordWrapped() const { return word_wrapped; }

		bool HasChoices() const { return choice_start >= 0; }
		int GetChoiceStartLine() const { return choice_start; }
		int GetNumChoices() const { return HasChoices() ? NumLines() - choice_start : 0; }
		int GetChoiceCancelType() const { return choice_cancel_type; }
		bool IsChoiceEnabled(int idx) const { return choice_enabled[idx]; }
		bool GetChoiceResetColor() const { return choice_reset_color; }

		bool HasNumberInput() const { return num_input_digits > 0; }
		int GetNumberInputDigits() const { return num_input_digits; }
		int GetNumberInputVariable() const { return num_input_variable; }
		int GetNumberInputStartLine() const { return NumLines(); }
	private:
		std::vector<std::string> texts;
		int choice_start = -1;
		int choice_cancel_type = 5;
		int num_input_variable = 0;
		int num_input_digits = 0;
		std::bitset<8> choice_enabled = {};
		bool word_wrapped = false;
		bool choice_reset_color = false;
};



#endif


