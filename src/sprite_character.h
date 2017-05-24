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

#ifndef _SPRITE_CHARACTER_H_
#define _SPRITE_CHARACTER_H_

// Headers
#include "sprite.h"
#include <string>
#include "async_handler.h"

class Game_Character;
class FileRequestAsync;
struct FileRequestResult;

/**
 * Sprite character class.
 */
class Sprite_Character : public Sprite {
public:
	/**
	 * Constructor.
	 *
	 * @param character game character to display
	 */
	Sprite_Character(Game_Character* character);

	/**
	 * Updates sprite state.
	 */
	void Update();

	/**
	 * Gets game character.
	 *
	 * @return game character.
	 */
	Game_Character* GetCharacter();

	 /**
	  * Sets game character.
	  *
	  * @param character new game character.
	  */
	void SetCharacter(Game_Character* character);

private:
	Game_Character* character;

	int tile_id;
	std::string character_name;
	int character_index;

	int chara_width;
	int chara_height;

	/** Returns true for charset sprites; false for tiles. */
	bool UsesCharset() const;

	void OnTileSpriteReady(FileRequestResult*);
	void OnCharSpriteReady(FileRequestResult*);

	FileRequestBinding request_id;
};

#endif
