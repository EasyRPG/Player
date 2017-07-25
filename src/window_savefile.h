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

/**
 * Window Save File Class.
 */
class Window_SaveFile :	public Window_Base {
public:
	/**
	 * Constructor.
	 */
	Window_SaveFile(int ix, int iy, int iwidth, int iheight);

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
	 * Party data displayed in the savegame slot.
	 *
	 * @param actors face_id and face_name of all party members.
	 * @param name name of the First party member.
	 * @param hp HP of the first party member.
	 * @param level level of the First party member.
	 */
	void SetParty(const std::vector<std::pair<int, std::string> >& actors,
		std::string name, int hp, int level);

	/**
	 * Gets if the slot holds a valid save.
	 *
	 * @return Whether save is valid
	 */
	bool IsValid();

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
	void UpdateCursorRect();

	int index;
	std::vector<std::pair<int, std::string> > party;
	std::string hero_name;
	int hero_hp;
	int hero_level;
	bool corrupted;
	bool has_save;
};

#endif
