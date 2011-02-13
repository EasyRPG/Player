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
#include <sstream>
#include "rpg_battlecommand.h"
#include "input.h"
#include "output.h"
#include "player.h"
#include "sprite.h"
#include "cache.h"
#include "game_system.h"
#include "game_temp.h"
#include "game_party.h"
#include "game_enemy.h"
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
Scene_Battle::Enemy::Enemy(const RPG::TroopMember* member, int id) :
	ID(id),
	game_enemy(new Game_Enemy(member->ID)),
	member(member),
	rpg_enemy(&Data::enemies[member->ID - 1]),
	sprite(NULL),
	fade(0)
{
	game_enemy->SetHidden(member->invisible);
}

void Scene_Battle::Enemy::CreateSprite() {
	Bitmap* graphic = Cache::Monster(rpg_enemy->battler_name);
	bool hue_change = rpg_enemy->battler_hue != 0;
	if (hue_change) {
		Surface* new_graphic = Surface::CreateSurface(graphic->GetWidth(), graphic->GetHeight());
		new_graphic->HueChangeBlit(0, 0, graphic, graphic->GetRect(), rpg_enemy->battler_hue);
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
	sprite->SetVisible(!game_enemy->IsHidden());
}

////////////////////////////////////////////////////////////
Scene_Battle::Ally::Ally(Game_Actor* game_actor, int id) :
	ID(id),
	game_actor(game_actor),
	rpg_actor(&Data::actors[game_actor->GetId() - 1]),
	sprite_frame(-1),
	sprite_file(""),
	sprite(NULL),
	anim_state(Idle),
	gauge(0)
{
}

void Scene_Battle::Ally::CreateSprite() {
	if (Player::engine != Player::EngineRpg2k3)
		return;

	sprite = new Sprite();
	sprite->SetOx(24);
	sprite->SetOy(24);
	sprite->SetX(rpg_actor->battle_x);
	sprite->SetY(rpg_actor->battle_y);
	sprite->SetZ(rpg_actor->battle_y);

	SetAnimState(anim_state);
	UpdateAnim(0);
}

void Scene_Battle::Ally::SetAnimState(int state) {
	if (Player::engine != Player::EngineRpg2k3)
		return;

	anim_state = state;

	const RPG::BattlerAnimation& anim = Data::battleranimations[rpg_actor->battler_animation - 1];
	const RPG::BattlerAnimationExtension& ext = anim.base_data[anim_state - 1];
	if (ext.battler_name == sprite_file)
		return;

	sprite_file = ext.battler_name;
	sprite->SetBitmap(Cache::BattleCharset(sprite_file));
}

void Scene_Battle::Ally::UpdateAnim(int cycle) {
	if (Player::engine != Player::EngineRpg2k3)
		return;

	static const int frames[] = {0,1,2,1};
	int frame = frames[(cycle / 15) % 4];
	if (frame == sprite_frame)
		return;

	const RPG::BattlerAnimation& anim = Data::battleranimations[rpg_actor->battler_animation - 1];
	const RPG::BattlerAnimationExtension& ext = anim.base_data[anim_state - 1];

	sprite->SetSrcRect(Rect(frame * 48, ext.battler_index * 48, 48, 48));
}

////////////////////////////////////////////////////////////
Scene_Battle::FloatText::FloatText(int x, int y, int color, const std::string& text, int _duration) {
	Rect rect = Surface::GetTextSize(text);

	Surface* graphic = Surface::CreateSurface(rect.width, rect.height);
	graphic->Clear();
	graphic->TextDraw(-rect.x, -rect.y, color, text);

	sprite = new Sprite();
	sprite->SetBitmap(graphic);
	sprite->SetOx(rect.width / 2);
	sprite->SetOy(rect.height + 5);
	sprite->SetX(x);
	sprite->SetY(y);
	sprite->SetZ(500+y);

	duration = _duration;
}

////////////////////////////////////////////////////////////
Scene_Battle::FloatText::~FloatText() {
	delete sprite;
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

	options_window = new Window_BattleOption(0, 172, 76, 68);

	status_window = new Window_BattleStatus();

	command_window = new Window_BattleCommand(244, 172, 76, 68);

	skill_window = new Window_BattleSkill(0, 172, 320, 68);
	skill_window->SetVisible(false);

	item_window = new Window_BattleItem(0, 172, 320, 68);
	item_window->SetVisible(false);
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
	active_ally = -1;
	auto_battle = false;
	action_timer = 0;

	if (!Game_Temp::battle_background.empty())
		background = new Background(Game_Temp::battle_background);
	else
		background = new Background(Game_Temp::battle_terrain_id);

	troop = &Data::troops[Game_Temp::battle_troop_id - 1];

	std::vector<RPG::TroopMember>::const_iterator ei;
	for (ei = troop->members.begin(); ei != troop->members.end(); ei++)
		enemies.push_back(Enemy(&*ei, ei - troop->members.begin()));

	const std::vector<Game_Actor*>& actors = Game_Party::GetActors();
	std::vector<Game_Actor*>::const_iterator ai;
	for (ai = actors.begin(); ai != actors.end(); ai++)
		allies.push_back(Ally(*ai, ai - actors.begin()));

	CreateSprites();
	CreateCursors();
	CreateWindows();

	int gauge = Game_Temp::battle_first_strike ? Ally::gauge_full : 0;
	std::fill(status_window->gauges, status_window->gauges+4, gauge);
	std::vector<Ally>::iterator it;
	for (it = allies.begin(); it != allies.end(); it++)
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
	if (state == State_Battle && auto_battle)
		state = State_AutoBattle;

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
			command_window->SetActor(GetActiveActor());
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
			skill_window->SetIndex(0);
			break;
		case State_AllyAttack:
		case State_AllyItem:
		case State_AllySkill:
		case State_Victory:
		case State_Defeat:
			break;
	}

	options_window->SetVisible(false);
	status_window->SetVisible(false);
	command_window->SetVisible(false);
	item_window->SetVisible(false);
	skill_window->SetVisible(false);
	help_window->SetVisible(false);

	item_window->SetHelpWindow(NULL);
	skill_window->SetHelpWindow(NULL);

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
		case State_AllyAttack:
		case State_AllyItem:
		case State_AllySkill:
			status_window->SetVisible(true);
			status_window->SetX(0);
			command_window->SetVisible(true);
			break;
		case State_Item:
			item_window->SetVisible(true);
			item_window->SetHelpWindow(help_window);
			help_window->SetVisible(true);
			break;
		case State_Skill:
			skill_window->SetVisible(true);
			skill_window->SetHelpWindow(help_window);
			help_window->SetVisible(true);
			break;
		case State_Victory:
		case State_Defeat:
			status_window->SetVisible(true);
			status_window->SetX(0);
			command_window->SetVisible(true);
			help_window->SetVisible(true);
			break;
	}
}

