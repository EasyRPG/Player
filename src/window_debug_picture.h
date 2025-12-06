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

#ifndef EP_WINDOW_DEBUG_PICTURE_H
#define EP_WINDOW_DEBUG_PICTURE_H

#include "window_selectable.h"
#include "window_base.h"
#include <vector>
#include <string>

 /**
  * Debug window showing the list of active pictures.
  */
class Window_DebugPictureList : public Window_Selectable {
public:
	Window_DebugPictureList(int x, int y, int w, int h);

	void Refresh();
	int GetPictureId() const;

private:
	std::vector<int> picture_ids;
};

/**
 * Debug window showing details of a specific picture.
 */
class Window_DebugPictureInfo : public Window_Base {
public:
	Window_DebugPictureInfo(int x, int y, int w, int h);

	void SetPictureId(int id);
	void Refresh();

private:
	int picture_id = 0;

	// Draw label and value. Returns next Y.
	int DrawLine(int y, std::string_view label, std::string_view value);

	// Draw two label/value pairs on one line. Returns next Y.
	int DrawDualLine(int y, std::string_view l1, std::string_view v1, std::string_view l2, std::string_view v2);

	// Draw a separator line.
	int DrawSeparator(int y);

	// Helper for drawing boolean flags compactly
	struct FlagInfo {
		const char* name;
		bool active;
	};
	int DrawFlags(int y, const std::vector<FlagInfo>& flags);
};

#endif
