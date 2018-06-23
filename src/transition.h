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
		TransitionRandomBlocksUp,
		TransitionRandomBlocksDown,
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
	~Transition() override;

	int GetZ() const override;
	DrawableType GetType() const override;
	
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

	void Draw() override;
	void Update();
	bool IsGlobal() const override;

	bool IsActive();
	bool IsErased();

private:

	static const int z = Priority_Transition;
	static const DrawableType type = TypeTransition;
	const uint32_t size_random_blocks = 4;

	BitmapRef black_screen;
	BitmapRef frozen_screen;
	BitmapRef old_frozen_screen;
	BitmapRef screen1;
	BitmapRef screen2;
	BitmapRef random_block_transition;

	TransitionType transition_type;
	Scene *scene;
	int current_frame;
	int total_frames;
	bool screen_erased;

	Color flash_color;
	int flash_duration;
	int flash_iterations;

	std::vector<int> zoom_position;
	std::vector<uint32_t> random_blocks;
	uint32_t current_blocks_print;

	void SetAttributesTransitions();
};

#endif