////////////////////////////////////////////////////////////
void Scene_Battle::Message(const std::string& msg) {
	help_window->SetText(msg);
	help_window->SetVisible(true);
	help_window->SetPause(true);
}

////////////////////////////////////////////////////////////
void Scene_Battle::Floater(const Sprite* ref, int color, const std::string& text, int duration) {
	int x = ref->GetX();
	int y = ref->GetY() - ref->GetOy();
	FloatText* floater = new FloatText(x, y, color, text, duration);
	floaters.push_back(floater);
}

////////////////////////////////////////////////////////////
void Scene_Battle::Floater(const Sprite* ref, int color, int value, int duration) {
	std::ostringstream out;
	out << value;
	Floater(ref, color, out.str(), duration);
}

////////////////////////////////////////////////////////////
void Scene_Battle::UpdateAnimState(Ally& ally, int default_state) {
	int anim_state = default_state;
	const RPG::State* state = ally.game_actor->GetState();
	if (state)
		anim_state = state->battler_animation_id == 100
			? 7
			: state->battler_animation_id + 1;
	ally.SetAnimState(anim_state);
}

////////////////////////////////////////////////////////////
void Scene_Battle::Restart(Ally& ally, int anim_state) {
	UpdateAnimState(ally, anim_state);
	ally.gauge = 0;
	status_window->SetTimeGauge(ally.ID, ally.gauge, Ally::gauge_full);
	if (state != State_AutoBattle)
		SetState(State_Battle);
	target_ally = -1;
	target_enemy = -1;
}

