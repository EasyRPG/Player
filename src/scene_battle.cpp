/////////////////////////////////////////////////////////////////////////////
// This file is part of EasyRPG Player.
// 
// EasyRPG Player is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// EasyRPG Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <algorithm>
#include "input.h"
#include "output.h"
#include "player.h"
#include "sprite.h"
#include "cache.h"
#include "game_system.h"
#include "game_temp.h"
#include "game_party.h"
#include "scene_battle.h"

////////////////////////////////////////////////////////////
Scene_Battle::Scene_Battle() {
	Scene::type = Scene::Battle;
}

////////////////////////////////////////////////////////////
Scene_Battle::~Scene_Battle() {
}

////////////////////////////////////////////////////////////
void Scene_Battle::Enemy::CreateSprite() {
	Bitmap* graphic = Cache::Monster(enemy->battler_name);
	bool hue_change = enemy->battler_hue != 0;
	if (hue_change) {
		Surface* new_graphic = Surface::CreateSurface(graphic->GetWidth(), graphic->GetHeight());
		new_graphic->HueChangeBlit(0, 0, graphic, graphic->GetRect(), enemy->battler_hue);
		delete graphic;
		graphic = new_graphic;
	}

	sprite = new Sprite();
	sprite->SetBitmap(graphic, hue_change);
	sprite->SetOx(graphic->GetWidth() / 2);
	sprite->SetOy(graphic->GetHeight() / 2);
	sprite->SetX(member->x);
	sprite->SetY(member->y);
	sprite->SetZ(member->y);
	sprite->SetVisible(visible);
}

void Scene_Battle::Ally::CreateSprite() {
	if (Player::engine != Player::EngineRpg2k3)
		return;

	const RPG::BattlerAnimation& anim = Data::battleranimations[rpg_actor->battler_animation - 1];
	const RPG::BattlerAnimationExtension& ext = anim.base_data[anim_state];
	Bitmap* graphic = Cache::BattleCharset(ext.battler_name);

	sprite = new Sprite();
	sprite->SetBitmap(graphic);
	sprite->SetSrcRect(Rect(0, ext.battler_index * 48, 48, 48));
	sprite->SetOx(24);
	sprite->SetOy(24);
	sprite->SetX(rpg_actor->battle_x);
	sprite->SetY(rpg_actor->battle_y);
	sprite->SetZ(rpg_actor->battle_y);
}

////////////////////////////////////////////////////////////
void Scene_Battle::CreateSprites() {
	for (std::vector<Enemy>::iterator it = enemies.begin(); it != enemies.end(); it++)
		it->CreateSprite();

	for (std::vector<Ally>::iterator it = allies.begin(); it != allies.end(); it++)
		it->CreateSprite();
}

////////////////////////////////////////////////////////////
void Scene_Battle::CreateCursors() {
	Bitmap* system2 = Cache::System2(Data::system.system2_name);

	ally_cursor = new Sprite();
	ally_cursor->SetBitmap(system2);
	ally_cursor->SetSrcRect(Rect(0, 16, 16, 16));
	ally_cursor->SetZ(999);
	ally_cursor->SetVisible(false);

	enemy_cursor = new Sprite();
	enemy_cursor->SetBitmap(system2);
	enemy_cursor->SetSrcRect(Rect(0, 0, 16, 16));
	enemy_cursor->SetZ(999);
	enemy_cursor->SetVisible(false);
}

////////////////////////////////////////////////////////////
void Scene_Battle::CreateWindows() {
	help_window = new Window_Help(0, 0, 320, 32);
	help_window->SetVisible(false);

	std::vector<std::string> options;
	options.push_back(Data::terms.battle_fight);
	options.push_back(Data::terms.battle_auto);
	options.push_back(Data::terms.battle_escape);

	options_window = new Window_BattleCommand(0, 172, 76, 68, options);

	status_window = new Window_BattleStatus();

	std::vector<std::string> commands;
	commands.push_back(!Data::terms.command_attack.empty() ? Data::terms.command_attack : "Attack");
	commands.push_back(!Data::terms.command_defend.empty() ? Data::terms.command_defend : "Defend");
	commands.push_back(!Data::terms.command_item.empty() ? Data::terms.command_item : "Item");
	commands.push_back(!Data::terms.command_skill.empty() ? Data::terms.command_skill : "Skill");

	command_window = new Window_BattleCommand(244, 172, 76, 68, commands);

	skill_window = new Window_Skill(0, 172, 320, 68);
	skill_window->SetVisible(false);
	skill_window->SetHelpWindow(help_window);

	item_window = new Window_Item(0, 172, 320, 68);
	item_window->SetVisible(false);
	item_window->SetHelpWindow(help_window);
}

