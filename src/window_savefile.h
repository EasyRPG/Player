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

#ifndef EP_WINDOW_SAVEFILE_H
#define EP_WINDOW_SAVEFILE_H

// Headers
#include <string>
#include "window_base.h"
#include <lcf/rpg/savetitle.h>

/**
 * Window Save File Class.
 */
class Window_SaveFile :	public Window_Base {
public:
	/**
	 * Constructor.
	 */
	Window_SaveFile(Scene* parent, int ix, int iy, int iwidth, int iheight);

	/**
	 * Renders the current save on the window.
	 */
	void Refresh();

	/**
	 * Sets the ID of the savegame.
	 *
	 * @param id savegame ID
	 */
	void SetIndex(int id);

	/**
	 * Sets a name (usually a path) to show instead of the File ID, and
	 *  push file ID display to Line 2.
	 * Useful for save file importing, where multiple "Save01.lsd" files may exist.
	 *
	 * @param name The name to show instead of the file ID
	 * @param index The file index override, shown on line 2
	 */
	void SetDisplayOverride(const std::string& name, int index);

	/**
	 * Set party data displayed in the savegame slot.
	 *
	 * @param title the savegame party data to set.
	 */
	void SetParty(lcf::rpg::SaveTitle title);

	/**
	 * Gets if the slot holds a valid save.
	 *
	 * @return Whether save is valid
	 */
	bool IsValid() const;

	/**
	 * @return Whether the save slot contains party information from a save file.
	 */
	bool HasParty() const;

	/**
	 * Sets if there is a savegame in the slot.
	 *
	 * @param has_save true if has save
	 */
	void SetHasSave(bool has_save);

	/**
	 * Sets if the savegame is broken.
	 * Displays an error message in that case.
	 */
	void SetCorrupted(bool corrupted);

	void Update() override;

protected:
	bool IsSystemGraphicUpdateAllowed() const override;

	void UpdateCursorRect();
	std::string GetSaveFileName() const;

	int index = 0;
	std::string override_name;
	int override_index = 0;
	lcf::rpg::SaveTitle data;
	bool corrupted = false;
	bool has_save = false;
	bool has_party = false;
};

inline bool Window_SaveFile::IsSystemGraphicUpdateAllowed() const {
	return false;
}

#endif