////////////////////////////////////////////////////////////
void Scene_Battle::Command() {
	RPG::BattleCommand command = command_window->GetCommand();
	switch (command.type) {
		case RPG::BattleCommand::Type_attack:
			target_enemy = 0;
			SetState(State_TargetEnemy);
			break;
		case RPG::BattleCommand::Type_skill:
			SetState(State_Skill);
			skill_window->SetSubset(RPG::Skill::Type_normal);
			break;
		case RPG::BattleCommand::Type_subskill:
		{
			int subset = command_window->GetSkillSubset();
			SetState(State_Skill);
			skill_window->SetSubset(subset);
		}
			break;
		case RPG::BattleCommand::Type_defense:
			Defend();
			break;
		case RPG::BattleCommand::Type_item:
			SetState(State_Item);
			break;
		case RPG::BattleCommand::Type_escape:
			Escape();
			break;
		case RPG::BattleCommand::Type_special:
			Special();
			break;
	}
}

void Scene_Battle::Escape() {
	if (Game_Temp::battle_escape_mode != 0) {
		// FIXME: escape probability
		Game_Temp::battle_result = Game_Temp::BattleEscape;
		Scene::Pop();
		return;
	}

	Ally& ally = allies[active_ally];
	Restart(ally);
}

void Scene_Battle::Special() {
	// FIXME: special commands (link to event)

	Ally& ally = allies[active_ally];
	Restart(ally);
}

void Scene_Battle::Defend() {
	Ally& ally = allies[active_ally];
	Restart(ally, Ally::Defending);
}

void Scene_Battle::Item() {
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
				ItemSkill(ally, item);
			else
				// can't be used
				Game_System::SePlay(Data::system.buzzer_se);
			break;
		case RPG::Item::Type_medicine:
			if (item.entire_party)
				SetState(State_AllyItem);
			else {
				target_ally = active_ally;
				SetState(State_TargetAlly);
			}
			break;
		case RPG::Item::Type_book:
		case RPG::Item::Type_material:
			// can't be used in battle?
			Game_System::SePlay(Data::system.buzzer_se);
			break;
		case RPG::Item::Type_special:
			ItemSkill(ally, item);
			break;
		case RPG::Item::Type_switch:
			Game_Switches[item.switch_id] = true;
			Restart(ally);
			break;
	}
}

void Scene_Battle::Skill() {
	Ally& ally = allies[active_ally];

	int skill_id = skill_window->GetSkillId();
	if (skill_id <= 0) {
		Game_System::SePlay(Data::system.buzzer_se);
		return;
	}

	const RPG::Skill& skill = Data::skills[skill_id - 1];

	Skill(ally, skill);
}

void Scene_Battle::ItemSkill(Ally& ally, const RPG::Item& item) {
	const RPG::Skill& skill = Data::skills[item.skill_id - 1];
	Skill(ally, skill);
}

void Scene_Battle::Skill(Ally& ally, const RPG::Skill& skill) {
	switch (skill.type) {
		case RPG::Skill::Type_teleport:
		case RPG::Skill::Type_escape:
		case RPG::Skill::Type_switch:
			SetState(State_AllySkill);
			return;
		case RPG::Skill::Type_normal:
		default:
			break;
	}

	switch (skill.scope) {
		case RPG::Skill::Scope_enemy:
			target_enemy = 0;
			SetState(State_TargetEnemy);
			return;
		case RPG::Skill::Scope_enemies:
			SetState(State_AllySkill);
			break;
		case RPG::Skill::Scope_self:
			SetState(State_AllySkill);
			break;
		case RPG::Skill::Scope_ally:
			target_ally = active_ally;
			SetState(State_TargetAlly);
			return;
		case RPG::Skill::Scope_party:
			SetState(State_AllySkill);
			break;
	}
}