////////////////////////////////////////////////////////////
void Scene_Battle::Start() {
	if (Player::battle_test_flag) {
		if (Player::battle_test_troop_id <= 0) {
			Output::Error("Invalid Monster Party Id");
		}
	}

	cycle = 0;
	active_enemy = -1;

	if (!Game_Temp::battle_background.empty())
		background = new Background(Game_Temp::battle_background);
	else
		background = new Background(Game_Temp::battle_terrain_id);

	troop = &Data::troops[Game_Temp::battle_troop_id - 1];

	for (std::vector<RPG::TroopMember>::const_iterator it = troop->members.begin();
		 it != troop->members.end();
		 it++)
		enemies.push_back(Enemy(&*it));

	const std::vector<Game_Actor*>& actors = Game_Party::GetActors();
	for (std::vector<Game_Actor*>::const_iterator it = actors.begin();
		 it != actors.end();
		 it++)
		allies.push_back(Ally(*it));

	CreateSprites();
	CreateCursors();
	CreateWindows();

	int gauge = Game_Temp::battle_first_strike ? Ally::gauge_full : 0;
	std::fill(status_window->gauges, status_window->gauges+4, gauge);
	for (std::vector<Ally>::iterator it = allies.begin(); it != allies.end(); it++)
		it->gauge = gauge;

	SetState(Options);
}

////////////////////////////////////////////////////////////
void Scene_Battle::Terminate() {
	// Remove conditions which end after battle
	for (std::vector<Ally>::iterator it = allies.begin(); it != allies.end(); it++)
		it->game_actor->RemoveStates();

	delete help_window;
	delete options_window;
	delete status_window;
	delete command_window;
	delete item_window;
	delete skill_window;
	delete background;
}

////////////////////////////////////////////////////////////
void Scene_Battle::SetState(Scene_Battle::State _state) {
	state = _state;

	options_window->SetActive(false);
	status_window->SetActive(false);
	command_window->SetActive(false);
	item_window->SetActive(false);
	skill_window->SetActive(false);

	switch (state) {
		case Options:
			options_window->SetActive(true);
			break;
		case Battle:
			status_window->SetActive(true);
			break;
		case AutoBattle:
			break;
		case Command:
			command_window->SetActive(true);
			break;
		case Target:
			break;
		case Item:
			item_window->SetActive(true);
			break;
		case Skill:
			skill_window->SetActive(true);
			skill_window->SetActor(GetActiveActor());
			break;
	}

	options_window->SetVisible(false);
	status_window->SetVisible(false);
	command_window->SetVisible(false);
	item_window->SetVisible(false);
	skill_window->SetVisible(false);
	help_window->SetVisible(false);

	switch (state) {
		case Options:
			options_window->SetVisible(true);
			status_window->SetVisible(true);
			status_window->SetX(76);
			break;
		case Battle:
		case AutoBattle:
		case Command:
		case Target:
			status_window->SetVisible(true);
			status_window->SetX(0);
			command_window->SetVisible(true);
			break;
		case Item:
			item_window->SetVisible(true);
			help_window->SetVisible(true);
			break;
		case Skill:
			skill_window->SetVisible(true);
			help_window->SetVisible(true);
			break;
	}
}

////////////////////////////////////////////////////////////
void Scene_Battle::Attack() {
	int index = status_window->GetActiveCharacter();
	Ally& ally = allies[index];
	// FIXME: Attack
	ally.anim_state = Ally::Idle;
	ally.gauge = 0;
	SetState(Battle);
}

void Scene_Battle::Defend() {
	int index = status_window->GetActiveCharacter();
	Ally& ally = allies[index];
	ally.anim_state = Ally::Defending;
	ally.gauge = 0;
	SetState(Battle);
}

void Scene_Battle::UseItem() {
	int index = status_window->GetActiveCharacter();
	Ally& ally = allies[index];
	// FIXME: Use Item
	ally.anim_state = Ally::Idle;
	ally.gauge = 0;
	SetState(Battle);
}

void Scene_Battle::UseSkill() {
	int index = status_window->GetActiveCharacter();
	Ally& ally = allies[index];
	// FIXME: Use Skill
	ally.anim_state = Ally::Idle;
	ally.gauge = 0;
	SetState(Battle);
}

