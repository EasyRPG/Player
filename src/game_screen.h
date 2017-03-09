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

#ifndef _EASYRPG_GAME_SCREEN_H_
#define _EASYRPG_GAME_SCREEN_H_

#include <vector>
#include "system.h"
#include "game_picture.h"
#include "game_character.h"

class Game_Battler;
class Screen;

class Game_Screen {

public:
	Game_Screen();

	void CreatePicturesFromSave();

	Game_Picture* GetPicture(int id);

	void Reset();
	void TintScreen(int r, int g, int b, int s, int tenths);
	void FlashOnce(int r, int g, int b, int s, int tenths);
	void FlashBegin(int r, int g, int b, int s, int tenths);
	void FlashEnd();
	void ShakeOnce(int power, int speed, int tenths);
	void ShakeBegin(int power, int speed);
	void ShakeEnd();
	void SetWeatherEffect(int type, int strength);
	void PlayMovie(const std::string& filename,
				   int pos_x, int pos_y, int res_x, int res_y);
	void Update();

	/**
	 * Returns the current screen tone.
	 *
	 * @return Tone
	 */
	Tone GetTone();

	/**
	 * Returns the current flash color.
	 *
	 * @param current_level Current strength adjusted to 0-255
	 * @param time_left how many frames of flashing are left
	 * @return Flash color
	 */
	Color GetFlash(int& current_level, int& time_left);

	/**
	 * Returns the weather type
	 *
	 * @return Weather type
	 */
	int GetWeatherType();

	/**
	 * Returns weather effect strength from (0 low to 2 high)
	 *
	 * @return Weather effect strength
	 */
	int GetWeatherStrength();

	struct Snowflake {
		uint16_t x;
		uint8_t y;
		uint8_t life;
	};

	const std::vector<Snowflake>& GetSnowflakes();

	enum Weather {
		Weather_None,
		Weather_Rain,
		Weather_Snow,
		Weather_Fog,
		Weather_Sandstorm
	};

private:
	std::vector<std::unique_ptr<Game_Picture>> pictures;

	RPG::SaveScreen& data;
	int flash_sat;		// RPGMaker bug: this isn't saved
	int flash_period;	// RPGMaker bug: this isn't saved
	int shake_direction;

	std::string movie_filename;
	int movie_pos_x;
	int movie_pos_y;
	int movie_res_x;
	int movie_res_y;

protected:
	std::vector<Snowflake> snowflakes;

	void StopWeather();
	void InitSnowRain();
	void UpdateSnowRain(int speed);
};

#endif
