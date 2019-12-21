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

#ifndef EP_TRANSITION_H
#define EP_TRANSITION_H

// Headers
#include <vector>
#include <string>
#include "drawable.h"
#include "system.h"
#include "scene.h"
#include "color.h"


/**
 * Transition class.
 * Shows the transition between two screens.
 */
class Transition : public Drawable {
public:
	enum TransitionType {
		TransitionFadeIn,
		TransitionFadeOut,
		TransitionRandomBlocks,
		TransitionRandomBlocksDown,
		TransitionRandomBlocksUp,
		TransitionBlindOpen,
		TransitionBlindClose,
		TransitionVerticalStripesIn,
		TransitionVerticalStripesOut,
		TransitionHorizontalStripesIn,
		TransitionHorizontalStripesOut,
		TransitionBorderToCenterIn,
		TransitionBorderToCenterOut,
		TransitionCenterToBorderIn,
		TransitionCenterToBorderOut,
		TransitionScrollUpIn,
		TransitionScrollDownIn,
		TransitionScrollLeftIn,
		TransitionScrollRightIn,
		TransitionScrollUpOut,
		TransitionScrollDownOut,
		TransitionScrollLeftOut,
		TransitionScrollRightOut,
		TransitionVerticalCombine,
		TransitionVerticalDivision,
		TransitionHorizontalCombine,
		TransitionHorizontalDivision,
		TransitionCrossCombine,
		TransitionCrossDivision,
		TransitionZoomIn,
		TransitionZoomOut,
		TransitionMosaicIn,
		TransitionMosaicOut,
		TransitionWaveIn,
		TransitionWaveOut,
		TransitionErase,
		TransitionNone
	};

	Transition();

	/**
	 * Defines a screen transition.
	 *
	 * @param type transition type.
	 * @param linked_scene scene transitioning, it should be either the scene summoning (this) or the current instance (Scene::instance.get())
	 * @param duration transition duration.
	 * @param erase erase screen flag.
	 */
	void Init(TransitionType type, Scene *linked_scene, int duration, bool erase = false);

	void AppendBefore(Color color, int duration, int iterations);

	void Draw(Bitmap& dst) override;
	void Update();

	bool IsActive();
	bool IsErased();

private:
	const uint32_t size_random_blocks = 4;

	BitmapRef black_screen;
	BitmapRef frozen_screen;
	BitmapRef old_frozen_screen;
	BitmapRef screen1;
	BitmapRef screen2;
	BitmapRef random_block_transition;

	TransitionType transition_type;
	Scene *scene;
	int current_frame = -1;
	int total_frames = -2;
	bool screen_erased = false;

	Color flash_color;
	int flash_duration = 0;
	int flash_iterations = 0;

	std::vector<int> zoom_position;
	std::vector<uint32_t> random_blocks;
	uint32_t current_blocks_print;

	void SetAttributesTransitions();
};

inline bool Transition::IsActive() {
	return current_frame <= total_frames;
}

inline bool Transition::IsErased() {
	return screen_erased && !IsActive();
}

#endif
