#include "nametag.h"
#include "../cache.h"
#include "../font.h"
#include "../drawable_mgr.h"
#include "../filefinder.h"
#include "../bitmap.h"
#include "../sprite_character.h"
#include "game_playerother.h"
#include "playerother.h"

std::map<std::string, std::array<int, 96>> sprite_y_offsets;

NameTag::NameTag(int id, PlayerOther& player, std::string nickname)
	:player(player),
	nickname(std::move(nickname)),
	Drawable(Priority_Screen + id) {
	DrawableMgr::Register(this);
}

void NameTag::Draw(Bitmap& dst) {
	auto nametag_mode = GMI().GetNametagMode();

	if (nametag_mode == Game_Multiplayer::NametagMode::NONE || nickname.empty() || !player.sprite.get()) {
		nick_img.reset();
		dirty = true;
		return;
	}

	if (nametag_mode_cache != nametag_mode) {
		nametag_mode_cache = nametag_mode;
		nick_img.reset();
		dirty = true;
	}

	if (dirty) {
		std::string nick_trim;

		if (nametag_mode != Game_Multiplayer::NametagMode::CLASSIC) {
			nick_trim = nickname;
		} else {
			nick_trim = nickname.substr(0, std::min(3, (int)nickname.size()));
		}

		// FIXME: Text::GetSize is broken and always returns a height of 0, use 12 for now
		auto rect = Text::GetSize(*Font::NameText(), nick_trim);
		nick_img = Bitmap::Create(rect.width, 12);

		Text::Draw(*nick_img, 0, 0, *Font::NameText(), *(sys_graphic ? sys_graphic : Cache::SystemOrBlack()), 0, nick_trim);

		dirty = false;

		effects_dirty = true;
	}

	if (flash_frames_left > 0) {
		--flash_frames_left;
		effects_dirty = true;
	}

	if (effects_dirty) {
		auto tone = player.sprite->GetTone();
		auto flash = player.sprite->GetCharacter()->GetFlashColor();

		effects_img.reset(); // maybe not needed?

		if (tone == Tone() && flash.alpha == 0) {
			effects_img = nick_img;
		} else {
			effects_img = Cache::SpriteEffect(nick_img, nick_img->GetRect(), false, false, tone, flash);
		}

		effects_dirty = false;
	}

	if (!player.ch->IsSpriteHidden()) {
		int x = player.ch->GetScreenX() - nick_img->GetWidth() / 2;
		int y = (player.ch->GetScreenY() - player.sprite->GetHeight()) + GetSpriteYOffset();

		if (transparent && base_opacity > 16) {
			SetBaseOpacity(base_opacity - 1);
		} else if (!transparent && base_opacity < 32) {
			SetBaseOpacity(base_opacity + 1);
		}

		dst.Blit(x, y, *effects_img, effects_img->GetRect(), Opacity(GetOpacity()));
	}
}

void NameTag::SetSystemGraphic(StringView sys_name) {
	sys_graphic = Cache::System(sys_name);
	dirty = true;
}

void NameTag::SetTransparent(bool val) {
	transparent = val;
}

int NameTag::GetOpacity() {
	float opacity = (float)player.ch->GetOpacity() * ((float)base_opacity / 32.0);
	return std::floor(opacity);
}

int NameTag::GetSpriteYOffset() {
	std::string sprite_name = player.ch->GetSpriteName();
	if (!sprite_y_offsets.count(sprite_name)) {
		auto filename = FileFinder::FindImage("CharSet", sprite_name);
		if (filename == "") {
			return 0;
		}

		auto offset_array = std::array<int, 96>{ 0 };

		const int BASE_OFFSET = -13;
		const size_t BGRA = 4;

		auto image = Cache::Charset(sprite_name);

		for (int hi = 0; hi < image->height() / 128; ++hi) {
			for (int wi = 0; wi < image->width() / 72; ++wi) {
				for (int fi = 0; fi < 4; ++fi) {
					for (int afi = 0; afi < 3; ++afi) {
						int i = ((hi << 2) + wi) * 12 + (fi * 3) + afi;

						int start_x = wi * 72 + afi * 24;
						int start_y = hi * 128 + fi * 32;

						int y = start_y;

						bool offset_found = false;

						for (; y < start_y + 32; ++y) {
							for (int x = start_x; x < start_x + 24; ++x) {
								size_t index = BGRA * (y * image->width() + x);
								auto pixels = reinterpret_cast<unsigned char*>(image->pixels());
								if (pixels[index + 3] != 0) { // check if alpha is not 0 (fully transparent)
									offset_found = true;
									break;
								}
							}

							if (offset_found) {
								break;
							}
						}

						if (offset_found) {
							if (y > start_y + 15) {
								y = start_y + 15;
							}
							offset_array[i] = BASE_OFFSET + (y - start_y);
						} else {
							offset_array[i] = 32;
						}
					}
				}
			}

			sprite_y_offsets[sprite_name] = std::array<int, 96>(offset_array);
		}
	}

	auto frame = player.ch->GetAnimFrame();
	if (frame >= lcf::rpg::EventPage::Frame_middle2) {
		frame = lcf::rpg::EventPage::Frame_middle;
	}

	int ret = sprite_y_offsets[sprite_name][player.ch->GetSpriteIndex() * 12 + player.ch->GetFacing() * 3 + frame];

	if (ret != 32) {
		last_valid_sprite_y_offset = ret;
	} else {
		return last_valid_sprite_y_offset;
	}

	return ret;
}
