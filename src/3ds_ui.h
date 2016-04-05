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

#ifndef _SDL_UI_H_
#define _SDL_UI_H_

// Headers
#include "baseui.h"
#include "color.h"
#include "rect.h"
#include "system.h"
#include <sf2d.h>

#include <boost/scoped_ptr.hpp>

/**
 * SdlUi class.
 */
class CtrUi : public BaseUi {
public:
	/**
	 * Constructor.
	 *
	 * @param width window client width.
	 * @param height window client height.
	 * @param title window title.
	 * @param fullscreen start in fullscreen flag.
	 */
	CtrUi(int width, int height);

	/**
	 * Destructor.
	 */
	~CtrUi();

	/**
	 * Inherited from BaseUi.
	 */
	/** @{ */

	void BeginDisplayModeChange();
	void EndDisplayModeChange();
	void Resize(long width, long height);
	void ToggleFullscreen();
	void ToggleZoom();
	void UpdateDisplay();
	void BeginScreenCapture();
	BitmapRef EndScreenCapture();
	void SetTitle(const std::string &title);
	bool ShowCursor(bool flag);

	void ProcessEvents();

	bool IsFullscreen();

	uint32_t GetTicks() const;
	void Sleep(uint32_t time_milli);
#ifdef SUPPORT_AUDIO
	AudioInterface& GetAudio();
	boost::scoped_ptr<AudioInterface> audio_;
#endif

	/** @} */
	sf2d_texture* main_texture;
	sf2d_texture* keyboard_texture;
	int frame;
	bool fullscreen;
	bool trigger_state;
	
};

#endif
