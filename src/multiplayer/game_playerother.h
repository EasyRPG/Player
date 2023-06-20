#ifndef EP_GAME_PLAYEROTHER_H
#define EP_GAME_PLAYEROTHER_H

#include "game_character.h"
#include <lcf/rpg/savepartylocation.h>

using Game_PlayerBase = Game_CharacterDataStorage<lcf::rpg::SavePartyLocation>;

/**
 * Game_PlayerOther class
 * game character of other clients
 */
class Game_PlayerOther : public Game_PlayerBase {
	public:
		Game_PlayerOther(int id) : Game_CharacterDataStorage(PlayerOther), id(id)
		{
			SetDirection(lcf::rpg::EventPage::Direction_down);
			SetMoveSpeed(4);
			SetAnimationType(lcf::rpg::EventPage::AnimType_non_continuous);
		}

		bool IsMultiplayerVisible();

		void SetMultiplayerVisible(bool mv);

		int GetBaseOpacity();

		void SetBaseOpacity(int bo);

		int GetOpacity() const override;

		Drawable::Z_t GetScreenZ(bool apply_shift = false) const override {
			return Game_Character::GetScreenZ(apply_shift) | (0xFFFEu << 16u) + id;
		}

		void UpdateNextMovementAction() override {
			//literally just do nothing
		}

		void UpdateMovement(int amount) override {
			SetRemainingStep(GetRemainingStep() - amount);
			if (GetRemainingStep() <= 0) {
				SetRemainingStep(0);
				SetJumping(false);
			}

			SetStopCount(0);
		}

		void Update() {
			Game_Character::Update();
		}

	private:
		int id;
		bool multiplayer_visible;
		/* 0 = Invisible, 32 = Opaque */
		int base_opacity;
};

inline bool Game_PlayerOther::IsMultiplayerVisible() {
	return multiplayer_visible;
}

inline void Game_PlayerOther::SetMultiplayerVisible(bool mv) {
	multiplayer_visible = mv;
}

inline int Game_PlayerOther::GetBaseOpacity() {
	return base_opacity;
}

inline void Game_PlayerOther::SetBaseOpacity(int bo) {
	base_opacity = std::clamp(bo, 0, 32);
}

inline int Game_PlayerOther::GetOpacity() const {
	float opacity = (float)Game_Character::GetOpacity() * ((float)base_opacity / 32.0);
	return std::floor(opacity);
}

#endif