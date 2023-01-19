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
#include <cstdint>
#include <string>
#include <vector>
#include <bitset>
#include <functional>
#include "async_op.h"

class PendingMessage {
	public:
		using ChoiceContinuation = std::function<AsyncOp(int)>;

		int PushLine(std::string msg);
		int PushChoice(std::string msg, bool enabled = true);
		int PushNumInput(int variable_id, int num_digits);
		void PushPageEnd();

		void SetWordWrapped(bool value);
		void SetChoiceCancelType(int value);
		void SetChoiceResetColors(bool value);
		void SetShowGoldWindow(bool value) { show_gold_window = value; }
		void SetChoiceContinuation(ChoiceContinuation f) { choice_continuation = std::move(f); }

		const std::vector<std::string>& GetLines() const { return texts; }

		bool IsActive() const { return NumLines() || HasNumberInput(); }
		int NumLines() const { return texts.size(); }
		bool IsWordWrapped() const { return word_wrapped; }
		bool ShowGoldWindow() const { return show_gold_window; }

		bool HasChoices() const { return choice_start >= 0; }
		int GetChoiceStartLine() const { return choice_start; }
		int GetNumChoices() const { return HasChoices() ? NumLines() - choice_start : 0; }
		int GetChoiceCancelType() const { return choice_cancel_type; }
		bool IsChoiceEnabled(int idx) const { return choice_enabled[idx]; }
		bool GetChoiceResetColor() const { return choice_reset_color; }
		const ChoiceContinuation& GetChoiceContinuation() const { return choice_continuation; }

		bool HasNumberInput() const { return num_input_digits > 0; }
		int GetNumberInputDigits() const { return num_input_digits; }
		int GetNumberInputVariable() const { return num_input_variable; }
		int GetNumberInputStartLine() const { return NumLines(); }

		void SetEnableFace(bool value) { enable_face = value; }
		bool IsFaceEnabled() const { return enable_face; }

		void SetIsEventMessage(bool value) { is_event_message = value; }
		bool IsEventMessage() const { return is_event_message; }
	private:
		int PushLineImpl(std::string msg);

		std::string ApplyTextInsertingCommands(std::string input, uint32_t escape_char);

	private:
		ChoiceContinuation choice_continuation;
		std::vector<std::string> texts;
		int choice_start = -1;
		int choice_cancel_type = 5;
		int num_input_variable = 0;
		int num_input_digits = 0;
		std::bitset<8> choice_enabled = {};
		bool word_wrapped = false;
		bool choice_reset_color = false;
		bool show_gold_window = false;
		bool enable_face = true;
		bool is_event_message = false;
};



#endif


