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
#include <ciso646>

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
#include "bitmap.h"

////////////////////////////////////////////////////////////
Scene_Battle::Scene_Battle() {
	Scene::type = Scene::Battle;
}

Scene_Battle::~Scene_Battle() {
	Game_Battle::Quit();
}

////////////////////////////////////////////////////////////
Scene_Battle::FloatText::FloatText(int x, int y, int color, const std::string& text, int _duration) {
	Rect rect = Font::Default()->GetSize(text);

	BitmapRef graphic = Bitmap::Create(rect.width, rect.height);
	graphic->Clear();
	graphic->TextDraw(-rect.x, -rect.y, color, text);

	sprite.reset(new Sprite());
	sprite->SetBitmap(graphic);
	sprite->SetOx(rect.width / 2);
	sprite->SetOy(rect.height + 5);
	sprite->SetX(x);
	sprite->SetY(y);
	sprite->SetZ(500+y);

	duration = _duration;
}

////////////////////////////////////////////////////////////
void Scene_Battle::CreateCursors() {
	BitmapRef system2 = Cache::System2(Data::system.system2_name);

	ally_cursor.reset(new Sprite());
	ally_cursor->SetBitmap(system2);
	ally_cursor->SetSrcRect(Rect(0, 16, 16, 16));
	ally_cursor->SetZ(999);
	ally_cursor->SetVisible(false);

	enemy_cursor.reset(new Sprite());
	enemy_cursor->SetBitmap(system2);
	enemy_cursor->SetSrcRect(Rect(0, 0, 16, 16));
	enemy_cursor->SetZ(999);
	enemy_cursor->SetVisible(false);
}

////////////////////////////////////////////////////////////
void Scene_Battle::CreateWindows() {
	help_window.reset(new Window_Help(0, 0, 320, 32));
	help_window->SetVisible(false);

	options_window.reset(new Window_BattleOption(0, 172, 76, 68));

	status_window.reset(new Window_BattleStatus());

	command_window.reset(new Window_BattleCommand(244, 172, 76, 68));

	skill_window.reset(new Window_BattleSkill(0, 172, 320, 68));
	skill_window->SetVisible(false);

	item_window.reset(new Window_BattleItem(0, 172, 320, 68));
	item_window->SetVisible(false);
	item_window->Refresh();
	item_window->SetIndex(0);
}

