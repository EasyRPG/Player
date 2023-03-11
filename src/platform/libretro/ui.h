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

#ifndef EP_PLATFORM_LIBRETRO_UI_H
#define EP_PLATFORM_LIBRETRO_UI_H

// Headers
#include "audio.h"
#include "baseui.h"
#include "color.h"
#include "rect.h"
#include "system.h"

#include "libretro.h"

/**
 * LibretroUi class.
 */
class LibretroUi final : public BaseUi {
public:
	/**
	 * Constructor.
	 *
	 * @param width window client width.
	 * @param height window client height.
	 * @param cfg video config options
	 */
	LibretroUi(int width, int height, const Game_Config& cfg);

	/**
	 * Inherited from BaseUi.
	 */
	/** @{ */
	bool vChangeDisplaySurfaceResolution(int new_width, int new_height) override;
	void UpdateDisplay() override;
	void ProcessEvents() override;
	void vGetConfig(Game_ConfigVideo& cfg) const override;

#ifdef SUPPORT_AUDIO
	AudioInterface& GetAudio() override;
	std::unique_ptr<AudioInterface> audio_;
#endif
	/** @} */

	static void SetRetroVideoCallback(retro_video_refresh_t cb);
	static void SetRetroInputStateCallback(retro_input_state_t cb);

	static retro_environment_t environ_cb;
	static retro_input_poll_t input_poll_cb;
	static bool player_exit_called;

	static Game_ConfigInput cfg_input;
private:
	static retro_video_refresh_t UpdateWindow;
	static retro_input_state_t CheckInputState;

	void UpdateVariables();
};

#endif