void Scene_Battle::TargetDone() {
	Ally& ally = allies[active_ally];

	switch (target_state) {
		case State_Command:
			// FIXME
			ally.SetAnimState(Ally::RightHand);
			action_timer = 60;
			SetState(State_AllyAttack);
			break;
		case State_Item:
			ally.SetAnimState(Ally::Item);
			action_timer = 60;
			SetState(State_AllyItem);
			break;
		case State_Skill:
			ally.SetAnimState(Ally::SkillUse);
			action_timer = 60;
			SetState(State_AllySkill);
			break;
		default:
			break;
	}
}

void Scene_Battle::ActionDone() {
	switch (state) {
		case State_AllyAttack:
			AttackDone();
			break;
		case State_AllyItem:
			ItemDone();
			break;
		case State_AllySkill:
			SkillDone();
			break;
		default:
			break;
	}
}

void Scene_Battle::AttackDone() {
	Ally& ally = allies[active_ally];
	Enemy& enemy = enemies[target_enemy];

	const RPG::Item& weapon = Data::items[ally.game_actor->GetWeaponId() - 1];
	double to_hit = 100 - (100 - weapon.hit) * (1 + (1.0 * enemy.game_enemy->GetAgi() / ally.game_actor->GetAgi() - 1) / 2);

	if (rand() % 100 < to_hit) {
		int effect = ally.game_actor->GetAtk() / 2 - enemy.game_enemy->GetDef() / 4;
		if (effect < 0)
			effect = 0;
		int act_perc = (rand() % 40) - 20;
		int change = effect * act_perc / 100;
		effect += change;

		enemy.game_enemy->SetHp(enemy.game_enemy->GetHp() - effect);
		Floater(enemy.sprite, Font::ColorDefault, effect, 60);
	}
	else
		Floater(enemy.sprite, Font::ColorDefault, Data::terms.miss, 60);

	Restart(ally);
}

void Scene_Battle::ItemDone() {
	Ally& ally = allies[active_ally];
	int item_id = item_window->GetItemId();
	const RPG::Item& item = Data::items[item_id - 1];
	Ally* ally_target = target_ally >= 0 ? &allies[target_ally] : NULL;

	UseItem(ally, item, ally_target);
	Restart(ally);
}

void Scene_Battle::SkillDone() {
	Ally& ally = allies[active_ally];
	int skill_id = skill_window->GetSkillId();
	const RPG::Skill& skill = Data::skills[skill_id - 1];
	Ally* ally_target = target_ally >= 0 ? &allies[target_ally] : NULL;
	Enemy* enemy_target = target_enemy >= 0 ? &enemies[target_enemy] : NULL;

	UseSkill(ally, skill, ally_target, enemy_target);

	Restart(ally);
}

void Scene_Battle::UseItem(Ally& ally, const RPG::Item& item, Ally* target_ally) {
	if (item.type != RPG::Item::Type_medicine)
		return;
	if (item.ocassion_field)
		return;

	if (!item.entire_party)
		UseItemAlly(ally, item, target_ally);
	else
		for (std::vector<Ally>::iterator it = allies.begin(); it != allies.end(); it++)
			UseItemAlly(ally, item, &*it);

	switch (item.uses) {
		case 0:
			// unlimited uses
			break;
		case 1:
			// single use
			Game_Party::LoseItem(item.ID, 1, false);
			item_window->Refresh();
			break;
		default:
			// multiple use
			// FIXME: we need a Game_Item type to hold the usage count
			break;
	}
}

void Scene_Battle::UseItemAlly(Ally& ally, const RPG::Item& item, Ally* target) {
	if (item.ko_only && !target->game_actor->IsDead())
		return;

	// HP recovery
	int hp = item.recover_hp_rate * target->game_actor->GetMaxHp() / 100 + item.recover_hp;
	target->game_actor->SetHp(target->game_actor->GetHp() + hp);

	// SP recovery
	int sp = item.recover_sp_rate * target->game_actor->GetMaxSp() / 100 + item.recover_sp;
	target->game_actor->SetSp(target->game_actor->GetSp() + sp);

	if (hp > 0)
		Floater(target->sprite, 9, hp, 60);
	else if (sp > 0)
		Floater(target->sprite, 9, sp, 60);

	// Status recovery
	for (int i = 0; i < (int) item.state_set.size(); i++)
		if (item.state_set[i])
			target->game_actor->RemoveState(i + 1);
}

