#ifndef EP_NAMETAG_H
#define EP_NAMETAG_H

#include <queue>

#include "game_multiplayer.h"

struct PlayerOther;

class NameTag : public Drawable {
public:
	NameTag(int id, PlayerOther& player, std::string nickname);

	void Draw(Bitmap& dst) override;

	void SetSystemGraphic(StringView sys_name);

	void SetEffectsDirty();

	void SetFlashFramesLeft(int frames);

	void SetTransparent(bool val);

private:
	PlayerOther& player;
	std::string nickname;
	BitmapRef nick_img;
	BitmapRef sys_graphic;
	BitmapRef effects_img;
	bool transparent;
	int base_opacity = 32;
	bool dirty = true;
	bool effects_dirty;
	Game_Multiplayer::NametagMode nametag_mode_cache;
	int flash_frames_left;
	int last_valid_sprite_y_offset;

	void SetBaseOpacity(int val);
	int GetOpacity();
	int GetSpriteYOffset();
};

inline void NameTag::SetEffectsDirty() {
	effects_dirty = true;
};

inline void NameTag::SetFlashFramesLeft(int frames) {
	flash_frames_left = frames;
};

inline void NameTag::SetBaseOpacity(int val) {
	base_opacity = std::clamp(val, 0, 32);
};

#endif
