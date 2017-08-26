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

#ifndef EP_WINDOW_BASE_H
#define EP_WINDOW_BASE_H

// Headers
#include <string>
#include "window.h"
#include "game_actor.h"
#include "main_data.h"
#include "async_handler.h"
#include <map>

/**
 * Window Base class.
 */
class Window_Base : public Window {
public:
	/**
	 * Constructor.
	 *
	 * @param x window x position.
	 * @param y window y position.
	 * @param width window width.
	 * @param height window height.
	 */
	Window_Base(int x, int y, int width, int height);

	/**
	 * Updates the window.
	 */
	virtual void Update();

	/**
	 * Draw helpers.
	 */
	/** @{ */
	void DrawFace(const std::string& face_name, int face_index, int cx, int cy, bool flip = false);
	void DrawActorFace(const Game_Actor& actor, int cx, int cy);
	void DrawActorName(const Game_Battler& actor, int cx, int cy) const;
	void DrawActorTitle(const Game_Actor& actor, int cx, int cy) const;
	void DrawActorClass(const Game_Actor& actor, int cx, int cy) const;
	void DrawActorLevel(const Game_Actor& actor, int cx, int cy) const;
	void DrawActorState(const Game_Battler& actor, int cx, int cy) const;
	void DrawActorExp(const Game_Actor& actor, int cx, int cy) const;
	void DrawActorHp(const Game_Battler& actor, int cx, int cy, bool draw_max = true) const;
	void DrawActorSp(const Game_Battler& actor, int cx, int cy, bool draw_max = true) const;
	void DrawActorParameter(const Game_Battler& actor, int cx, int cy, int type) const;
	void DrawEquipmentType(const Game_Actor& actor, int cx, int cy, int type) const;
	void DrawItemName(const RPG::Item& item, int cx, int cy, bool enabled = true) const;
	void DrawSkillName(const RPG::Skill& skill, int cx, int cy, bool enabled = true) const;
	void DrawCurrencyValue(int money, int cx, int cy) const;
	void DrawGauge(const Game_Battler& actor, int cx, int cy) const;
	/** @} */

	/**
	 * Cancels async loading of faces.
	 * Used to prevent rendering faces that are loaded too slow on the wrong page.
	 */
	void CancelFace();

protected:
	void OnFaceReady(FileRequestResult* result, int face_index, int cx, int cy, bool flip);

	std::string windowskin_name;

	std::vector<FileRequestBinding> face_request_ids;
};

#endif