////////////////////////////////////////////////////////////
void Scene_Battle::Start() {
	if (Player::battle_test_flag) {
		if (Player::battle_test_troop_id <= 0) {
			Output::Error("Invalid Monster Party Id");
		} else {
			Game_Temp::battle_troop_id = Player::battle_test_troop_id;
			// ToDo: Rpg2k does specify a background graphic instead
			Game_Temp::battle_terrain_id = 1;
		}
	}

	Game_Battle::Init(this);

	cycle = 0;
	auto_battle = false;
	enemy_action = NULL;

	CreateCursors();
	CreateWindows();

	animation.reset();
	animations.clear();

	if (!Game_Temp::battle_background.empty())
		background.reset(new Background(Game_Temp::battle_background));
	else
		background.reset(new Background(Game_Temp::battle_terrain_id));

	SetState(State_Options);
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
			item_window->SetHelpWindow(help_window.get());
			help_window->SetVisible(true);
			break;
		case State_Skill:
			skill_window->SetVisible(true);
			skill_window->SetHelpWindow(help_window.get());
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
	floaters.push_back(EASYRPG_MAKE_SHARED<FloatText>(x, y, color, text, duration));
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
void Scene_Battle::UpdateAnimState() {
	Battle::Ally& ally = Game_Battle::GetActiveAlly();
	int anim_state = Battle::Ally::Idle;
	if (ally.defending)
		anim_state = Battle::Ally::Defending;
	const RPG::State* state = ally.GetActor()->GetState();
	if (state)
		anim_state = state->battler_animation_id == 100
			? 7
			: state->battler_animation_id + 1;
	SetAnimState(ally, anim_state);
}

////////////////////////////////////////////////////////////
void Scene_Battle::Restart() {
	UpdateAnimState();
	SetState(State_Battle);
	Game_Battle::GetActiveAlly().last_command = pending_command;
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
	if (Game_Battle::Escape())
		Scene::Pop();
	else
		Game_Battle::Restart();
}

////////////////////////////////////////////////////////////
void Scene_Battle::Special() {
	// FIXME: special commands (link to event)

	Game_Battle::Restart();
}

////////////////////////////////////////////////////////////
void Scene_Battle::Defend() {
	Game_Battle::Defend();
	Game_Battle::Restart();
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
			Game_Battle::Restart();
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
	actions.push_back(EASYRPG_MAKE_SHARED<Battle::SpriteAction>(&ally, Battle::Ally::WalkingLeft));
	actions.push_back(EASYRPG_MAKE_SHARED<Battle::MoveAction>(ally.sprite.get(), x0, x1, 8));
	actions.push_back(EASYRPG_MAKE_SHARED<Battle::SpriteAction>(&ally, Battle::Ally::RightHand));
	actions.push_back(EASYRPG_MAKE_SHARED<Battle::WaitAction>(15));
	actions.push_back(EASYRPG_MAKE_SHARED<Battle::CommandAction>(&Game_Battle::Attack));
	actions.push_back(EASYRPG_MAKE_SHARED<Battle::SpriteAction>(&ally, Battle::Ally::WalkingRight));
	actions.push_back(EASYRPG_MAKE_SHARED<Battle::MoveAction>(ally.sprite.get(), x1, x0, 8));
	actions.push_back(EASYRPG_MAKE_SHARED<Battle::CommandAction>(&Game_Battle::Restart));
	actions.push_back(EASYRPG_MAKE_SHARED<Battle::WaitAction>(20));

	SetState(State_AllyAction);
}

////////////////////////////////////////////////////////////
void Scene_Battle::BeginItem() {
	Battle::Ally& ally = Game_Battle::GetActiveAlly();

	Game_Battle::SetItem(item_window->GetItemId());

	actions.clear();
	actions.push_back(EASYRPG_MAKE_SHARED<Battle::SpriteAction>(&ally, Battle::Ally::Item));
	actions.push_back(EASYRPG_MAKE_SHARED<Battle::WaitAction>(60));
	actions.push_back(EASYRPG_SHARED_PTR<Battle::CommandAction>(new Battle::CommandAction(&Game_Battle::UseItem)));
	actions.push_back(EASYRPG_SHARED_PTR<Battle::CommandAction>(new Battle::CommandAction(&Game_Battle::Restart)));
	actions.push_back(EASYRPG_MAKE_SHARED<Battle::WaitAction>(20));

	SetState(State_AllyAction);
}

////////////////////////////////////////////////////////////
void Scene_Battle::BeginSkill() {
	Battle::Ally& ally = Game_Battle::GetActiveAlly();
	const RPG::Skill& skill = Data::skills[skill_id - 1];
	int anim_state = SkillAnimation(skill, ally);
	const RPG::Animation* animation = (skill.animation_id != 0)
		? &Data::animations[skill.animation_id - 1]
		: NULL;
	EASYRPG_SHARED_PTR<Battle::Action> action;
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
						action = EASYRPG_MAKE_SHARED<Battle::AnimationAction>(Game_Battle::GetTargetEnemy().sprite.get(), animation);
					break;
				case RPG::Skill::Scope_enemies:
					if (animation != NULL) {
						Game_Battle::EnemiesCentroid(x, y);
						action = EASYRPG_MAKE_SHARED<Battle::AnimationAction>(x, y, animation);
					}
					break;
				case RPG::Skill::Scope_self:
					if (animation != NULL)
						action = EASYRPG_MAKE_SHARED<Battle::AnimationAction>(ally.sprite.get(), animation);
					break;
				case RPG::Skill::Scope_ally:
					if (animation != NULL)
						action = EASYRPG_MAKE_SHARED<Battle::AnimationAction>(Game_Battle::GetTargetAlly().sprite.get(), animation);
					break;
				case RPG::Skill::Scope_party:
					if (animation != NULL) {
						Game_Battle::AlliesCentroid(x, y);
						action = EASYRPG_MAKE_SHARED<Battle::AnimationAction>(x, y, animation);
					}
				default:
					break;
			}
	}

	if (not action)
		action = EASYRPG_MAKE_SHARED<Battle::WaitAction>(10);

	actions.clear();
	actions.push_back(EASYRPG_MAKE_SHARED<Battle::SpriteAction>(&ally, anim_state));
	actions.push_back(action);
	actions.push_back(EASYRPG_SHARED_PTR<Battle::CommandAction>(new Battle::CommandAction(&Game_Battle::UseSkill)));
	actions.push_back(EASYRPG_SHARED_PTR<Battle::CommandAction>(new Battle::CommandAction(&Game_Battle::Restart)));
	actions.push_back(EASYRPG_MAKE_SHARED<Battle::WaitAction>(20));

	SetState(State_AllyAction);
}

////////////////////////////////////////////////////////////
void Scene_Battle::DoItem() {
	Game_Battle::UseItem();
}

