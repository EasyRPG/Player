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

#ifndef EP_SCENE_BATTLE_RPG2K3_H
#define EP_SCENE_BATTLE_RPG2K3_H

// Headers
#include "scene_battle.h"
#include "async_handler.h"
#include "window_actorsp.h"

// CBA constant
// The CBA move frame counter is incremented twice per frame in RPG_RT,
// so the effective frame count is 13
constexpr int cba_num_move_frames = 25;

/**
 * Scene_Battle class.
 * Manages the battles.
 */
class Scene_Battle_Rpg2k3 : public Scene_Battle {
public:
	/** The return value from a scene action state machine callback */
	enum class SceneActionReturn {
		/** Return from Update() and wait until the next frame */
		eWaitTillNextFrame,
		/** Continue processing this frame, unless CheckWait() etc.. requires us to block */
		eContinueThisFrame,
	};

	/** The return value from a battle action state machine callback */
	enum class BattleActionReturn {
		/** The battle action is not yet finished but we can try again this frame */
		eContinue,
		/** The battle action is not yet finished and we need to wait for another frame*/
		eWait,
		/** The battle action is finished */
		eFinished,
	};

	enum class EventTriggerType {
		eBeforeBattleAction,
		eAfterBattleAction,
		eAll,
	};

	enum BattleActionState {
		BattleActionState_Begin,
		BattleActionState_PreEvents,
		BattleActionState_Conditions,
		BattleActionState_Notify,
		BattleActionState_Combo,
		BattleActionState_StartAlgo,
		BattleActionState_CBAInit,
		BattleActionState_CBAMove,
		BattleActionState_StartAnimation,
		BattleActionState_CBARangedWeaponInit,
		BattleActionState_CBARangedWeaponMove,
		BattleActionState_Animation,
		BattleActionState_AnimationReflect,
		BattleActionState_FinishPose,
		BattleActionState_Execute,
		BattleActionState_SwitchEvents,
		BattleActionState_Apply,
		BattleActionState_PostAction,
		BattleActionState_PostEvents,
		BattleActionState_Finished
	};

public:
	explicit Scene_Battle_Rpg2k3(const BattleArgs& args);
	~Scene_Battle_Rpg2k3() override;

	void Start() override;
	void vUpdate() override;
	void OnPartyChanged(Game_Actor* actor, bool add) override;
	void OnEventHpChanged(Game_Battler* battler, int hp) override;

protected:
	void Start2();
	void InitAtbGauge(Game_Battler& battler, int preempt_atb, int ambush_atb);
	void InitBattleCondition(lcf::rpg::System::BattleCondition condition);
	void InitEnemies();
	void InitActors();
	void InitAtbGauges();

	void UpdateEnemiesDirection();
	void UpdateActorsDirection();

	bool IsAtbAccumulating() const;
	bool IsBattleActionPending() const;
	void CreateEnemyActions();
	void CreateActorAutoActions();
	bool UpdateAtb();
	void UpdateAnimations();
	bool UpdateBattleState();

	void OnSystem2Ready(FileRequestResult* result);
	void SetupSystem2Graphics();
	void CreateUi() override;
	void CreateEnemySprites();
	void CreateActorSprites();
	void ResetAllBattlerZ();

	void CreateBattleTargetWindow();
	void CreateBattleStatusWindow();
	void CreateBattleCommandWindow();
	void RefreshTargetWindow();
	void RefreshCommandWindow(const Game_Actor* actor);
	void SetActiveActor(int idx);

	void DrawFloatText(int x, int y, int color, StringView text);

	bool IsTransparent() const;


	void SetState(Scene_Battle::State new_state) override;

	void FaceTarget(Game_Actor& source, const Game_Battler& target);

	void CommandSelected();
	void AttackSelected() override;
	void SubskillSelected(int command);
	void SpecialSelected();
	void EscapeSelected();
	void RowSelected();

	void ActionSelectedCallback(Game_Battler* for_battler) override;

	void ShowNotification(std::string text);
	void EndNotification();

	bool IsEscapeAllowed() const = delete; // disable accidental calls to base class version
	bool IsEscapeAllowedFromOptionWindow() const;
	bool IsEscapeAllowedFromActorCommand() const;

	bool CheckAnimFlip(Game_Battler* battler);

	void SetWait(int min_wait, int max_wait);
	bool CheckWait();

	void ResetWindows(bool make_invisible);
	void MoveCommandWindows(int x, int frames);

	void SetSceneActionSubState(int substate);
	void ReturnToMainBattleState();

	// SceneAction State Machine Driver
	SceneActionReturn ProcessSceneAction();