////////////////////////////////////////////////////////////
void Scene_Battle::Update() {
	options_window->Update();
	status_window->Update();
	command_window->Update();

	cycle++;

	if (Input::IsTriggered(Input::DECISION)) {
		Game_System::SePlay(Data::system.decision_se);
		switch (state) {
			case Options:
				switch (options_window->GetIndex()) {
					case 0:
						SetState(Battle);
						break;
					case 1:
						SetState(AutoBattle);
						break;
					case 2:
						if (Game_Temp::battle_escape_mode != 0) {
							Game_Temp::battle_result = Game_Temp::BattleEscape;
							Scene::Pop();
						}
						break;
				}
				break;
			case Battle:
				if (status_window->GetActiveCharacter() >= 0)
					SetState(Command);
				break;
			case AutoBattle:
				// no-op
				break;
			case Command:
				switch (command_window->GetIndex()) {
					case 0:
						active_enemy = 0;
						SetState(Target);
						break;
					case 1:
						Defend();
						break;
					case 2:
						SetState(Item);
						break;
					case 3:
						SetState(Skill);
						break;
				}
				break;
			case Target:
				Attack();
				SetState(Battle);
				break;
			case Item:
				UseItem();
				SetState(Battle);
				break;
			case Skill:
				UseSkill();
				SetState(Battle);
				break;
		}
	}

	if (Input::IsTriggered(Input::CANCEL)) {
		Game_System::SePlay(Data::system.cancel_se);
		switch (state) {
			case Options:
				Scene::Pop();
				break;
			case Battle:
			case AutoBattle:
				SetState(Options);
				break;
			case Command:
				SetState(Battle);
				break;
			case Target:
			case Item:
			case Skill:
				SetState(Command);
				break;
		}
	}

	int active_ally = status_window->GetActiveCharacter();
	if (active_ally >= 0) {
		const Ally& ally = allies[active_ally];
		ally_cursor->SetVisible(true);
		ally_cursor->SetX(ally.rpg_actor->battle_x - ally_cursor->GetWidth() / 2);
		ally_cursor->SetY(ally.rpg_actor->battle_y - ally.sprite->GetHeight() / 2 - ally_cursor->GetHeight() - 2);
		static const int frames[] = {0,1,2,1};
		int frame = frames[(cycle / 15) % 4];
		ally_cursor->SetSrcRect(Rect(frame * 16, 16, 16, 16));
	}
	else
		ally_cursor->SetVisible(false);

	if (state == Target && active_enemy >= 0) {
		if (Input::IsRepeated(Input::DOWN))
			active_enemy++;
		if (Input::IsRepeated(Input::UP))
			active_enemy--;

		active_enemy += enemies.size();
		active_enemy %= enemies.size();

		if (!enemies[active_enemy].visible) {
			for (int i = 1; i < (int) enemies.size(); i++) {
				active_enemy++;
				if (enemies[active_enemy].visible)
					break;
			}
			if (!enemies[active_enemy].visible)
				active_enemy = -1;
		}
	}

	if (state == Target && active_enemy >= 0) {
		const Enemy& enemy = enemies[active_enemy];
		enemy_cursor->SetVisible(true);
		enemy_cursor->SetX(enemy.member->x + enemy.sprite->GetWidth() / 2 + 2);
		enemy_cursor->SetY(enemy.member->y - enemy_cursor->GetHeight() / 2);
		static const int frames[] = {0,1,2,1};
		int frame = frames[(cycle / 15) % 4];
		enemy_cursor->SetSrcRect(Rect(frame * 16, 0, 16, 16));
	}
	else
		enemy_cursor->SetVisible(false);

	for (std::vector<Enemy>::iterator it = enemies.begin(); it != enemies.end(); it++) {
		if (!it->enemy->levitate)
			continue;
		int y = (int) (3 * sin(cycle / 15.0));
		it->sprite->SetY(it->member->y + y);
		it->sprite->SetZ(it->member->y + y);
	}

	for (std::vector<Ally>::iterator it = allies.begin(); it != allies.end(); it++) {
		if (Player::engine == Player::EngineRpg2k3) {
			const RPG::BattlerAnimation& anim = Data::battleranimations[it->rpg_actor->battler_animation - 1];
			const RPG::BattlerAnimationExtension& ext = anim.base_data[it->anim_state];
			static const int frames[] = {0,1,2,1};
			int frame = frames[(cycle / 15) % 4];
			it->sprite->SetSrcRect(Rect(frame * 48, ext.battler_index * 48, 48, 48));
		}

		switch (state) {
			case Battle:
			case AutoBattle:
				if (it->gauge < Ally::gauge_full) {
					// FIXME: this should account for agility, paralysis, etc
					it->gauge++;
					status_window->SetTimeGauge(it - allies.begin(), it->gauge, Ally::gauge_full);
				}
				break;
			default:
				break;
		}
	}
}

////////////////////////////////////////////////////////////
int Scene_Battle::GetActiveActor() {
	int idx = status_window->GetActiveCharacter();
	if (idx < 0)
		return idx;
	
	const std::vector<Game_Actor*>& actors = Game_Party::GetActors();
	return actors[idx]->GetId();
}

