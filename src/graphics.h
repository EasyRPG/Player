/////////////////////////////////////////////////////////////////////////////
// This file is part of EasyRPG Player.
//
// EasyRPG Player is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// EasyRPG Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
/////////////////////////////////////////////////////////////////////////////

#ifndef _GRAPHICS_H_
#define _GRAPHICS_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <string>
#include <list>
#include <SDL.h>
#include <boost/shared_ptr.hpp>
#include <boost/cstdint.hpp>

#include "system.h"
#include "drawable.h"
#include "zobj.h"

class Bitmap;

////////////////////////////////////////////////////////////
/// Graphics namespace.
/// Handles screen drawing.
////////////////////////////////////////////////////////////
namespace Graphics {
	////////////////////////////////////////////////////////
	/// Initializes Graphics.
	////////////////////////////////////////////////////////
	void Init();

	////////////////////////////////////////////////////////
	/// Disposes Graphics.
	////////////////////////////////////////////////////////
	void Quit();

	////////////////////////////////////////////////////////
	/// Updates the screen.
	////////////////////////////////////////////////////////
	void Update();

	////////////////////////////////////////////////////////
	/// Resets the fps count, should be called after an
	/// expensive operation.
	////////////////////////////////////////////////////////
	void FrameReset();

	////////////////////////////////////////////////////////
	/// Waits frames.
	/// @param duration : frames to wait
	////////////////////////////////////////////////////////
	void Wait(int duration);

	////////////////////////////////////////////////////////
	/// Get a bitmap with the actual contents of the screen.
	/// @return screen contents
	////////////////////////////////////////////////////////
	boost::shared_ptr<Bitmap> SnapToBitmap();

	/// Transition types.
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

	////////////////////////////////////////////////////////
	/// Does a screen transition.
	/// @param type : transition type
	/// @param duration : transition duration
	/// @param erase : erase screen flag
	////////////////////////////////////////////////////////
	void Transition(TransitionType type, int duration, bool erase = false);

	////////////////////////////////////////////////////////
	/// Freezes the screen, and prepares it for a
	/// transition.
	////////////////////////////////////////////////////////
	void Freeze();

	/// @return frame count since player started
	int GetFrameCount();

	/// @param framecount : frame count since player started
	void SetFrameCount(int framecount);

	void RegisterDrawable(uint32_t ID, Drawable* drawable);
	void RemoveDrawable(uint32_t ID);
	ZObj* RegisterZObj(int z, uint32_t ID);
	void RegisterZObj(int z, uint32_t ID, bool multiz);
	void RemoveZObj(uint32_t ID);
	void RemoveZObj(uint32_t ID, bool multiz);
	void UpdateZObj(ZObj* zobj, int z);

	extern bool fps_on_screen;
	extern uint32_t drawable_id;

	void Push();
	void Pop();
}

#endif
