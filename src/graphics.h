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

#ifndef _GRAPHICS_H_
#define _GRAPHICS_H_

// Headers
#include <string>

#include "system.h"
#include "drawable.h"

/**
 * Graphics namespace.
 * Handles screen drawing.
 */
namespace Graphics {
	/**
	 * Initializes Graphics.
	 */
	void Init();

	/**
	 * Disposes Graphics.
	 **/
	void Quit();

	/**
	 * Updates the screen.
	 * The rendering is skipped when time_left is false.
	 *
	 * @param time_left Whether time is left to render the frame.
	 */
	void Update(bool time_left);

	/**
	 * Resets the fps count.
	 * Don't call this function directly, use Player::FrameReset.
	 */
	void FrameReset();

	/**
	 * Gets a bitmap with the actual contents of the screen.
	 * @return screen contents
	 */
	BitmapRef SnapToBitmap();

	/** Transition types. */
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

	/**
	 * Does a screen transition.
	 *
	 * @param type transition type.
	 * @param duration transition duration.
	 * @param erase erase screen flag.
	 */
	void Transition(TransitionType type, int duration, bool erase = false);

	/**
	 * Gets if a screen transition is executing.
	 */
	bool IsTransitionPending();

	/**
	 * Freezes the screen, and prepares it for a
	 * transition.
	 */
	void Freeze();

	void RegisterDrawable(Drawable* drawable);
	void RemoveDrawable(Drawable* drawable);

	void UpdateZCallback();

	void Push(bool draw_background = true);
	void Pop();

	/**
	 * Gets target frame rate.
	 *
	 * @return target frame rate
	 */
	int GetDefaultFps();
}

#endif
