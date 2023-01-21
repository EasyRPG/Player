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

#ifndef EP_PLATFORM_PSVITA_UI_H
#define EP_PLATFORM_PSVITA_UI_H

// Headers
#include "baseui.h"
#include "color.h"
#include "rect.h"
#include "system.h"
#include <vector>

/**
 * Psp2Ui class.
 */
class Psp2Ui final : public BaseUi {
public:
	/**
	 * Constructor.
	 *
	 * @param width window client width.
	 * @param height window client height.
	 * @param cfg config options
	 */
	Psp2Ui(int width, int height, const Game_Config& cfg);

	/**
	 * Destructor.
	 */
	~Psp2Ui();

	/**
	 * Inherited from BaseUi.
	 */
	/** @{ */
	void UpdateDisplay() override;
	bool LogMessage(const std::string &message) override;
	void ProcessEvents() override;
	void SetScalingMode(ScalingMode) override;
	void ToggleStretch() override;
	void ToggleTouchUi() override;
	void ToggleVsync() override;
	void vGetConfig(Game_ConfigVideo& cfg) const override;

#ifdef SUPPORT_AUDIO
	AudioInterface& GetAudio();
#endif
	/** @} */

private:

#ifdef SUPPORT_AUDIO
	std::unique_ptr<AudioInterface> audio_;
#endif

};

#endif