void Scene_Battle::UseSkill(Ally& ally, const RPG::Skill& skill,
							Ally* target_ally, Enemy* target_enemy) {
	int sp = ally.game_actor->CalculateSkillCost(skill.ID);
	if (sp > ally.game_actor->GetSp()) // not enough SP
		return;

	switch (skill.type) {
		case RPG::Skill::Type_teleport:
			// FIXME: teleport skill
			break;
		case RPG::Skill::Type_escape:
			Escape();
			break;
		case RPG::Skill::Type_switch:
			if (!skill.occasion_battle)
				return;
			Game_Switches[skill.switch_id] = true;
			break;
		case RPG::Skill::Type_normal:
		default:
			switch (skill.scope) {
				case RPG::Skill::Scope_enemy:
					UseSkillEnemy(ally, skill, target_enemy);
					return;
				case RPG::Skill::Scope_enemies:
					for (std::vector<Enemy>::iterator it = enemies.begin(); it != enemies.end(); it++)
						UseSkillEnemy(ally, skill, &*it);
					break;
				case RPG::Skill::Scope_self:
					UseSkillAlly(ally, skill, &ally);
					break;
				case RPG::Skill::Scope_ally:
					UseSkillAlly(ally, skill, target_ally);
					return;
				case RPG::Skill::Scope_party:
					for (std::vector<Ally>::iterator it = allies.begin(); it != allies.end(); it++)
						UseSkillAlly(ally, skill, &*it);
					break;
			}
			break;
	}

	if (skill.type != RPG::Skill::Type_normal)
		Game_System::SePlay(skill.sound_effect);

	ally.game_actor->SetSp(ally.game_actor->GetSp() - sp);
}

void Scene_Battle::UseSkillAlly(Ally& ally, const RPG::Skill& skill, Ally* target) {
	Game_Actor* actor = target->game_actor;
	bool miss = true;

	if (skill.power > 0) {
		if (rand() % 100 < skill.hit) {
			miss = false;

			// FIXME: is this still affected by stats for allies?
			int effect = skill.power;

			if (skill.variance > 0) {
				int var_perc = skill.variance * 5;
				int act_perc = rand() % (var_perc * 2) - var_perc;
				int change = effect * act_perc / 100;
				effect += change;
			}

			if (skill.affect_hp)
				actor->SetHp(actor->GetHp() + effect);
			if (skill.affect_sp)
				actor->SetSp(actor->GetSp() + effect);
			if (skill.affect_attack)
				actor->SetAtk(actor->GetAtk() + effect);
			if (skill.affect_defense)
				actor->SetDef(actor->GetDef() + effect);
			if (skill.affect_spirit)
				actor->SetSpi(actor->GetSpi() + effect);
			if (skill.affect_agility)
				actor->SetAgi(actor->GetAgi() + effect);

			if (skill.affect_hp || skill.affect_sp)
				Floater(target->sprite, 9, effect, 60);
		}
	}

	for (int i = 0; i < (int) skill.state_effects.size(); i++) {
		if (!skill.state_effects[i])
			continue;
		if (rand() % 100 >= skill.hit)
			continue;

		miss = false;

		if (skill.state_effect)
			actor->AddState(i + 1);
		else
			actor->RemoveState(i + 1);
	}

	if (miss)
		Floater(target->sprite, Font::ColorDefault, Data::terms.miss, 60);
}

