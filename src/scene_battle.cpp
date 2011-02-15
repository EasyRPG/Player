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
#include "graphics.h"
#include "filefinder.h"
#include "cache.h"
#include "game_system.h"
#include "game_temp.h"
#include "game_party.h"
#include "game_enemy.h"
#include "game_switches.h"
#include "game_battle.h"
#include "battle_battler.h"
#include "battle_animation.h"
#include "battle_actions.h"
#include "scene_battle.h"

////////////////////////////////////////////////////////////
Scene_Battle::Scene_Battle() {
	Scene::type = Scene::Battle;
}

////////////////////////////////////////////////////////////
Scene_Battle::~Scene_Battle() {
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
		else
			Game_Temp::battle_troop_id = Player::battle_test_troop_id;
	}

	Game_Battle::Init();

	cycle = 0;
	auto_battle = false;
	enemy_action = NULL;

	CreateCursors();
	CreateWindows();

	SetState(State_Options);
}

////////////////////////////////////////////////////////////
void Scene_Battle::Terminate() {
	delete help_window;
	delete options_window;
	delete status_window;
	delete command_window;
	delete item_window;
	delete skill_window;

	Game_Battle::Quit();
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
			command_window->SetActor(Game_Battle::GetActiveActor());
			break;
		case State_TargetEnemy:
			break;
		case State_TargetAlly:
			status_window->SetActive(true);
			break;
		case State_Item:
			item_window->SetActive(true);
			item_window->SetActor(Game_Battle::GetActiveActor());
			item_window->Refresh();
			break;
		case State_Skill:
			skill_window->SetActive(true);
			skill_window->SetActor(Game_Battle::GetActiveActor());
			skill_window->SetIndex(0);
			break;
		case State_AllyAction:
		case State_EnemyAction:
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
		case State_AllyAction:
		case State_EnemyAction:
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
void Scene_Battle::Message(const std::string& msg, bool pause) {
	help_window->SetText(msg);
	help_window->SetVisible(true);
	help_window->SetPause(pause);
	if (!pause)
		message_timer = 60;
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
void Scene_Battle::SetAnimState(Battle::Ally& ally, int state) {
	ally.SetAnimState(state);
	ally.UpdateAnim(cycle);
}

////////////////////////////////////////////////////////////
void Scene_Battle::UpdateAnimState(Battle::Ally& ally, int default_state) {
	int anim_state = default_state;
	const RPG::State* state = ally.GetActor()->GetState();
	if (state)
		anim_state = state->battler_animation_id == 100
			? 7
			: state->battler_animation_id + 1;
	SetAnimState(ally, anim_state);
}

////////////////////////////////////////////////////////////
void Scene_Battle::Restart(Battle::Ally& ally, int anim_state) {
	UpdateAnimState(ally, anim_state);
	ally.gauge = 0;
	SetState(State_Battle);
	Game_Battle::ClearTargetAlly();
	Game_Battle::ClearTargetEnemy();
	ally.defending = false;
	ally.last_command = pending_command;
}

////////////////////////////////////////////////////////////
void Scene_Battle::Restart() {
	Restart(Game_Battle::GetActiveAlly());
}

////////////////////////////////////////////////////////////
void Scene_Battle::Command() {
	RPG::BattleCommand command = command_window->GetCommand();
	pending_command = command.ID;

	switch (command.type) {
		case RPG::BattleCommand::Type_attack:
			Game_Battle::SetTargetEnemy(0);
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

////////////////////////////////////////////////////////////
void Scene_Battle::Escape() {
	if (Game_Temp::battle_escape_mode != 0) {
		// FIXME: escape probability
		Game_Temp::battle_result = Game_Temp::BattleEscape;
		Scene::Pop();
		return;
	}

	Restart();
}

////////////////////////////////////////////////////////////
void Scene_Battle::Special() {
	// FIXME: special commands (link to event)

	Restart();
}

////////////////////////////////////////////////////////////
void Scene_Battle::Defend() {
	Battle::Ally& ally = Game_Battle::GetActiveAlly();
	Restart(ally, Battle::Ally::Defending);
	ally.defending = true;
}

////////////////////////////////////////////////////////////
void Scene_Battle::Item() {
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
				ItemSkill(item);
			else
				// can't be used
				Game_System::SePlay(Data::system.buzzer_se);
			break;
		case RPG::Item::Type_medicine:
			if (item.entire_party)
				BeginItem();
			else {
				Game_Battle::TargetActiveAlly();
				SetState(State_TargetAlly);
			}
			break;
		case RPG::Item::Type_book:
		case RPG::Item::Type_material:
			// can't be used in battle?
			Game_System::SePlay(Data::system.buzzer_se);
			break;
		case RPG::Item::Type_special:
			ItemSkill(item);
			break;
		case RPG::Item::Type_switch:
			Game_Switches[item.switch_id] = true;
			Restart();
			break;
	}
}

////////////////////////////////////////////////////////////
void Scene_Battle::Skill() {
	int skill_id = skill_window->GetSkillId();
	if (skill_id <= 0) {
		Game_System::SePlay(Data::system.buzzer_se);
		return;
	}

	const RPG::Skill& skill = Data::skills[skill_id - 1];

	Skill(skill);
}

////////////////////////////////////////////////////////////
void Scene_Battle::ItemSkill(const RPG::Item& item) {
	const RPG::Skill& skill = Data::skills[item.skill_id - 1];
	Skill(skill);
}

////////////////////////////////////////////////////////////
void Scene_Battle::Skill(const RPG::Skill& skill) {
	skill_id = skill.ID;

	switch (skill.type) {
		case RPG::Skill::Type_teleport:
		case RPG::Skill::Type_escape:
		case RPG::Skill::Type_switch:
			BeginSkill();
			return;
		case RPG::Skill::Type_normal:
		default:
			break;
	}

	switch (skill.scope) {
		case RPG::Skill::Scope_enemy:
			Game_Battle::SetTargetEnemy(0);
			SetState(State_TargetEnemy);
			return;
		case RPG::Skill::Scope_ally:
			Game_Battle::TargetActiveAlly();
			SetState(State_TargetAlly);
			break;
		case RPG::Skill::Scope_enemies:
		case RPG::Skill::Scope_self:
		case RPG::Skill::Scope_party:
			BeginSkill();
			break;
	}
}

////////////////////////////////////////////////////////////
void Scene_Battle::TargetDone() {
	switch (target_state) {
		case State_Command:
			BeginAttack();
			break;
		case State_Item:
			BeginItem();
			break;
		case State_Skill:
			BeginSkill();
			break;
		default:
			break;
	}
}

////////////////////////////////////////////////////////////
void Scene_Battle::BeginAttack() {
	Battle::Ally& ally = Game_Battle::GetActiveAlly();
	Battle::Enemy& enemy = Game_Battle::GetTargetEnemy();
	int x0 = ally.rpg_actor->battle_x;
	int x1 = enemy.sprite->GetX() - enemy.sprite->GetOx() + enemy.sprite->GetWidth() +
		ally.sprite->GetOx();

	actions.clear();
	actions.push_back(new Battle::SpriteAction(&ally, Battle::Ally::WalkingLeft));
	actions.push_back(new Battle::MoveAction(ally.sprite, x0, x1, 8));
	actions.push_back(new Battle::SpriteAction(&ally, Battle::Ally::RightHand));
	actions.push_back(new Battle::WaitAction(15));
	actions.push_back(new Battle::CommandAction(&Scene_Battle::DoAttack));
	actions.push_back(new Battle::SpriteAction(&ally, Battle::Ally::WalkingRight));
	actions.push_back(new Battle::MoveAction(ally.sprite, x1, x0, 8));
	actions.push_back(new Battle::CommandAction(&Scene_Battle::Restart));
	actions.push_back(new Battle::WaitAction(20));

	SetState(State_AllyAction);
}

////////////////////////////////////////////////////////////
void Scene_Battle::BeginItem() {
	Battle::Ally& ally = Game_Battle::GetActiveAlly();

	actions.clear();
	actions.push_back(new Battle::SpriteAction(&ally, Battle::Ally::Item));
	actions.push_back(new Battle::WaitAction(60));
	actions.push_back(new Battle::CommandAction(&Scene_Battle::DoItem));
	actions.push_back(new Battle::CommandAction(&Scene_Battle::Restart));
	actions.push_back(new Battle::WaitAction(20));

	SetState(State_AllyAction);
}

////////////////////////////////////////////////////////////
void Scene_Battle::DoAttack() {
	Battle::Ally& ally = Game_Battle::GetActiveAlly();
	Battle::Enemy& enemy = Game_Battle::GetTargetEnemy();

	AttackEnemy(ally, enemy);
}

////////////////////////////////////////////////////////////
void Scene_Battle::BeginSkill() {
	Battle::Ally& ally = Game_Battle::GetActiveAlly();
	const RPG::Skill& skill = Data::skills[skill_id - 1];
	int anim_state = SkillAnimation(skill, ally);
	const RPG::Animation* animation = (skill.animation_id != 0)
		? &Data::animations[skill.animation_id - 1]
		: NULL;
	Battle::Action* action = NULL;
	int x, y;

	switch (skill.type) {
		case RPG::Skill::Type_teleport:
		case RPG::Skill::Type_escape:
		case RPG::Skill::Type_switch:
			break;
		case RPG::Skill::Type_normal:
		default:
			switch (skill.scope) {
				case RPG::Skill::Scope_enemy:
					if (animation != NULL)
						action = new Battle::AnimationAction(Game_Battle::GetTargetEnemy().sprite, animation);
					break;
				case RPG::Skill::Scope_enemies:
					if (animation != NULL) {
						Game_Battle::EnemiesCentroid(x, y);
						action = new Battle::AnimationAction(x, y, animation);
					}
					break;
				case RPG::Skill::Scope_self:
					if (animation != NULL)
						action = new Battle::AnimationAction(ally.sprite, animation);
					break;
				case RPG::Skill::Scope_ally:
					if (animation != NULL)
						action = new Battle::AnimationAction(Game_Battle::GetTargetAlly().sprite, animation);
					break;
				case RPG::Skill::Scope_party:
					if (animation != NULL) {
						Game_Battle::AlliesCentroid(x, y);
						action = new Battle::AnimationAction(x, y, animation);
					}
				default:
					break;
			}
	}

	if (action == NULL)
		action = new Battle::WaitAction(10);

	actions.clear();
	actions.push_back(new Battle::SpriteAction(&ally, anim_state));
	actions.push_back(action);
	actions.push_back(new Battle::CommandAction(&Scene_Battle::DoSkill));
	actions.push_back(new Battle::CommandAction(&Scene_Battle::Restart));
	actions.push_back(new Battle::WaitAction(20));

	SetState(State_AllyAction);
}

////////////////////////////////////////////////////////////
void Scene_Battle::DoItem() {
	Battle::Ally& ally = Game_Battle::GetActiveAlly();
	int item_id = item_window->GetItemId();
	const RPG::Item& item = Data::items[item_id - 1];

	UseItem(ally, item);
}

////////////////////////////////////////////////////////////
void Scene_Battle::DoSkill() {
	Battle::Ally& ally = Game_Battle::GetActiveAlly();
	const RPG::Skill& skill = Data::skills[skill_id - 1];

	UseSkill(ally, skill);
}

////////////////////////////////////////////////////////////
int Scene_Battle::SkillAnimation(const RPG::Skill& skill, const Battle::Ally& ally) {
	const std::vector<RPG::BattlerAnimationData>& anim_data = skill.battler_animation_data;
	std::vector<RPG::BattlerAnimationData>::const_iterator it;
	for (it = anim_data.begin(); it != anim_data.end(); it++) {
		const RPG::BattlerAnimationData& data = *it;
		if (data.ID != ally.game_actor->GetId())
			continue;
		return data.pose == 0 ? Battle::Ally::SkillUse : data.pose;
	}

	return Battle::Ally::SkillUse;
}

////////////////////////////////////////////////////////////
void Scene_Battle::EnemyAction() {
	Battle::Enemy& enemy = Game_Battle::GetActiveEnemy();

	if (!enemy.game_enemy->Exists())
		return;

	enemy.defending = false;

	enemy_action = ChooseEnemyAction(enemy);
	if (enemy_action == NULL)
		return;

	actions.clear();

	switch (enemy_action->kind) {
		case RPG::EnemyAction::Kind_basic:
			EnemyActionBasic();
			break;
		case RPG::EnemyAction::Kind_skill:
			EnemyActionSkill();
			break;
		case RPG::EnemyAction::Kind_transformation:
			actions.push_back(new Battle::WaitAction(20));
			actions.push_back(new Battle::CommandAction(&Scene_Battle::EnemyTransform));
			break;
	}

	actions.push_back(new Battle::CommandAction(&Scene_Battle::EnemyActionDone));
	actions.push_back(new Battle::WaitAction(20));

	SetState(State_EnemyAction);
}

////////////////////////////////////////////////////////////
void Scene_Battle::EnemyActionBasic() {
	switch (enemy_action->basic) {
		case RPG::EnemyAction::Basic_attack:
		{
			Game_Battle::TargetRandomAlly();
			Battle::Ally &ally = Game_Battle::GetTargetAlly();
			actions.push_back(new Battle::WaitAction(20));
			actions.push_back(new Battle::AnimationAction(ally.sprite, &Data::animations[0]));
			actions.push_back(new Battle::CommandAction1(&Scene_Battle::EnemyAttack, (void*) &ally));
			break;
		}
		case RPG::EnemyAction::Basic_dual_attack:
		{
			Game_Battle::TargetRandomAlly();
			Battle::Ally &ally1 = Game_Battle::GetTargetAlly();
			actions.push_back(new Battle::WaitAction(20));
			actions.push_back(new Battle::AnimationAction(ally1.sprite, &Data::animations[0]));
			actions.push_back(new Battle::CommandAction1(&Scene_Battle::EnemyAttack, (void*) &ally1));
			Game_Battle::TargetRandomAlly();
			Battle::Ally &ally2 = Game_Battle::GetTargetAlly();
			actions.push_back(new Battle::WaitAction(20));
			actions.push_back(new Battle::AnimationAction(ally2.sprite, &Data::animations[0]));
			actions.push_back(new Battle::CommandAction1(&Scene_Battle::EnemyAttack, (void*) &ally2));
			break;
		}
		case RPG::EnemyAction::Basic_defense:
			actions.push_back(new Battle::WaitAction(20));
			actions.push_back(new Battle::CommandAction(&Scene_Battle::EnemyDefend));
			break;
		case RPG::EnemyAction::Basic_observe:
			actions.push_back(new Battle::WaitAction(20));
			actions.push_back(new Battle::CommandAction(&Scene_Battle::EnemyObserve));
			break;
		case RPG::EnemyAction::Basic_charge:
			actions.push_back(new Battle::WaitAction(20));
			actions.push_back(new Battle::CommandAction(&Scene_Battle::EnemyCharge));
			break;
		case RPG::EnemyAction::Basic_autodestruction:
			actions.push_back(new Battle::WaitAction(20));
			actions.push_back(new Battle::CommandAction(&Scene_Battle::EnemyDestruct));
		case RPG::EnemyAction::Basic_nothing:
			actions.push_back(new Battle::WaitAction(20));
			break;
	}
}

////////////////////////////////////////////////////////////
void Scene_Battle::EnemyActionSkill() {
	Battle::Enemy& enemy = Game_Battle::GetActiveEnemy();
	const RPG::Skill& skill = Data::skills[enemy_action->skill_id - 1];
	const RPG::Animation* animation = (skill.animation_id != 0)
		? &Data::animations[skill.animation_id - 1]
		: NULL;
	Battle::Action* action = NULL;
	int x, y;

	switch (skill.type) {
		case RPG::Skill::Type_teleport:
		case RPG::Skill::Type_escape:
			action = new Battle::MoveAction(enemy.sprite, enemy.sprite->GetX(), -enemy.sprite->GetWidth(), 8);
			break;
		case RPG::Skill::Type_switch:
			break;
		case RPG::Skill::Type_normal:
		default:
			switch (skill.scope) {
				case RPG::Skill::Scope_enemy:
					Game_Battle::TargetRandomAlly();
					if (animation != NULL)
						action = new Battle::AnimationAction(Game_Battle::GetTargetAlly().sprite, animation);
					break;
				case RPG::Skill::Scope_self:
					if (animation != NULL)
						action = new Battle::AnimationAction(enemy.sprite, animation);
					break;
				case RPG::Skill::Scope_ally:
					Game_Battle::TargetRandomEnemy();
					if (animation != NULL)
						action = new Battle::AnimationAction(Game_Battle::GetTargetEnemy().sprite, animation);
					break;
				case RPG::Skill::Scope_enemies:
					if (animation != NULL) {
						Game_Battle::AlliesCentroid(x, y);
						action = new Battle::AnimationAction(x, y, animation);
					}
					break;
				case RPG::Skill::Scope_party:
					if (animation != NULL) {
						Game_Battle::EnemiesCentroid(x, y);
						action = new Battle::AnimationAction(x, y, animation);
					}
					break;
				default:
					break;
			}
	}

	if (action == NULL)
		action = new Battle::WaitAction(30);

	actions.push_back(action);
	actions.push_back(new Battle::CommandAction(&Scene_Battle::EnemySkill));
}

////////////////////////////////////////////////////////////
void Scene_Battle::EnemyAttack(void* target) {
	Battle::Enemy& enemy = Game_Battle::GetActiveEnemy();
	Battle::Ally& ally = *((Battle::Ally*)target);

	EnemyAttackAlly(enemy, ally);
}

////////////////////////////////////////////////////////////
void Scene_Battle::EnemyDefend() {
	Battle::Enemy& enemy = Game_Battle::GetActiveEnemy();
	const std::string msg = !Data::terms.defending.empty()
		? Data::terms.defending
		: " is defending";
	Message(enemy.rpg_enemy->name + msg);
	enemy.defending = true;
}

////////////////////////////////////////////////////////////
void Scene_Battle::EnemyObserve() {
	Battle::Enemy& enemy = Game_Battle::GetActiveEnemy();
	const std::string msg = !Data::terms.observing.empty()
		? Data::terms.observing
		: " is observing the battle";
	Message(enemy.rpg_enemy->name + msg);
}

////////////////////////////////////////////////////////////
void Scene_Battle::EnemyCharge() {
	Battle::Enemy& enemy = Game_Battle::GetActiveEnemy();
	const std::string msg = !Data::terms.focus.empty()
		? Data::terms.focus
		: " is charging";
	Message(enemy.rpg_enemy->name + msg);
	enemy.charged = true;
}

////////////////////////////////////////////////////////////
void Scene_Battle::EnemyDestruct() {
	Battle::Enemy& enemy = Game_Battle::GetActiveEnemy();
	const std::string msg = !Data::terms.autodestruction.empty()
		? Data::terms.autodestruction
		: "Self-destruct";
	Message(msg);
	enemy.charged = true;
	for (std::vector<Battle::Ally>::iterator it = Game_Battle::allies.begin(); it != Game_Battle::allies.end(); it++)
		EnemyAttack((void*)&*it);
	enemy.game_enemy->SetHp(0);
}

////////////////////////////////////////////////////////////
void Scene_Battle::EnemySkill() {
	const RPG::EnemyAction* action = enemy_action;
	const RPG::Skill& skill = Data::skills[action->skill_id - 1];
	Battle::Enemy& enemy = Game_Battle::GetActiveEnemy();
	EnemySkill(enemy, skill);
}

////////////////////////////////////////////////////////////
void Scene_Battle::EnemyTransform() {
	Battle::Enemy& enemy = Game_Battle::GetActiveEnemy();
	const RPG::EnemyAction* action = enemy_action;

	enemy.Transform(action->enemy_id);
}

////////////////////////////////////////////////////////////
void Scene_Battle::EnemyActionDone() {
	const RPG::EnemyAction* action = enemy_action;

	if (action->switch_on)
		Game_Switches[action->switch_on_id] = true;
	if (action->switch_off)
		Game_Switches[action->switch_off_id] = false;

	SetState(State_Battle);
}

////////////////////////////////////////////////////////////
void Scene_Battle::ProcessActions() {
	switch (state) {
		case State_Battle:
		case State_AutoBattle:
			Game_Battle::Update();

			CheckWin();
			CheckLose();
			CheckAbort();
			CheckFlee();

			if (help_window->GetVisible() && message_timer > 0) {
				message_timer--;
				if (message_timer <= 0)
					help_window->SetVisible(false);
			}

			while (Game_Battle::NextActiveEnemy())
				EnemyAction();

			break;
		case State_AllyAction:
		case State_EnemyAction:
			if (!actions.empty()) {
				Battle::Action* action = actions.front();
				if ((*action)()) {
					delete action;
					actions.pop_front();
				}
			}
		default:
			break;
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
						auto_battle = false;
						SetState(State_Battle);
						break;
					case 1:
						auto_battle = true;
						SetState(State_Battle);
						break;
					case 2:
						Escape();
						break;
				}
				break;
			case State_Battle:
				Game_Battle::SetActiveAlly(status_window->GetActiveCharacter());
				if (Game_Battle::HaveActiveAlly())
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
			case State_AllyAction:
			case State_EnemyAction:
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
			case State_AllyAction:
			case State_EnemyAction:
				break;
			case State_Victory:
			case State_Defeat:
				Scene::Pop();
				break;
		}
	}

	if (state == State_TargetEnemy && Game_Battle::HaveTargetEnemy()) {
		if (Input::IsRepeated(Input::DOWN))
			Game_Battle::TargetNextEnemy();
		if (Input::IsRepeated(Input::UP))
			Game_Battle::TargetPreviousEnemy();
		Game_Battle::ChooseEnemy();
	}

	if (state == State_TargetAlly && Game_Battle::HaveTargetAlly()) {
		if (Input::IsRepeated(Input::DOWN))
			Game_Battle::TargetNextAlly();
		if (Input::IsRepeated(Input::UP))
			Game_Battle::TargetPreviousAlly();
	}
}

////////////////////////////////////////////////////////////
void Scene_Battle::DoAuto() {
	if (state != State_AutoBattle)
		return;

	Game_Battle::SetActiveAlly(status_window->GetActiveCharacter());
	if (!Game_Battle::HaveActiveAlly())
		return;

	// FIXME: this assumes that the character has an "Attack" command.
	// FIXME: should try to use some intelligence

	Game_Battle::ChooseEnemy();
	if (!Game_Battle::HaveTargetEnemy())
		return;

	pending_command = 1;
	DoAttack();

	Restart(Game_Battle::GetActiveAlly());
}

////////////////////////////////////////////////////////////
void Scene_Battle::UpdateCursors() {
	if (Game_Battle::HaveActiveAlly()) {
		const Battle::Ally& ally = state == State_TargetAlly && Game_Battle::HaveTargetAlly()
			? Game_Battle::GetTargetAlly()
			: Game_Battle::GetActiveAlly();
		ally_cursor->SetVisible(true);
		ally_cursor->SetX(ally.rpg_actor->battle_x - ally_cursor->GetWidth() / 2);
		ally_cursor->SetY(ally.rpg_actor->battle_y - ally.sprite->GetHeight() / 2 - ally_cursor->GetHeight() - 2);
		static const int frames[] = {0,1,2,1};
		int frame = frames[(cycle / 15) % 4];
		ally_cursor->SetSrcRect(Rect(frame * 16, 16, 16, 16));
	}
	else
		ally_cursor->SetVisible(false);

	if (state == State_TargetEnemy && Game_Battle::HaveTargetEnemy()) {
		const Battle::Enemy& enemy = Game_Battle::GetTargetEnemy();
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
void Scene_Battle::UpdateSprites() {
	for (std::vector<Battle::Enemy>::iterator it = Game_Battle::enemies.begin(); it != Game_Battle::enemies.end(); it++) {
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

	for (std::vector<Battle::Ally>::iterator it = Game_Battle::allies.begin(); it != Game_Battle::allies.end(); it++)
		it->UpdateAnim(cycle);
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

	Game_Battle::SetActiveAlly(status_window->GetActiveCharacter());
	command_window->SetActor(Game_Battle::GetActiveActor());

	cycle++;

	ProcessActions();
	ProcessInput();
	DoAuto();
	UpdateCursors();
	UpdateSprites();
	UpdateFloaters();
	Game_Battle::UpdateAnimations();
}

////////////////////////////////////////////////////////////
void Scene_Battle::CheckWin() {
	if (state == State_Victory || state == State_Defeat)
		return;

	if (!Game_Battle::CheckWin())
		return;

	for (std::vector<Battle::Ally>::iterator it = Game_Battle::allies.begin(); it != Game_Battle::allies.end(); it++)
		it->SetAnimState(Battle::Ally::Victory);
	Game_Temp::battle_result = Game_Temp::BattleVictory;
	SetState(State_Victory);
	Message(Data::terms.victory.empty() ? Data::terms.victory : "Victory");
}

////////////////////////////////////////////////////////////
void Scene_Battle::CheckLose() {
	if (state == State_Victory || state == State_Defeat)
		return;

	if (!Game_Battle::CheckLose())
		return;

	Game_Temp::battle_result = Game_Temp::BattleDefeat;
	SetState(State_Defeat);
	Message(!Data::terms.defeat.empty() ? Data::terms.defeat : "Defeat");
}

////////////////////////////////////////////////////////////
void Scene_Battle::CheckAbort() {
	if (!Game_Battle::terminate)
		return;
	Game_Temp::battle_result = Game_Temp::BattleAbort;
	Scene::Pop();
}

////////////////////////////////////////////////////////////
void Scene_Battle::CheckFlee() {
	if (!Game_Battle::allies_flee)
		return;
	Game_Battle::allies_flee = false;
	Game_Temp::battle_result = Game_Temp::BattleEscape;
	Scene::Pop();
}

