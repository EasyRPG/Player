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

#ifndef EP_PLATFORM_3DS_UI_H
#define EP_PLATFORM_3DS_UI_H

#ifdef _3DS

// Headers
#include "baseui.h"
#include "color.h"
#include "rect.h"
#include "system.h"
#include <citro3d.h>
#include <citro2d.h>

/**
 * CtrUi class.
 */
class CtrUi : public BaseUi {
public:
	/**
	 * Constructor.
	 *
	 * @param width window client width.
	 * @param height window client height.
	 * @param cfg video config options
	 */
	CtrUi(int width, int height, const Game_ConfigVideo& cfg);

	/**
	 * Destructor.
	 */
	~CtrUi();

	/**
	 * Inherited from BaseUi.
	 */
	/** @{ */

	void ToggleFullscreen() override;
	void ToggleZoom() override;
	void UpdateDisplay() override;
	void SetTitle(const std::string &title) override;
	bool ShowCursor(bool flag) override;
	void ProcessEvents() override;

#ifdef SUPPORT_AUDIO
	AudioInterface& GetAudio();
#endif

	/** @} */

private:
	C3D_RenderTarget *top_screen, *bottom_screen;
	C2D_Image top_image, bottom_image;
	C2D_SpriteSheet assets;
	bool fullscreen;
	bool trigger_state;
	bool show_touchscreen;

#ifdef SUPPORT_AUDIO
	std::unique_ptr<AudioInterface> audio_;
#endif
	
};

#endif

#endif