void Scene_Battle::UseSkillEnemy(Ally& ally, const RPG::Skill& skill, Enemy* target) {
	Game_Enemy* enemy = target->game_enemy;
	bool miss = true;

	if (skill.power > 0) {
		if (rand() % 100 < skill.hit) {
			miss = false;

			// FIXME: This is what the help file says, but it doesn't look right
			int effect = skill.power +
				ally.game_actor->GetAtk() * skill.pdef_f / 20 + 
				enemy->GetDef() * skill.mdef_f / 40;

			if (skill.variance > 0) {
				int var_perc = skill.variance * 5;
				int act_perc = rand() % (var_perc * 2) - var_perc;
				int change = effect * act_perc / 100;
				effect += change;
			}

			if (skill.affect_hp)
				enemy->SetHp(enemy->GetHp() - effect);
			if (skill.affect_sp)
				enemy->SetSp(enemy->GetSp() - effect);
			if (skill.affect_attack)
				enemy->SetAtk(enemy->GetAtk() - effect);
			if (skill.affect_defense)
				enemy->SetDef(enemy->GetDef() - effect);
			if (skill.affect_spirit)
				enemy->SetSpi(enemy->GetSpi() - effect);
			if (skill.affect_agility)
				enemy->SetAgi(enemy->GetAgi() - effect);

			if (skill.affect_hp || skill.affect_sp)
				Floater(target->sprite, Font::ColorDefault, effect, 60);
		}
	}

	for (int i = 0; i < (int) skill.state_effects.size(); i++) {
		if (!skill.state_effects[i])
			continue;
		if (rand() % 100 >= skill.hit)
			continue;

		miss = false;

		if (skill.state_effect)
			enemy->RemoveState(i + 1);
		else
			enemy->AddState(i + 1);
	}

	if (miss)
		Floater(target->sprite, Font::ColorDefault, Data::terms.miss, 60);
}

////////////////////////////////////////////////////////////
void Scene_Battle::ProcessActions() {
	switch (state) {
		case State_AllyAttack:
			action_timer--;
			if (action_timer <= 0)
				AttackDone();
			break;
		case State_AllyItem:
			action_timer--;
			if (action_timer <= 0)
				ItemDone();
			break;
		case State_AllySkill:
			action_timer--;
			if (action_timer <= 0)
				SkillDone();
			break;
		default:
			break;
	}

	for (std::vector<Ally>::iterator it = allies.begin(); it != allies.end(); it++) {
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
void Scene_Battle::ProcessInput() {
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
						Escape();
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
				Command();
				break;
			case State_TargetEnemy:
			case State_TargetAlly:
				TargetDone();
				break;
			case State_Item:
				Item();
				break;
			case State_Skill:
				Skill();
				break;
			case State_AllyAttack:
			case State_AllyItem:
			case State_AllySkill:
				break;
			case State_Victory:
			case State_Defeat:
				Scene::Pop();
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
			case State_AllyAttack:
			case State_AllyItem:
			case State_AllySkill:
				break;
			case State_Victory:
			case State_Defeat:
				Scene::Pop();
				break;
		}
	}

	if (state == State_TargetEnemy && target_enemy >= 0) {
		if (Input::IsRepeated(Input::DOWN))
			target_enemy++;
		if (Input::IsRepeated(Input::UP))
			target_enemy--;

		target_enemy += enemies.size();
		target_enemy %= enemies.size();

		ChooseEnemy();
	}

	if (state == State_TargetAlly && target_ally >= 0) {
		if (Input::IsRepeated(Input::DOWN))
			target_ally++;
		if (Input::IsRepeated(Input::UP))
			target_ally--;

		target_ally += allies.size();
		target_ally %= allies.size();
	}
}

////////////////////////////////////////////////////////////
void Scene_Battle::ChooseEnemy() {
	if (target_enemy < 0)
		target_enemy = 0;

	if (enemies[target_enemy].game_enemy->Exists())
		return;

	for (int i = 1; i < (int) enemies.size(); i++) {
		target_enemy++;
		if (enemies[target_enemy].game_enemy->Exists())
			break;
	}

	if (!enemies[target_enemy].game_enemy->Exists())
		target_enemy = -1;
}

////////////////////////////////////////////////////////////
void Scene_Battle::DoAuto() {
	if (state != State_AutoBattle)
		return;

	active_ally = status_window->GetActiveCharacter();
	if (active_ally < 0)
		return;

	ChooseEnemy();
	if (target_enemy < 0)
		return;

	AttackDone();
}

////////////////////////////////////////////////////////////
void Scene_Battle::UpdateCursors() {
	int selected_ally = target_ally;
	if (selected_ally < 0)
		selected_ally = status_window->GetActiveCharacter();
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
}

