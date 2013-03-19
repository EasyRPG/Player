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

#ifndef _BOT_UI_H_
#define _BOT_UI_H_

// Headers
#include "system.h"
#include "baseui.h"
#include "audio.h"

struct BotInterface;

struct BotUi : public BaseUi {
	BotUi(EASYRPG_SHARED_PTR<BotInterface> const& inf);

	void BeginDisplayModeChange() {}
	void EndDisplayModeChange() {}
	void Resize(long, long) {}
	void ToggleFullscreen() {}
	void ToggleZoom() {}
	void UpdateDisplay() {}
	void BeginScreenCapture() {}
	void SetTitle(const std::string&) {}
	void DrawScreenText(const std::string&) {}
	void DrawScreenText(const std::string&, int, int, Color const&) {}
	void DrawScreenText(const std::string&, Rect const&, Color const&) {}
	bool ShowCursor(bool) { return false; }
	bool IsFullscreen() { return false; }

	BitmapRef EndScreenCapture();
	void ProcessEvents();

	uint32_t GetTicks() const;
	void Sleep(uint32_t time_milli);

	AudioInterface& GetAudio();

 private:
	EASYRPG_SHARED_PTR<BotInterface> bot_;
	size_t counter_;
	EmptyAudio audio_;
}; // struct BotUi

#endif // _BOT_UI_H_
