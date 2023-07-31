#include "playerother.h"
#include "output.h"
#include "scene.h"
#include "drawable_mgr.h"
#include "../sprite_character.h"
#include "../battle_animation.h"
#include "game_playerother.h"
#include "nametag.h"

PlayerOther PlayerOther::Shadow(int x, int y) {
	PlayerOther po;
	auto scene_map = Scene::Find(Scene::SceneType::Map);
	if (!scene_map) {
		Output::Error("MP: unexpected, {}:{}", __FILE__, __LINE__);
		std::terminate();
	}

	auto old_list = &DrawableMgr::GetLocalList();
	DrawableMgr::SetLocalList(&scene_map->GetDrawableList());

	po.ch.reset(new Game_PlayerOther(0));
	{
		auto& p = *po.ch;
		p.SetX(x);
		p.SetY(y);
		p.SetFacing(ch->GetFacing());
		p.SetSpriteGraphic(ch->GetSpriteName(), ch->GetSpriteIndex());
		p.SetMoveSpeed(ch->GetMoveSpeed());
		p.SetMoveFrequency(ch->GetMoveFrequency());
		p.SetThrough(true);
		p.SetLayer(ch->GetLayer());
		p.SetMultiplayerVisible(false);
		// shadow disappears slowly
		p.SetBaseOpacity(32);
	}
	po.sprite.reset(new Sprite_Character(po.ch.get()));
	po.sprite->SetTone(sprite->GetTone());
	DrawableMgr::SetLocalList(old_list);
	return po;
}