////////////////////////////////////////////////////////////
void Scene_Battle::UpdateAttack() {
	const Ally& ally = allies[active_ally];
	const Enemy& enemy = enemies[target_enemy];
	double k =
		(action_timer > 48) ? (60 - action_timer) / 12.0 :
		(action_timer < 12) ? action_timer / 12.0 :
		1.0;
	int x0 = ally.rpg_actor->battle_x;
	int x1 = enemy.sprite->GetX() - enemy.sprite->GetOx() + enemy.sprite->GetWidth() +
		ally.sprite->GetOx();
	ally.sprite->SetX((int)(x0 * (1 - k) + x1 * k));
}

////////////////////////////////////////////////////////////
void Scene_Battle::UpdateSprites() {
	for (std::vector<Enemy>::iterator it = enemies.begin(); it != enemies.end(); it++) {
		if (it->sprite->GetVisible() && !it->game_enemy->Exists() && it->fade == 0)
			it->fade = 60;

		if (it->fade > 0) {
			it->sprite->SetOpacity(it->fade * 255 / 60);
			it->fade--;
			if (it->fade == 0)
				it->sprite->SetVisible(false);
		}
			
		if (!it->rpg_enemy->levitate)
			continue;
		int y = (int) (3 * sin(cycle / 30.0));
		it->sprite->SetY(it->member->y + y);
		it->sprite->SetZ(it->member->y + y);
	}

	for (std::vector<Ally>::iterator it = allies.begin(); it != allies.end(); it++)
		it->UpdateAnim(cycle);

	if (state == State_AllyAttack)
		UpdateAttack();
}

////////////////////////////////////////////////////////////
void Scene_Battle::UpdateFloaters() {
	std::vector<FloatText*>::const_iterator it;
	std::vector<FloatText*>::iterator dst = floaters.begin();
	for (it = floaters.begin(); it != floaters.end(); it++) {
		FloatText* floater = *it;
		floater->duration--;
		if (floater->duration <= 0)
			delete floater;
		else
			*dst++ = floater;
	}

	floaters.erase(dst, floaters.end());
}

////////////////////////////////////////////////////////////
void Scene_Battle::Update() {
	options_window->Update();
	status_window->Update();
	command_window->Update();
	help_window->Update();
	item_window->Update();
	skill_window->Update();

	if (state == State_Battle)
		command_window->SetActor(GetTargetActor());

	cycle++;

	CheckWin();
	CheckLose();

	ProcessActions();
	ProcessInput();
	DoAuto();
	UpdateCursors();
	UpdateSprites();
	UpdateFloaters();
}

////////////////////////////////////////////////////////////
int Scene_Battle::GetActiveActor() {
	if (active_ally < 0)
		return active_ally;
	
	const std::vector<Game_Actor*>& actors = Game_Party::GetActors();
	return actors[active_ally]->GetId();
}

////////////////////////////////////////////////////////////
int Scene_Battle::GetTargetActor() {
	const std::vector<Game_Actor*>& actors = Game_Party::GetActors();
	return actors[std::max(target_ally, 0)]->GetId();
}

////////////////////////////////////////////////////////////
bool Scene_Battle::HaveCorpse() {
	for (std::vector<Ally>::iterator it = allies.begin(); it != allies.end(); it++)
		if (it->game_actor->IsDead())
			return true;
	return false;
}

////////////////////////////////////////////////////////////
void Scene_Battle::CheckWin() {
	if (state == State_Victory || state == State_Defeat)
		return;

	for (std::vector<Enemy>::iterator it = enemies.begin(); it != enemies.end(); it++)
		if (!it->game_enemy->IsDead())
			return;
	for (std::vector<Ally>::iterator it = allies.begin(); it != allies.end(); it++)
		it->SetAnimState(Ally::Victory);
	Game_Temp::battle_result = Game_Temp::BattleVictory;
	SetState(State_Victory);
	Message(Data::terms.victory.empty() ? Data::terms.victory : "Victory");
}

////////////////////////////////////////////////////////////
void Scene_Battle::CheckLose() {
	if (state == State_Victory || state == State_Defeat)
		return;

	for (std::vector<Ally>::iterator it = allies.begin(); it != allies.end(); it++)
		if (!it->game_actor->IsDead())
			return;
	Game_Temp::battle_result = Game_Temp::BattleDefeat;
	SetState(State_Defeat);
	Message(!Data::terms.defeat.empty() ? Data::terms.defeat : "Defeat");
}

