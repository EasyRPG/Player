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

#ifndef EP_GAME_SCREEN_H
#define EP_GAME_SCREEN_H

#include <vector>
#include "system.h"
#include "game_picture.h"
#include "game_character.h"
#include "battle_animation.h"

class Game_Battler;
class Screen;

class Game_Screen {

public:
	Game_Screen();

	void SetupNewGame();
	void SetupFromSave();

	Game_Picture* GetPicture(int id);

	void Reset();
	void TintScreen(int r, int g, int b, int s, int tenths);
	void FlashOnce(int r, int g, int b, int s, int frames);
	void FlashBegin(int r, int g, int b, int s, int frames);
	void FlashEnd();
	void FlashMapStepDamage();
	void ShakeOnce(int power, int speed, int tenths);
	void ShakeBegin(int power, int speed);
	void ShakeEnd();
	void SetWeatherEffect(int type, int strength);
	void PlayMovie(const std::string& filename,
				   int pos_x, int pos_y, int res_x, int res_y);
	void Update();
	void UpdateGraphics();

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
	 * @return Flash color
	 */
	Color GetFlashColor() const;

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

	/**
	 * Plays the given animation against a character.
	 *
	 * @param animation_id the animation ID
	 * @param target_id the ID of the targeted character
	 * @param global whether to "show on the entire map"
	 * @param start_frame which frame to start on.
	 *
	 * @return the number of frames the animation will run.
	 */
	int ShowBattleAnimation(int animation_id, int target_id, bool global, int start_frame = 0);

	/**
	 * Update the currently running battle animation by 1 frame.
	 */
	void UpdateBattleAnimation();

	/**
	 * Cancel the currently running battle animation.
	 */
	void CancelBattleAnimation();

	/**
	 * Whether or not a battle animation is currently playing.
	 */
	bool IsBattleAnimationWaiting();

	/**
	 * Animates the screen shake algorithm given the parameters
	 *
	 * @param strength the strength of the shake
	 * @param speed of the shake
	 * @param time_left how much time is left in frames
	 * @param position current shake displacement
	 *
	 * @return next shake displacement
	 */
	static int AnimateShake(int strength, int speed, int time_left, int position);

	/**
	 * Get the X panning offset for full screen effects
	 *
	 * @return pan_x offset
	 */
	int GetPanX();

	/**
	 * Get the Y panning offset for full screen effects
	 *
	 * @return pan_y offset
	 */
	int GetPanY();

private:
	std::vector<Game_Picture> pictures;
	std::unique_ptr<BattleAnimation> animation;

	RPG::SaveScreen& data;
	int flash_sat;		// RPGMaker bug: this isn't saved
	int flash_period;	// RPGMaker bug: this isn't saved

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
	void UpdateFog(int dx, int dy);
	void CreatePicturesFromSave();
	void PreallocatePictureData(int id);
};

#endif
