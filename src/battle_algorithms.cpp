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

// Headers
#include <cstdlib>
#include "rpg_item.h"
#include "rpg_skill.h"
#include "rpg_enemyaction.h"
#include "game_system.h"
#include "game_party.h"
#include "game_switches.h"
#include "game_battle.h"
#include "font.h"

void Game_Battle::AttackEnemy(Battle::Ally& ally, Battle::Enemy& enemy) {
	const RPG::Item& weapon = Data::items[ally.game_actor->GetWeaponId() - 1];
	double to_hit = 100 - (100 - weapon.hit) * (1 + (1.0 * enemy.GetAgi() / ally.GetAgi() - 1) / 2);

	if (rand() % 100 < to_hit) {
		int effect = ally.GetAtk() / 2 - enemy.GetDef() / 4;
		if (effect < 0)
			effect = 0;
		int act_perc = (rand() % 40) - 20;
		int change = effect * act_perc / 100;
		effect += change;

		enemy.game_enemy->SetHp(enemy.game_enemy->GetHp() - effect);
		//GetScene()->Floater(enemy.sprite.get(), Font::ColorDefault, effect, 60);
	}
	//else
	//	GetScene()->Floater(enemy.sprite.get(), Font::ColorDefault, Data::terms.miss, 60);
}

void Game_Battle::UseItem(Battle::Ally& ally, const RPG::Item& item) {
	if (item.type != RPG::Item::Type_medicine)
		return;
	if (item.occasion_field2)
		return;

	if (!item.entire_party)
		UseItemAlly(ally, item, Game_Battle::GetTargetAlly());
	else
		for (std::vector<Battle::Ally>::iterator it = Game_Battle::allies.begin(); it != Game_Battle::allies.end(); it++)
			UseItemAlly(ally, item, *it);

	switch (item.uses) {
		case 0:
			// unlimited uses
			break;
		case 1:
			// single use
			Main_Data::game_party->LoseItem(item.ID, 1);
			break;
		default:
			// multiple use
			// FIXME: we need a Game_Item type to hold the usage count
			break;
	}
}

void Game_Battle::UseItemAlly(Battle::Ally& /* ally */, const RPG::Item& item, Battle::Ally& target) {
	if (item.ko_only && !target.GetActor()->IsDead())
		return;

	// HP recovery
	int hp = item.recover_hp_rate * target.GetActor()->GetMaxHp() / 100 + item.recover_hp;
	target.GetActor()->SetHp(target.GetActor()->GetHp() + hp);

	// SP recovery
	int sp = item.recover_sp_rate * target.GetActor()->GetMaxSp() / 100 + item.recover_sp;
	target.GetActor()->SetSp(target.GetActor()->GetSp() + sp);

	/*if (hp > 0)
		GetScene()->Floater(target.sprite.get(), 9, hp, 60);
	else if (sp > 0)
		GetScene()->Floater(target.sprite.get(), 9, sp, 60);*/

	// Status recovery
	for (int i = 0; i < (int) item.state_set.size(); i++)
		if (item.state_set[i])
			target.GetActor()->RemoveState(i + 1);
}

void Game_Battle::UseSkill(Battle::Ally& ally, const RPG::Skill& skill) {

	int sp = ally.GetActor()->CalculateSkillCost(skill.ID);
	if (sp > ally.GetActor()->GetSp()) // not enough SP
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
					UseSkillEnemy(ally, skill, Game_Battle::GetTargetEnemy());
					return;
				case RPG::Skill::Scope_enemies:
					for (std::vector<Battle::Enemy>::iterator it = Game_Battle::enemies.begin(); it != Game_Battle::enemies.end(); it++)
						UseSkillEnemy(ally, skill, *it);
					break;
				case RPG::Skill::Scope_self:
					UseSkillAlly(ally, skill, ally);
					break;
				case RPG::Skill::Scope_ally:
					UseSkillAlly(ally, skill, Game_Battle::GetTargetAlly());
					return;
				case RPG::Skill::Scope_party:
					for (std::vector<Battle::Ally>::iterator it = Game_Battle::allies.begin(); it != Game_Battle::allies.end(); it++)
						UseSkillAlly(ally, skill, *it);
					break;
			}
			break;
	}

	if (skill.type != RPG::Skill::Type_normal)
		Game_System::SePlay(skill.sound_effect);

	ally.GetActor()->SetSp(ally.GetActor()->GetSp() - sp);
}

void Game_Battle::UseSkillAlly(Battle::Battler& /* user */, const RPG::Skill& skill, Battle::Battler& target) {
	Game_Battler* actor = target.GetActor();
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
				target.ModifyAtk(effect);
			if (skill.affect_defense)
				target.ModifyDef(effect);
			if (skill.affect_spirit)
				target.ModifySpi(effect);
			if (skill.affect_agility)
				target.ModifyAgi(effect);

			/*if (skill.affect_hp || skill.affect_sp)
				GetScene()->Floater(target.sprite.get(), 9, effect, 60);*/
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

	//if (miss)
	//	GetScene()->Floater(target.sprite.get(), Font::ColorDefault, Data::terms.miss, 60);
}

