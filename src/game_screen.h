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
#include "options.h"
#include "game_picture.h"
#include "game_character.h"
#include "battle_animation.h"

class Game_Battler;
class Screen;
class Weather;

class Game_Screen {

public:
	Game_Screen();
	~Game_Screen();

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

	struct Particle {
		int16_t x = 0;
		int16_t y = 0;
		int16_t life = 0;
	};

	const std::vector<Particle>& GetParticles();

	enum WeatherType {
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

	/** @return current pan_x offset for screen effects in 1/16 pixels */
	int GetPanX() const;

	/** @return current pan_y offset for screen effects in 1/16 pixels */
	int GetPanY() const;

	/** @return maximum pan_x offset for screen effects in 1/16 pixels */
	static constexpr int GetPanLimitX();

	/** @return maximum pan_y offset for screen effects in 1/16 pixels */
	static constexpr int GetPanLimitY();

	/** @return a Rect describing position and size of screen effects in pixels */
	Rect GetScreenEffectsRect() const;

	/** @return Return screen shake X offset */
	int GetShakeOffsetX() const;

	/** @return Return screen shake Y offset */
	int GetShakeOffsetY() const;

private:
	std::vector<Game_Picture> pictures;
	std::unique_ptr<BattleAnimation> animation;
	std::unique_ptr<Weather> weather;

	RPG::SaveScreen& data;
	int flash_sat;		// RPGMaker bug: this isn't saved
	int flash_period;	// RPGMaker bug: this isn't saved

	std::string movie_filename;
	int movie_pos_x;
	int movie_pos_y;
	int movie_res_x;
	int movie_res_y;

protected:
	std::vector<Particle> particles;

	void StopWeather();
	void UpdateRain();
	void UpdateSnow();
	void UpdateFog(int dx, int dy);
	void OnWeatherChanged();
	void InitRainSnow(int lifetime);
	void CreatePicturesFromSave();
	void PreallocatePictureData(int id);
};

inline int Game_Screen::GetPanX() const {
	return data.pan_x;
}

inline int Game_Screen::GetPanY() const {
	return data.pan_y;
}

inline constexpr int Game_Screen::GetPanLimitX() {
	return SCREEN_TARGET_WIDTH * 16;
}

inline constexpr int Game_Screen::GetPanLimitY() {
	return SCREEN_TARGET_HEIGHT * 16 * 2 / 3;
}

inline Rect Game_Screen::GetScreenEffectsRect() const {
	return Rect{ GetPanX() / 16, GetPanY() / 16, GetPanLimitX() / 16, GetPanLimitY() / 16 };
}

inline int Game_Screen::GetShakeOffsetX() const {
	return data.shake_position;
}

inline int Game_Screen::GetShakeOffsetY() const {
	return data.shake_position_y;
}

inline Tone Game_Screen::GetTone() {
	return Tone((int) ((data.tint_current_red) * 128 / 100),
		(int) ((data.tint_current_green) * 128 / 100),
		(int) ((data.tint_current_blue) * 128 / 100),
		(int) ((data.tint_current_sat) * 128 / 100));
}

inline Color Game_Screen::GetFlashColor() const {
	return MakeFlashColor(data.flash_red, data.flash_green, data.flash_blue, data.flash_current_level);
}

inline int Game_Screen::GetWeatherType() {
	return data.weather;
}

inline int Game_Screen::GetWeatherStrength() {
	return data.weather_strength;
}

inline const std::vector<Game_Screen::Particle>& Game_Screen::GetParticles() {
	return particles;
}

inline bool Game_Screen::IsBattleAnimationWaiting() {
	return (bool)animation;
}

#endif
