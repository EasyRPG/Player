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

#ifndef EP_PLATFORM_PSP_UI_H
#define EP_PLATFORM_PSP_UI_H

// Headers
#include "baseui.h"
#include "color.h"
#include "rect.h"
#include "system.h"
#include <cstdint>
#include <vector>

/**
 * PspUi class.
 *
 * Software renderer: the game frame is drawn into a cached RAM surface and
 * copied (1:1, centered) into one of two VRAM framebuffers of the 480x272
 * PSP screen, which are swapped on every vblank.
 */
class PspUi final : public BaseUi {
public:
	/**
	 * Constructor.
	 *
	 * @param width game frame width.
	 * @param height game frame height.
	 * @param cfg config options
	 */
	PspUi(int width, int height, const Game_Config& cfg);

	/**
	 * Destructor.
	 */
	~PspUi();

	/**
	 * Inherited from BaseUi.
	 */
	/** @{ */
	void UpdateDisplay() override;
	bool ProcessEvents() override;
	void SetScalingMode(ConfigEnum::ScalingMode) override;
	void ToggleStretch() override;
	void ToggleTouchUi() override;
	void ToggleVsync() override;
	void vGetConfig(Game_ConfigVideo& cfg) const override;

#ifdef SUPPORT_AUDIO
	AudioInterface& GetAudio() override;
#endif
	/** @} */

private:
	int frame_width = 0;
	int frame_height = 0;

	// Game frame in cacheable RAM, wrapped by main_surface
	std::vector<uint32_t> frame_pixels;

	// Index of the VRAM framebuffer that is written next (never the one on screen)
	int back_buffer = 1;

#ifdef SUPPORT_AUDIO
	std::unique_ptr<AudioInterface> audio_;
#endif
};

#endif
