#ifndef EP_PLAYEROTHER_H
#define EP_PLAYEROTHER_H

#include <deque>
#include <memory>

struct Game_PlayerOther;
struct Sprite_Character;
struct NameTag;
struct BattleAnimation;

struct PlayerOther {
	bool account; // player is on an account
	std::deque<std::pair<int, int>> mvq; // queue of move commands
	std::unique_ptr<Game_PlayerOther> ch; // character
	std::unique_ptr<Sprite_Character> sprite;
	std::unique_ptr<NameTag> name_tag;
	std::unique_ptr<BattleAnimation> battle_animation; // battle animation

	// create a shadow of this
	// shadow has no name, no battle animation and no move commands
	// but it is visible, in other words this function modifies the
	// global drawable list
	//
	// a shadow must be put inside dc_players after creation
	// destroying shadows outside dc_players is undefined behavior
	PlayerOther Shadow(int x, int y);
};

#endif