	// SceneAction State Machine Handlers
	SceneActionReturn ProcessSceneActionStart();
	SceneActionReturn ProcessSceneActionFightAutoEscape();
	SceneActionReturn ProcessSceneActionActor();
	SceneActionReturn ProcessSceneActionAutoBattle();
	SceneActionReturn ProcessSceneActionCommand();
	SceneActionReturn ProcessSceneActionItem();
	SceneActionReturn ProcessSceneActionSkill();
	SceneActionReturn ProcessSceneActionEnemyTarget();
	SceneActionReturn ProcessSceneActionAllyTarget();
	SceneActionReturn ProcessSceneActionBattle();
	SceneActionReturn ProcessSceneActionVictory();
	SceneActionReturn ProcessSceneActionDefeat();
	SceneActionReturn ProcessSceneActionEscape();

	void NextTurn(Game_Battler* battler);
	bool CheckBattleEndAndScheduleEvents(EventTriggerType tt, Game_Battler* source);
	bool CheckBattleEndConditions();

	void SetBattleActionState(BattleActionState state);

	/** Battle Action Driver */
	BattleActionReturn ProcessBattleAction(Game_BattleAlgorithm::AlgorithmBase* action);

	/** Battle Action State Machine callbacks */
	BattleActionReturn ProcessBattleActionBegin(Game_BattleAlgorithm::AlgorithmBase* action);
	BattleActionReturn ProcessBattleActionPreEvents(Game_BattleAlgorithm::AlgorithmBase* action);
	BattleActionReturn ProcessBattleActionConditions(Game_BattleAlgorithm::AlgorithmBase* action);
	BattleActionReturn ProcessBattleActionNotify(Game_BattleAlgorithm::AlgorithmBase* action);
	BattleActionReturn ProcessBattleActionCombo(Game_BattleAlgorithm::AlgorithmBase* action);
	BattleActionReturn ProcessBattleActionStartAlgo(Game_BattleAlgorithm::AlgorithmBase* action);
	BattleActionReturn ProcessBattleActionCBAInit(Game_BattleAlgorithm::AlgorithmBase* action);
	BattleActionReturn ProcessBattleActionCBAMove(Game_BattleAlgorithm::AlgorithmBase* action);
	BattleActionReturn ProcessBattleActionStartAnimation(Game_BattleAlgorithm::AlgorithmBase* action);
	BattleActionReturn ProcessBattleActionCBARangedWeaponInit(Game_BattleAlgorithm::AlgorithmBase* action);
	BattleActionReturn ProcessBattleActionCBARangedWeaponMove(Game_BattleAlgorithm::AlgorithmBase* action);
	BattleActionReturn ProcessBattleActionAnimation(Game_BattleAlgorithm::AlgorithmBase* action);
	BattleActionReturn ProcessBattleActionAnimationReflect(Game_BattleAlgorithm::AlgorithmBase* action);
	BattleActionReturn ProcessBattleActionFinishPose(Game_BattleAlgorithm::AlgorithmBase* action);
	BattleActionReturn ProcessBattleActionExecute(Game_BattleAlgorithm::AlgorithmBase* action);
	BattleActionReturn ProcessBattleActionSwitchEvents(Game_BattleAlgorithm::AlgorithmBase* action);
	BattleActionReturn ProcessBattleActionApply(Game_BattleAlgorithm::AlgorithmBase* action);
	BattleActionReturn ProcessBattleActionPostAction(Game_BattleAlgorithm::AlgorithmBase* action);
	BattleActionReturn ProcessBattleActionPostEvents(Game_BattleAlgorithm::AlgorithmBase* action);
	BattleActionReturn ProcessBattleActionFinished(Game_BattleAlgorithm::AlgorithmBase* action);

	std::vector<std::string> GetBattleCommandNames(const Game_Actor* actor);
	void SetBattleCommandsDisable(Window_Command& window, const Game_Actor* actor);

	std::unique_ptr<Sprite> ally_cursor, enemy_cursor;

	struct FloatText {
		std::shared_ptr<Sprite> sprite;
		int remaining_time = 30;
	};

	std::vector<FloatText> floating_texts;
	int battle_action_wait = 0;
	int battle_action_min_wait = 0;
	int scene_action_substate = 0;
	int battle_action_state = BattleActionState_Begin;
	int battle_end_timer = 0;

	std::unique_ptr<Window_ActorSp> sp_window;
	void RecreateSpWindow(Game_Battler* battler);

	FileRequestBinding request_id;
	std::shared_ptr<Game_BattleAlgorithm::AlgorithmBase> pending_battle_action = {};
	bool first_strike = false;
	bool running_away = false;
	bool resume_from_debug_scene = false;
	bool auto_battle = false;

	// CBA stuff
	void CBAInit();
	void CBAMove();

	Game_BattleAlgorithm::AlgorithmBase* cba_action;
	bool cba_direction_back = false;
	int cba_move_frame = 0;
	Point cba_start_pos;
	Point cba_end_pos;

	std::vector<std::pair<Game_Battler&, std::unique_ptr<Sprite_Weapon>>> cba_ranged;
	Point cba_ranged_center;
	int cba_ranged_weapon_move_frame = 0;
	int cba_num_ranged_weapon_move_frames = 60;

	// ATB order stuff
	void UpdateReadyActors();
	int GetNextReadyActor();

	std::vector<int> atb_order;
};

#endif
