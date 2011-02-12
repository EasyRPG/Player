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
#include "game_switches.h"
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
	const RPG::BattlerAnimationExtension& ext = anim.base_data[anim_state - 1];
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

	item_window = new Window_BattleItem(0, 172, 320, 68);
	item_window->SetVisible(false);
	item_window->SetHelpWindow(help_window);
	item_window->Refresh();
	item_window->SetIndex(0);
}

////////////////////////////////////////////////////////////
void Scene_Battle::Start() {
	if (Player::battle_test_flag) {
		if (Player::battle_test_troop_id <= 0) {
			Output::Error("Invalid Monster Party Id");
		}
	}

	cycle = 0;
	target_enemy = -1;
	target_ally = -1;

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

	SetState(State_Options);
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
void Scene_Battle::SetState(Scene_Battle::State new_state) {
	target_state = state;
	state = new_state;

	options_window->SetActive(false);
	status_window->SetActive(false);
	command_window->SetActive(false);
	item_window->SetActive(false);
	skill_window->SetActive(false);

	switch (state) {
		case State_Options:
			options_window->SetActive(true);
			break;
		case State_Battle:
			status_window->SetActive(true);
			break;
		case State_AutoBattle:
			break;
		case State_Command:
			command_window->SetActive(true);
			break;
		case State_TargetEnemy:
			break;
		case State_TargetAlly:
			status_window->SetActive(true);
			break;
		case State_Item:
			item_window->SetActive(true);
			item_window->SetActor(GetActiveActor());
			item_window->SetHaveCorpse(HaveCorpse());
			item_window->Refresh();
			break;
		case State_Skill:
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
		case State_Options:
			options_window->SetVisible(true);
			status_window->SetVisible(true);
			status_window->SetX(76);
			break;
		case State_Battle:
		case State_AutoBattle:
		case State_Command:
		case State_TargetEnemy:
		case State_TargetAlly:
			status_window->SetVisible(true);
			status_window->SetX(0);
			command_window->SetVisible(true);
			break;
		case State_Item:
			item_window->SetVisible(true);
			help_window->SetVisible(true);
			break;
		case State_Skill:
			skill_window->SetVisible(true);
			help_window->SetVisible(true);
			break;
	}
}

////////////////////////////////////////////////////////////
void Scene_Battle::Message(const std::string& msg) {
}

////////////////////////////////////////////////////////////
void Scene_Battle::UpdateAnimState(Ally& ally, int default_state) {
	const RPG::State* state = ally.game_actor->GetState();
	ally.anim_state = state
		? (state->battler_animation_id == 100 ? 7 : state->battler_animation_id + 1)
		: default_state;
}

////////////////////////////////////////////////////////////
void Scene_Battle::Restart(Ally& ally, int state) {
	UpdateAnimState(ally, state);
	ally.gauge = 0;
	SetState(State_Battle);
}

////////////////////////////////////////////////////////////
void Scene_Battle::Attack() {
	Ally& ally = allies[active_ally];
	// FIXME: Attack
	Restart(ally);
}

void Scene_Battle::Defend() {
	Ally& ally = allies[active_ally];
	Restart(ally, Ally::Defending);
}

void Scene_Battle::UseItem() {
	Ally& ally = allies[active_ally];

	int item_id = item_window->GetItemId();
	if (item_id <= 0) {
		Game_System::SePlay(Data::system.buzzer_se);
		return;
	}

	const RPG::Item& item = Data::items[item_id - 1];
	switch (item.type) {
		case RPG::Item::Type_normal:
			Game_System::SePlay(Data::system.buzzer_se);
			break;
		case RPG::Item::Type_weapon:
		case RPG::Item::Type_shield:
		case RPG::Item::Type_armor:
		case RPG::Item::Type_helmet:
		case RPG::Item::Type_accessory:
			if (item.use_skill)
				UseItemSkill(ally, item);
			else
				// can't be used
				Game_System::SePlay(Data::system.buzzer_se);
			break;
		case RPG::Item::Type_medicine:
			if (item.entire_party) {
				UseItem(ally, item, Target_Allies, NULL, NULL);
				Restart(ally);
			}
			else
				SetState(State_TargetAlly);
			break;
		case RPG::Item::Type_book:
		case RPG::Item::Type_material:
			// can't be used in battle?
			Game_System::SePlay(Data::system.buzzer_se);
			break;
		case RPG::Item::Type_special:
			UseItemSkill(ally, item);
			break;
		case RPG::Item::Type_switch:
			Game_Switches[item.switch_id] = true;
			Restart(ally);
			break;
	}
}

void Scene_Battle::UseSkill() {
	Ally& ally = allies[active_ally];

	int skill_id = skill_window->GetSkillId();
	if (skill_id <= 0) {
		Game_System::SePlay(Data::system.buzzer_se);
		return;
	}

	const RPG::Skill& skill = Data::skills[skill_id - 1];

	UseSkill(ally, skill);
}

void Scene_Battle::UseItemSkill(Ally& ally, const RPG::Item& item) {
	const RPG::Skill& skill = Data::skills[item.skill_id - 1];
	UseSkill(ally, skill);
}

void Scene_Battle::UseSkill(Ally& ally, const RPG::Skill& skill) {
	switch (skill.scope) {
		case RPG::Skill::Scope_enemy:
			SetState(State_TargetEnemy);
			return;
		case RPG::Skill::Scope_enemies:
			UseSkill(ally, skill, Target_Enemies);
			Restart(ally);
			break;
		case RPG::Skill::Scope_self:
			UseSkill(ally, skill, Target_Self);
			Restart(ally);
			break;
		case RPG::Skill::Scope_ally:
			SetState(State_TargetAlly);
			return;
		case RPG::Skill::Scope_party:
			UseSkill(ally, skill, Target_Allies);
			Restart(ally);
			break;
	}
}

void Scene_Battle::TargetDone() {
	switch (target_state) {
		case State_Command:
			Attack();
			break;
		case State_Item:
			UseItemDone();
			break;
		case State_Skill:
			UseSkillDone();
			break;
		default:
			break;
	}
}

void Scene_Battle::UseItemDone() {
	Ally& ally = allies[active_ally];
	int item_id = item_window->GetItemId();
	const RPG::Item& item = Data::items[item_id - 1];

	switch (state) {
		case State_TargetAlly:
			UseItem(ally, item, Target_Ally, &allies[target_ally], NULL);
			break;
		case State_TargetEnemy:
			UseItem(ally, item, Target_Enemy, NULL, &enemies[target_enemy]);
			break;
		default:
			break;
	}

	Restart(ally);
}

void Scene_Battle::UseSkillDone() {
	Ally& ally = allies[active_ally];
	int skill_id = skill_window->GetSkillId();
	const RPG::Skill& skill = Data::skills[skill_id - 1];

	switch (state) {
		case State_TargetAlly:
			UseSkill(ally, skill, Target_Ally, &allies[target_ally], NULL);
			break;
		case State_TargetEnemy:
			UseSkill(ally, skill, Target_Enemy, NULL, &enemies[target_enemy]);
			break;
		default:
			break;
	}

	Restart(ally);
}

void Scene_Battle::UseItem(Ally& ally, const RPG::Item& item,
						   Target target, Ally* target_ally, Enemy* target_enemy) {
	// FIXME: Use Item
	// FIXME: Decrement item count or use count
	item_window->Refresh();
}

void Scene_Battle::UseSkill(Ally& ally, const RPG::Skill& skill,
							Target target, Ally* target_ally, Enemy* target_enemy) {
	// FIXME: use skill
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
			case State_Options:
				switch (options_window->GetIndex()) {
					case 0:
						SetState(State_Battle);
						break;
					case 1:
						SetState(State_AutoBattle);
						break;
					case 2:
						if (Game_Temp::battle_escape_mode != 0) {
							Game_Temp::battle_result = Game_Temp::BattleEscape;
							Scene::Pop();
						}
						break;
				}
				break;
			case State_Battle:
				active_ally = status_window->GetActiveCharacter();
				if (active_ally >= 0)
					SetState(State_Command);
				break;
			case State_AutoBattle:
				// no-op
				break;
			case State_Command:
				switch (command_window->GetIndex()) {
					case 0:
						target_enemy = 0;
						SetState(State_TargetEnemy);
						break;
					case 1:
						Defend();
						break;
					case 2:
						SetState(State_Item);
						break;
					case 3:
						SetState(State_Skill);
						break;
				}
				break;
			case State_TargetEnemy:
			case State_TargetAlly:
				TargetDone();
				break;
			case State_Item:
				UseItem();
				break;
			case State_Skill:
				UseSkill();
				break;
		}
	}

	if (Input::IsTriggered(Input::CANCEL)) {
		Game_System::SePlay(Data::system.cancel_se);
		switch (state) {
			case State_Options:
				Scene::Pop();
				break;
			case State_Battle:
			case State_AutoBattle:
				SetState(State_Options);
				break;
			case State_Command:
				SetState(State_Battle);
				break;
			case State_TargetEnemy:
			case State_Item:
			case State_Skill:
				SetState(State_Command);
				break;
			case State_TargetAlly:
				SetState(State_Item);
				break;
		}
	}

	target_ally = status_window->GetActiveCharacter();
	if (target_ally >= 0) {
		const Ally& ally = allies[target_ally];
		ally_cursor->SetVisible(true);
		ally_cursor->SetX(ally.rpg_actor->battle_x - ally_cursor->GetWidth() / 2);
		ally_cursor->SetY(ally.rpg_actor->battle_y - ally.sprite->GetHeight() / 2 - ally_cursor->GetHeight() - 2);
		static const int frames[] = {0,1,2,1};
		int frame = frames[(cycle / 15) % 4];
		ally_cursor->SetSrcRect(Rect(frame * 16, 16, 16, 16));
	}
	else
		ally_cursor->SetVisible(false);

	if (state == State_TargetEnemy && target_enemy >= 0) {
		if (Input::IsRepeated(Input::DOWN))
			target_enemy++;
		if (Input::IsRepeated(Input::UP))
			target_enemy--;

		target_enemy += enemies.size();
		target_enemy %= enemies.size();

		if (!enemies[target_enemy].visible) {
			for (int i = 1; i < (int) enemies.size(); i++) {
				target_enemy++;
				if (enemies[target_enemy].visible)
					break;
			}
			if (!enemies[target_enemy].visible)
				target_enemy = -1;
		}
	}

	if (state == State_TargetEnemy && target_enemy >= 0) {
		const Enemy& enemy = enemies[target_enemy];
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
		int y = (int) (3 * sin(cycle / 30.0));
		it->sprite->SetY(it->member->y + y);
		it->sprite->SetZ(it->member->y + y);
	}

	for (std::vector<Ally>::iterator it = allies.begin(); it != allies.end(); it++) {
		if (Player::engine == Player::EngineRpg2k3) {
			const RPG::BattlerAnimation& anim = Data::battleranimations[it->rpg_actor->battler_animation - 1];
			const RPG::BattlerAnimationExtension& ext = anim.base_data[it->anim_state - 1];
			static const int frames[] = {0,1,2,1};
			int frame = frames[(cycle / 15) % 4];
			it->sprite->SetSrcRect(Rect(frame * 48, ext.battler_index * 48, 48, 48));
		}

		switch (state) {
			case State_Battle:
			case State_AutoBattle:
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
	if (active_ally < 0)
		return active_ally;
	
	const std::vector<Game_Actor*>& actors = Game_Party::GetActors();
	return actors[active_ally]->GetId();
}

////////////////////////////////////////////////////////////
bool Scene_Battle::HaveCorpse() {
	for (std::vector<Ally>::iterator it = allies.begin(); it != allies.end(); it++)
		if (it->game_actor->IsDead())
			return true;
	return false;
}

