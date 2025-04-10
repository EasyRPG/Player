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

#ifndef EP_SPRITE_CHARACTER_H
#define EP_SPRITE_CHARACTER_H

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
	 * @param x_offset X Render offset when being a clone
	 * @param y_offset Y Render offset when being a clone
	 */
	Sprite_Character(Game_Character* character, int x_offset = 0, int y_offset = 0);

	void Draw(Bitmap& dst) override;

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

	/**
	 * Returns a Rect describing the boundaries for a single character
	 *
	 * @param name name of charset
	 * @param index index of character
	 * @param bitmap_rect The dimensions of the charset bitmap
	 *
	 * @return Rect describing image sub region containing character frames.
	 */
	static Rect GetCharacterRect(std::string_view name, int index, Rect bitmap_rect);

	/**
	 * Called when the map chipset is changed by an event
	 */
	void ChipsetUpdated();

private:
	Game_Character* character;

	int tile_id;
	std::string character_name;
	int character_index;

	int chara_width;
	int chara_height;

	/** Returns true for charset sprites; false for tiles. */
	bool UsesCharset() const;

	int x_offset = 0;
	int y_offset = 0;
	bool refresh_bitmap = false;

	void OnTileSpriteReady(FileRequestResult*);
	void OnCharSpriteReady(FileRequestResult*);

	FileRequestBinding request_id;
};

#endif