void Game_Battle::UseSkillEnemy(Battle::Battler& user, const RPG::Skill& skill, Battle::Battler& target) {
	Game_Battler* actor = target.GetActor();
	bool miss = true;

	if (skill.power > 0) {
		if (rand() % 100 < skill.hit) {
			miss = false;

			// FIXME: This is what the help file says, but it doesn't look right
			int effect = skill.power +
				user.GetAtk() * skill.pdef_f / 20 +
				actor->GetDef() * skill.mdef_f / 40;

			if (skill.variance > 0) {
				int var_perc = skill.variance * 5;
				int act_perc = rand() % (var_perc * 2) - var_perc;
				int change = effect * act_perc / 100;
				effect += change;
			}

			if (skill.affect_hp)
				actor->SetHp(actor->GetHp() - effect);
			if (skill.affect_sp)
				actor->SetSp(actor->GetSp() - effect);
			if (skill.affect_attack)
				target.ModifyAtk(-effect);
			if (skill.affect_defense)
				target.ModifyDef(-effect);
			if (skill.affect_spirit)
				target.ModifySpi(-effect);
			if (skill.affect_agility)
				target.ModifyAgi(-effect);

			//if (skill.affect_hp || skill.affect_sp)
			//	GetScene()->Floater(target.sprite.get(), Font::ColorDefault, effect, 60);
		}
	}

	for (int i = 0; i < (int) skill.state_effects.size(); i++) {
		if (!skill.state_effects[i])
			continue;
		if (rand() % 100 >= skill.hit)
			continue;

		miss = false;

		if (skill.state_effect)
			actor->RemoveState(i + 1);
		else
			actor->AddState(i + 1);
	}

	///if (miss)
	//	GetScene()->Floater(target.sprite.get(), Font::ColorDefault, Data::terms.miss, 60);
}

void Game_Battle::EnemyAttackAlly(Battle::Enemy& enemy, Battle::Ally& ally) {
	if (ally.GetActor()->IsDead())
		return;

	int hit = enemy.rpg_enemy->miss ? 70 : 90;
	double to_hit = 100 - (100 - hit) * (1 + (1.0 * ally.GetAgi() / enemy.GetAgi() - 1) / 2);

	if (rand() % 100 < to_hit) {
		int effect = enemy.GetAtk() / 2 - ally.GetDef() / 4;
		if (effect < 0)
			effect = 0;
		int act_perc = (rand() % 40) - 20;
		int change = effect * act_perc / 100;
		effect += change;

		ally.GetActor()->SetHp(ally.GetActor()->GetHp() - effect);
		//GetScene()->Floater(ally.sprite.get(), Font::ColorDefault, effect, 60);
	}
	//else
	//	GetScene()->Floater(ally.sprite.get(), Font::ColorDefault, Data::terms.miss, 60);
}

void Game_Battle::EnemySkill(Battle::Enemy& enemy, const RPG::Skill& skill) {
	int sp = enemy.game_enemy->CalculateSkillCost(skill.ID);
	if (sp > enemy.game_enemy->GetSp()) // not enough SP
		return;

	switch (skill.type) {
		case RPG::Skill::Type_teleport:
			// FIXME: can monsters teleport?
			Game_Battle::EnemyEscape();
			break;
		case RPG::Skill::Type_escape:
			Game_Battle::EnemyEscape();
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
					UseSkillAlly(enemy, skill, Game_Battle::GetTargetAlly());
					return;
				case RPG::Skill::Scope_enemies:
					for (std::vector<Battle::Ally>::iterator it = Game_Battle::allies.begin(); it != Game_Battle::allies.end(); it++)
						UseSkillAlly(enemy, skill, *it);
					break;
				case RPG::Skill::Scope_self:
					UseSkillEnemy(enemy, skill, enemy);
					break;
				case RPG::Skill::Scope_ally:
					UseSkillEnemy(enemy, skill, Game_Battle::GetTargetEnemy());
					return;
				case RPG::Skill::Scope_party:
					for (std::vector<Battle::Enemy>::iterator it = Game_Battle::enemies.begin(); it != Game_Battle::enemies.end(); it++)
						UseSkillEnemy(enemy, skill, *it);
					break;
			}
			break;
	}

	if (skill.type != RPG::Skill::Type_normal)
		Game_System::SePlay(skill.sound_effect);

	enemy.game_enemy->SetSp(enemy.game_enemy->GetSp() - sp);
}
