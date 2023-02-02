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
#include <cstdint>
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
	enum Type {
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
		TransitionCutIn,
		TransitionCutOut,
		TransitionNone
	};

	/**
	 * Gets the default number of frames for the given transition type.
	 *
	 * @param type Which type to request
	 *
	 * @return default number of frames for this transition type
	 */
	static int GetDefaultFrames(Type type);

	/** @return the transition singleton */
	static Transition& instance();

	/**
	 * Initiate a ShowScreen transition
	 *
	 * @param type transition type.
	 * @param linked_scene scene transitioning, it should be either the scene summoning (this) or the current instance (Scene::instance.get())
	 * @param duration transition duration, set to -1 to use the default number of frames.
	 */
	void InitShow(Type type, Scene *linked_scene, int duration = -1);

	/**
	 * Initiate an EraseScreen transition
	 *
	 * @param type transition type.
	 * @param linked_scene scene transitioning, it should be either the scene summoning (this) or the current instance (Scene::instance.get())
	 * @param duration transition duration, set to -1 to use the default number of frames.
	 */
	void InitErase(Type type, Scene *linked_scene, int duration = -1);

	void PrependFlashes(int r, int g, int b, int power, int duration, int iterations);

	void Draw(Bitmap& dst) override;
	void Update();

	bool IsActive() const;
	bool IsErasedNotActive() const;

	bool FromErase() const;
	bool ToErase() const;

private:
	Transition();
	void Init(Type type, Scene *linked_scene, int duration, bool erase);

	const uint32_t size_random_blocks = 4;

	BitmapRef screen1;
	BitmapRef screen2;
	BitmapRef random_block_transition;

	Type transition_type = TransitionNone;
	Scene *scene = nullptr;
	int current_frame = 0;
	int total_frames = 0;
	bool from_erase = false;
	bool to_erase = false;

	struct FlashData {
		int32_t red = 0;
		int32_t blue = 0;
		int32_t green = 0;
		double current_level = 0.0;
		int32_t time_left = 0;
	};
	FlashData flash;

	int flash_power = 0;
	int flash_duration = 0;
	int flash_iterations = 0;

	std::vector<int> zoom_position;
	std::vector<uint32_t> random_blocks;
	uint32_t current_blocks_print;

	void SetAttributesTransitions();
};

inline Transition& Transition::instance() {
	static Transition transition;
	return transition;
}

inline void Transition::InitShow(Type type, Scene *linked_scene, int duration) {
	Init(type, linked_scene, duration, false);
}

inline void Transition::InitErase(Type type, Scene *linked_scene, int duration) {
	Init(type, linked_scene, duration, true);
}

inline bool Transition::IsActive() const {
	return current_frame < total_frames || flash_iterations != 0;
}

inline bool Transition::IsErasedNotActive() const {
	return ToErase() && !IsActive();
}

inline bool Transition::FromErase() const {
	return from_erase;
}

inline bool Transition::ToErase() const {
	return to_erase;
}

#endif