////////////////////////////////////////////////////////////
void Scene_Battle::DoSkill() {
	Game_Battle::SetSkill(skill_id);
	Game_Battle::UseSkill();
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

	enemy_action = Game_Battle::ChooseEnemyAction(enemy);
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
			Game_Battle::SetMorph(enemy_action->enemy_id);
			actions.push_back(EASYRPG_MAKE_SHARED<Battle::WaitAction>(20));
			actions.push_back(EASYRPG_MAKE_SHARED<Battle::CommandAction>(&Game_Battle::EnemyTransform));
			break;
	}

	actions.push_back(EASYRPG_MAKE_SHARED<Battle::CommandAction>(&Game_Battle::EnemyActionDone));
	actions.push_back(EASYRPG_MAKE_SHARED<Battle::CommandAction1>(&Scene_Battle::EnemyActionDone, (void*) this));
	actions.push_back(EASYRPG_MAKE_SHARED<Battle::WaitAction>(20));

	SetState(State_EnemyAction);
}

////////////////////////////////////////////////////////////
void Scene_Battle::EnemyActionBasic() {
	switch (enemy_action->basic) {
		case RPG::EnemyAction::Basic_attack:
		{
			Game_Battle::TargetRandomAlly();
			Battle::Ally &ally = Game_Battle::GetTargetAlly();
			actions.push_back(EASYRPG_MAKE_SHARED<Battle::WaitAction>(20));
			actions.push_back(EASYRPG_MAKE_SHARED<Battle::AnimationAction>(ally.sprite.get(), &Data::animations[0]));
			actions.push_back(EASYRPG_MAKE_SHARED<Battle::CommandAction1>(&Game_Battle::EnemyAttack, (void*) &ally));
			break;
		}
		case RPG::EnemyAction::Basic_dual_attack:
		{
			Game_Battle::TargetRandomAlly();
			Battle::Ally &ally1 = Game_Battle::GetTargetAlly();
			actions.push_back(EASYRPG_MAKE_SHARED<Battle::WaitAction>(20));
			actions.push_back(EASYRPG_MAKE_SHARED<Battle::AnimationAction>(ally1.sprite.get(), &Data::animations[0]));
			actions.push_back(EASYRPG_MAKE_SHARED<Battle::CommandAction1>(&Game_Battle::EnemyAttack, (void*) &ally1));
			Game_Battle::TargetRandomAlly();
			Battle::Ally &ally2 = Game_Battle::GetTargetAlly();
			actions.push_back(EASYRPG_MAKE_SHARED<Battle::WaitAction>(20));
			actions.push_back(EASYRPG_MAKE_SHARED<Battle::AnimationAction>(ally2.sprite.get(), &Data::animations[0]));
			actions.push_back(EASYRPG_MAKE_SHARED<Battle::CommandAction1>(&Game_Battle::EnemyAttack, (void*) &ally2));
			break;
		}
		case RPG::EnemyAction::Basic_defense:
			actions.push_back(EASYRPG_MAKE_SHARED<Battle::WaitAction>(20));
			actions.push_back(EASYRPG_MAKE_SHARED<Battle::CommandAction>(&Game_Battle::EnemyDefend));
			break;
		case RPG::EnemyAction::Basic_observe:
			actions.push_back(EASYRPG_MAKE_SHARED<Battle::WaitAction>(20));
			actions.push_back(EASYRPG_MAKE_SHARED<Battle::CommandAction>(&Game_Battle::EnemyObserve));
			break;
		case RPG::EnemyAction::Basic_charge:
			actions.push_back(EASYRPG_MAKE_SHARED<Battle::WaitAction>(20));
			actions.push_back(EASYRPG_MAKE_SHARED<Battle::CommandAction>(&Game_Battle::EnemyCharge));
			break;
		case RPG::EnemyAction::Basic_autodestruction:
			actions.push_back(EASYRPG_MAKE_SHARED<Battle::WaitAction>(20));
			actions.push_back(EASYRPG_MAKE_SHARED<Battle::CommandAction>(&Game_Battle::EnemyDestruct));
		case RPG::EnemyAction::Basic_nothing:
			actions.push_back(EASYRPG_MAKE_SHARED<Battle::WaitAction>(20));
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
	EASYRPG_SHARED_PTR<Battle::Action> action;
	int x, y;

	Game_Battle::SetSkill(enemy_action->skill_id);

	switch (skill.type) {
		case RPG::Skill::Type_teleport:
		case RPG::Skill::Type_escape:
			action = EASYRPG_MAKE_SHARED<Battle::MoveAction>(enemy.sprite.get(), enemy.sprite->GetX(), -enemy.sprite->GetWidth(), 8);
			break;
		case RPG::Skill::Type_switch:
			break;
		case RPG::Skill::Type_normal:
		default:
			switch (skill.scope) {
				case RPG::Skill::Scope_enemy:
					Game_Battle::TargetRandomAlly();
					if (animation != NULL)
						action = EASYRPG_MAKE_SHARED<Battle::AnimationAction>(Game_Battle::GetTargetAlly().sprite.get(), animation);
					break;
				case RPG::Skill::Scope_self:
					if (animation != NULL)
						action = EASYRPG_MAKE_SHARED<Battle::AnimationAction>(enemy.sprite.get(), animation);
					break;
				case RPG::Skill::Scope_ally:
					Game_Battle::TargetRandomEnemy();
					if (animation != NULL)
						action = EASYRPG_MAKE_SHARED<Battle::AnimationAction>(Game_Battle::GetTargetEnemy().sprite.get(), animation);
					break;
				case RPG::Skill::Scope_enemies:
					if (animation != NULL) {
						Game_Battle::AlliesCentroid(x, y);
						action = EASYRPG_MAKE_SHARED<Battle::AnimationAction>(x, y, animation);
					}
					break;
				case RPG::Skill::Scope_party:
					if (animation != NULL) {
						Game_Battle::EnemiesCentroid(x, y);
						action = EASYRPG_MAKE_SHARED<Battle::AnimationAction>(x, y, animation);
					}
					break;
				default:
					break;
			}
	}

	if (not action)
		action = EASYRPG_MAKE_SHARED<Battle::WaitAction>(30);

	actions.push_back(action);
	actions.push_back(EASYRPG_SHARED_PTR<Battle::CommandAction>(new Battle::CommandAction(&Game_Battle::EnemySkill)));
}

////////////////////////////////////////////////////////////
void Scene_Battle::EnemyActionDone(void* param) {
	Scene_Battle* thiz = (Scene_Battle*) param;
	thiz->SetState(State_Battle);
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
				Battle::Action& action = *actions.front();
				if (action()) {
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
	Game_Battle::Attack();

	Game_Battle::Restart();
}

////////////////////////////////////////////////////////////
void Scene_Battle::UpdateBackground() {
	if (Game_Temp::battle_background != Game_Battle::background_name) {
		Game_Temp::battle_background = Game_Battle::background_name;
		background.reset(new Background(Game_Temp::battle_background));
	}
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
	std::vector<EASYRPG_SHARED_PTR<FloatText> >::iterator it;
	std::vector<EASYRPG_SHARED_PTR<FloatText> >::iterator dst = floaters.begin();
	for (it = floaters.begin(); it != floaters.end(); it++) {
		FloatText* floater = it->get();
		floater->duration--;
		if (floater->duration <= 0)
			it->reset();
		else
			*dst++ = *it;
	}

	floaters.erase(dst, floaters.end());
}

////////////////////////////////////////////////////////////
void Scene_Battle::ShowAnimation(int animation_id, bool allies, Battle::Ally* ally, Battle::Enemy* enemy, bool wait) {
	const RPG::Animation* rpg_anim = &Data::animations[animation_id - 1];
	int x, y;

	if (ally != NULL) {
		x = ally->sprite->GetX();
		y = ally->sprite->GetY();
	}
	else if (enemy != NULL) {
		x = enemy->sprite->GetX();
		y = enemy->sprite->GetY();
	}
	else if (allies)
		Game_Battle::AlliesCentroid(x, y);
	else
		Game_Battle::EnemiesCentroid(x, y);

	EASYRPG_SHARED_PTR<BattleAnimation> new_animation =
		EASYRPG_MAKE_SHARED<BattleAnimation>(x, y, rpg_anim);

	if (wait) {
		animation = new_animation;
	}
	else
		animations.push_back(new_animation);
}

////////////////////////////////////////////////////////////
void Scene_Battle::UpdateAnimations() {
	if (animation) {
		animation->Update();
		if (animation->IsDone()) {
			animation.reset();
		}
	}

	for (std::deque<EASYRPG_SHARED_PTR<BattleAnimation> >::iterator it = animations.begin(); it != animations.end(); it++) {
		BattleAnimation* anim = it->get();
		if (anim == NULL)
			continue;
		anim->Update();
		if (anim->IsDone()) {
			it->reset();
		}
	}

	std::deque<EASYRPG_SHARED_PTR<BattleAnimation> >::iterator end;
	end = std::remove(animations.begin(), animations.end(), EASYRPG_SHARED_PTR<BattleAnimation>());
	animations.erase(end, animations.end());
}

////////////////////////////////////////////////////////////
bool Scene_Battle::IsAnimationWaiting() {
	return bool(animation);
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

	UpdateBackground();
	UpdateCursors();
	UpdateSprites();
	UpdateFloaters();
	UpdateAnimations();
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
