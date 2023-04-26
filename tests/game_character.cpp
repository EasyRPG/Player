#include "game_character.h"
#include "game_player.h"
#include "game_event.h"
#include "game_vehicle.h"
#include "doctest.h"
#include <climits>

static_assert(Game_Character::GetMaxStopCountForStep(1) == 256, "StopCountBroken");
static_assert(Game_Character::GetMaxStopCountForStep(2) == 128, "StopCountBroken");
static_assert(Game_Character::GetMaxStopCountForStep(3) == 64, "StopCountBroken");
static_assert(Game_Character::GetMaxStopCountForStep(4) == 32, "StopCountBroken");
static_assert(Game_Character::GetMaxStopCountForStep(5) == 16, "StopCountBroken");
static_assert(Game_Character::GetMaxStopCountForStep(6) == 8, "StopCountBroken");
static_assert(Game_Character::GetMaxStopCountForStep(7) == 4, "StopCountBroken");
static_assert(Game_Character::GetMaxStopCountForStep(8) == 0, "StopCountBroken");

static_assert(Game_Character::GetMaxStopCountForTurn(1) == 128, "StopCountBroken");
static_assert(Game_Character::GetMaxStopCountForTurn(2) == 64, "StopCountBroken");
static_assert(Game_Character::GetMaxStopCountForTurn(3) == 32, "StopCountBroken");
static_assert(Game_Character::GetMaxStopCountForTurn(4) == 16, "StopCountBroken");
static_assert(Game_Character::GetMaxStopCountForTurn(5) == 8, "StopCountBroken");
static_assert(Game_Character::GetMaxStopCountForTurn(6) == 4, "StopCountBroken");
static_assert(Game_Character::GetMaxStopCountForTurn(7) == 2, "StopCountBroken");
static_assert(Game_Character::GetMaxStopCountForTurn(8) == 0, "StopCountBroken");

static_assert(Game_Character::GetMaxStopCountForWait(1) == 148, "StopCountBroken");
static_assert(Game_Character::GetMaxStopCountForWait(2) == 84, "StopCountBroken");
static_assert(Game_Character::GetMaxStopCountForWait(3) == 52, "StopCountBroken");
static_assert(Game_Character::GetMaxStopCountForWait(4) == 36, "StopCountBroken");
static_assert(Game_Character::GetMaxStopCountForWait(5) == 28, "StopCountBroken");
static_assert(Game_Character::GetMaxStopCountForWait(6) == 24, "StopCountBroken");
static_assert(Game_Character::GetMaxStopCountForWait(7) == 22, "StopCountBroken");
static_assert(Game_Character::GetMaxStopCountForWait(8) == 20, "StopCountBroken");

// FIXME: Diagonals??
static_assert(Game_Character::GetDirection90DegreeLeft(Game_Character::Up) == Game_Character::Left, "90DegLeftBroken");
static_assert(Game_Character::GetDirection90DegreeLeft(Game_Character::Left) == Game_Character::Down, "90DegLeftBroken");
static_assert(Game_Character::GetDirection90DegreeLeft(Game_Character::Down) == Game_Character::Right, "90DegLeftBroken");
static_assert(Game_Character::GetDirection90DegreeLeft(Game_Character::Right) == Game_Character::Up, "90DegLeftBroken");

// FIXME: Diagonals??
static_assert(Game_Character::GetDirection90DegreeRight(Game_Character::Up) == Game_Character::Right, "90DegRightBroken");
static_assert(Game_Character::GetDirection90DegreeRight(Game_Character::Right) == Game_Character::Down, "90DegRightBroken");
static_assert(Game_Character::GetDirection90DegreeRight(Game_Character::Down) == Game_Character::Left, "90DegRightBroken");
static_assert(Game_Character::GetDirection90DegreeRight(Game_Character::Left) == Game_Character::Up, "90DegRightBroken");

// FIXME: Diagonals??
static_assert(Game_Character::GetDirection180Degree(Game_Character::Up) == Game_Character::Down, "180DegBroken");
static_assert(Game_Character::GetDirection180Degree(Game_Character::Right) == Game_Character::Left, "180DegBroken");
static_assert(Game_Character::GetDirection180Degree(Game_Character::Down) == Game_Character::Up, "180DegBroken");
static_assert(Game_Character::GetDirection180Degree(Game_Character::Left) == Game_Character::Right, "180DegBroken");


static_assert(Game_Character::GetDxFromDirection(Game_Character::Up) == 0, "DxBroken");
static_assert(Game_Character::GetDxFromDirection(Game_Character::Left) == -1, "DxBroken");
static_assert(Game_Character::GetDxFromDirection(Game_Character::Down) == 0, "DxBroken");
static_assert(Game_Character::GetDxFromDirection(Game_Character::Right) == 1, "DxBroken");
static_assert(Game_Character::GetDxFromDirection(Game_Character::UpRight) == 1, "DxBroken");
static_assert(Game_Character::GetDxFromDirection(Game_Character::DownRight) == 1, "DxBroken");
static_assert(Game_Character::GetDxFromDirection(Game_Character::DownLeft) == -1, "DxBroken");
static_assert(Game_Character::GetDxFromDirection(Game_Character::UpLeft) == -1, "DxBroken");

static_assert(Game_Character::GetDyFromDirection(Game_Character::Up) == -1, "DyBroken");
static_assert(Game_Character::GetDyFromDirection(Game_Character::Left) == 0, "DyBroken");
static_assert(Game_Character::GetDyFromDirection(Game_Character::Down) == 1, "DyBroken");
static_assert(Game_Character::GetDyFromDirection(Game_Character::Right) == 0, "DyBroken");
static_assert(Game_Character::GetDyFromDirection(Game_Character::UpRight) == -1, "DyBroken");
static_assert(Game_Character::GetDyFromDirection(Game_Character::DownRight) == 1, "DyBroken");
static_assert(Game_Character::GetDyFromDirection(Game_Character::DownLeft) == 1, "DyBroken");
static_assert(Game_Character::GetDyFromDirection(Game_Character::UpLeft) == -1, "DyBroken");

static_assert(!Game_Character::IsDirectionFixedAnimationType(lcf::rpg::EventPage::AnimType_non_continuous), "DirFixedBroken");
static_assert(!Game_Character::IsDirectionFixedAnimationType(lcf::rpg::EventPage::AnimType_continuous), "DirFixedBroken");
static_assert(Game_Character::IsDirectionFixedAnimationType(lcf::rpg::EventPage::AnimType_fixed_non_continuous), "DirFixedBroken");
static_assert(Game_Character::IsDirectionFixedAnimationType(lcf::rpg::EventPage::AnimType_fixed_continuous), "DirFixedBroken");
static_assert(Game_Character::IsDirectionFixedAnimationType(lcf::rpg::EventPage::AnimType_fixed_graphic), "DirFixedBroken");
static_assert(!Game_Character::IsDirectionFixedAnimationType(lcf::rpg::EventPage::AnimType_spin), "DirFixedBroken");
static_assert(!Game_Character::IsDirectionFixedAnimationType(lcf::rpg::EventPage::AnimType_step_frame_fix), "DirFixedBroken");

#if 0

	/** @return the direction we would need to face the hero. */
	int GetDirectionToHero();

	/** @return the direction we would need to face away from hero. */
	int GetDirectionAwayHero();

	virtual Drawable::Z_t GetScreenZ(bool apply_shift = false) const;

	int DistanceXfromPlayer() const;
	int DistanceYfromPlayer() const;
#endif

TEST_SUITE_BEGIN("Game_Character");

static void testInit(Game_Character& ch, int max_stop_count = 0) {
	REQUIRE_EQ(ch.GetX(), 0);
	REQUIRE_EQ(ch.GetY(), 0);
	REQUIRE_EQ(ch.GetMapId(), 0);
	REQUIRE(!ch.IsFacingLocked());
	REQUIRE(!ch.IsOverlapForbidden());

	// FIXME: Check move route
	REQUIRE_EQ(ch.GetMoveRouteIndex(), 0);
	REQUIRE(!ch.IsMoveRouteOverwritten());
	REQUIRE(!ch.IsMoveRouteFinished());
	REQUIRE_EQ(ch.GetSpriteName(), "");
	REQUIRE(ch.HasTileSprite());
	REQUIRE_EQ(ch.GetSpriteIndex(), 0);
	REQUIRE_EQ(ch.GetAnimFrame(), 1);
	REQUIRE(!ch.IsAnimPaused());
	REQUIRE_EQ(ch.GetFlashColor(), Color());
	REQUIRE_EQ(ch.GetFlashLevel(), 0.0);
	REQUIRE_EQ(ch.GetFlashTimeLeft(), 0);
	REQUIRE_EQ(ch.GetStopCount(), 0);
	REQUIRE_EQ(ch.GetMaxStopCount(), max_stop_count);
	REQUIRE_EQ(ch.IsStopCountActive(), (max_stop_count != 0));
	REQUIRE_EQ(ch.GetAnimCount(), 0);
	REQUIRE(!ch.IsMoving());
	REQUIRE(!ch.IsJumping());
	REQUIRE_EQ(ch.GetBeginJumpX(), 0);
	REQUIRE_EQ(ch.GetBeginJumpY(), 0);
	REQUIRE(!ch.IsFlying());
	REQUIRE(!ch.IsProcessed());
	REQUIRE(!ch.IsPaused());
	REQUIRE(ch.IsActive());
	REQUIRE(ch.IsStopping());
	REQUIRE_EQ(ch.GetJumpHeight(), 0);
	REQUIRE_EQ(ch.GetTileId(), 0);
	REQUIRE_EQ(ch.GetSpriteX(), 0);
	REQUIRE_EQ(ch.GetSpriteY(), 0);
	REQUIRE_EQ(ch.GetRemainingStep(), 0);
	REQUIRE_EQ(ch.GetAnimationType(), Game_Character::AnimType(0));
	// FIXME: Map
	//REQUIRE(ch.IsInPosition(0, 0));

	REQUIRE_EQ(ch.GetOpacity(), 255);
	REQUIRE_EQ(ch.GetTransparency(), 0);
	// FIXME: MAP
	//REQUIRE(ch.IsVisible());
	REQUIRE(!ch.IsSpriteHidden());
	REQUIRE(ch.IsAnimated());
	REQUIRE(!ch.IsContinuous());
	REQUIRE(!ch.IsSpinning());
	// FIXME: MAP
	// REQUIRE_EQ(ch.GetBushDepth(), 0);

}

static void testInitVehicle(Game_Vehicle::Type vt) {
	Game_Vehicle ch(vt);

	testInit(ch);

	REQUIRE_EQ(ch.GetDirection(), 3);
	REQUIRE_EQ(ch.GetFacing(), 3);
	if (vt == Game_Vehicle::Airship) {
		REQUIRE_EQ(ch.GetMoveSpeed(), 5);
	} else {
		REQUIRE_EQ(ch.GetMoveSpeed(), 4);
	}
	REQUIRE(!ch.GetThrough());
	REQUIRE_EQ(ch.GetLayer(), 1);
	REQUIRE_EQ(ch.GetMoveFrequency(), 2);

	REQUIRE_EQ(ch.GetVehicleType(), vt);
	REQUIRE(!ch.IsAscending());
	REQUIRE(!ch.IsDescending());
	REQUIRE(!ch.IsAscendingOrDescending());
	REQUIRE_EQ(ch.GetAltitude(), 0);
	// FIXME: Player
	//REQUIRE(!ch.IsInUse());
	//REQUIRE(!ch.IsAboard());
	// FIXME: Map
	//REQUIRE(!ch.CanLand());
	REQUIRE_EQ(ch.GetOrigSpriteName(), "");
	REQUIRE_EQ(ch.GetOrigSpriteIndex(), 0);

	REQUIRE_EQ(ch.GetMoveRoute(), lcf::rpg::MoveRoute());
}

TEST_CASE("InitVehicle") {
	testInitVehicle(Game_Vehicle::Boat);
	testInitVehicle(Game_Vehicle::Ship);
	testInitVehicle(Game_Vehicle::Airship);
}

TEST_CASE("InitPlayer") {
	Game_Player ch;

	testInit(ch);

	REQUIRE_EQ(ch.GetDirection(), 2);
	REQUIRE_EQ(ch.GetFacing(), 2);
	REQUIRE_EQ(ch.GetMoveSpeed(), 4);
	REQUIRE(!ch.GetThrough());
	REQUIRE_EQ(ch.GetVehicleType(), Game_Vehicle::None);
	REQUIRE_EQ(ch.GetLayer(), 1);
	REQUIRE_EQ(ch.GetMoveFrequency(), 2);

	REQUIRE(!ch.IsPendingTeleport());
	REQUIRE(!ch.InVehicle());
	REQUIRE(!ch.InAirship());
	REQUIRE(!ch.IsAboard());
	REQUIRE(!ch.IsBoardingOrUnboarding());
	REQUIRE_EQ(ch.GetVehicle(), nullptr);

	REQUIRE(!ch.IsEncounterCalling());
	REQUIRE_EQ(ch.GetTotalEncounterRate(), 0);
	REQUIRE(!ch.IsMenuCalling());

	REQUIRE(!ch.IsPanActive());
	REQUIRE(!ch.IsPanLocked());
	REQUIRE_EQ(ch.GetPanX(), lcf::rpg::SavePartyLocation::kPanXDefault);
	REQUIRE_EQ(ch.GetPanY(), lcf::rpg::SavePartyLocation::kPanYDefault);
	REQUIRE_EQ(ch.GetTargetPanX(), lcf::rpg::SavePartyLocation::kPanXDefault);
	REQUIRE_EQ(ch.GetTargetPanY(), lcf::rpg::SavePartyLocation::kPanYDefault);
	REQUIRE_EQ(ch.GetPanWait(), 0);

	REQUIRE(ch.IsMapCompatibleWithSave(0));
	REQUIRE(ch.IsDatabaseCompatibleWithSave(0));
}

TEST_CASE("InitEventNoPage") {
	lcf::rpg::Event event;
	Game_Event ch(0, &event);

	testInit(ch);

	REQUIRE_EQ(ch.GetDirection(), 2);
	REQUIRE_EQ(ch.GetFacing(), 2);
	REQUIRE_EQ(ch.GetMoveSpeed(), 4);
	REQUIRE(ch.GetThrough());
	REQUIRE_EQ(ch.GetLayer(), 1);
	REQUIRE_EQ(ch.GetMoveFrequency(), 2);

	REQUIRE_EQ(ch.GetId(), 0);
	REQUIRE_EQ(ch.GetName(), "");

	REQUIRE(!ch.IsWaitingForegroundExecution());
	REQUIRE(!ch.WasStartedByDecisionKey());
	REQUIRE_EQ(ch.GetTrigger(), -1);
	REQUIRE_EQ(ch.GetOriginalMoveRouteIndex(), 0);
	REQUIRE_EQ(ch.GetPage(1), nullptr);
	REQUIRE_EQ(ch.GetActivePage(), nullptr);
}

TEST_CASE("InitEventDefaultPage") {
	lcf::rpg::Event event;
	event.pages.push_back({});
	event.pages.back().move_type = lcf::rpg::EventPage::MoveType_stationary;
	Game_Event ch(0, &event);

	testInit(ch, 64);

	REQUIRE_EQ(ch.GetDirection(), 2);
	REQUIRE_EQ(ch.GetFacing(), 2);
	REQUIRE_EQ(ch.GetMoveSpeed(), 3);
	REQUIRE(!ch.GetThrough());
	REQUIRE_EQ(ch.GetLayer(), 0);
	REQUIRE_EQ(ch.GetMoveFrequency(), 3);

	REQUIRE_EQ(ch.GetId(), 0);
	REQUIRE_EQ(ch.GetName(), "");

	REQUIRE(!ch.IsWaitingForegroundExecution());
	REQUIRE(!ch.WasStartedByDecisionKey());
	REQUIRE_EQ(ch.GetTrigger(), 0);
	REQUIRE_EQ(ch.GetOriginalMoveRouteIndex(), 0);
	REQUIRE_EQ(ch.GetPage(1), event.pages.data());
	REQUIRE_EQ(ch.GetActivePage(), event.pages.data());
}

static void testBasicSet(Game_Character& ch) {
	ch.SetX(55);
	REQUIRE_EQ(ch.GetX(), 55);

	ch.SetY(66);
	REQUIRE_EQ(ch.GetY(), 66);

	// FIXME: Remove this method.
	ch.SetMapId(44);
	REQUIRE_EQ(ch.GetMapId(), 44);

	ch.SetDirection(Game_Character::UpLeft);
	REQUIRE_EQ(ch.GetDirection(), Game_Character::UpLeft);

	ch.SetFacing(Game_Character::Up);
	REQUIRE_EQ(ch.GetFacing(), Game_Character::Up);

	ch.SetFacingLocked(true);
	REQUIRE(ch.IsFacingLocked());

	ch.SetLayer(2);
	REQUIRE_EQ(ch.GetLayer(), 2);

	ch.SetMoveSpeed(6);
	REQUIRE_EQ(ch.GetMoveSpeed(), 6);

	ch.SetMoveFrequency(7);
	REQUIRE_EQ(ch.GetMoveFrequency(), 7);

	// FIXME: Test move routes
	// Should SetMoveRoute() and SetMoveRouteIndex() be public?
	ch.SetMoveRouteIndex(3);
	REQUIRE_EQ(ch.GetMoveRouteIndex(), 3);

	ch.SetMoveRouteOverwritten(true);
	REQUIRE(ch.IsMoveRouteOverwritten());

	ch.SetSpriteGraphic("SPRITE", 8);
	REQUIRE_EQ(ch.GetSpriteName(), "SPRITE");
	REQUIRE_EQ(ch.GetSpriteIndex(), 8);
	REQUIRE(!ch.HasTileSprite());

	ch.SetAnimFrame(3);
	REQUIRE_EQ(ch.GetAnimFrame(), 3);

	ch.SetAnimPaused(true);
	REQUIRE(ch.IsAnimPaused());

	// FIXME: Test flashing
	ch.SetFlashLevel(99);
	REQUIRE_EQ(ch.GetFlashLevel(), 99);

	ch.SetFlashTimeLeft(99);
	REQUIRE_EQ(ch.GetFlashTimeLeft(), 99);

	ch.SetThrough(true);
	REQUIRE(ch.GetThrough());

	//FIXME: Test ResetThrough

	ch.SetAnimationType(lcf::rpg::EventPage::AnimType_step_frame_fix);
	REQUIRE_EQ(ch.GetAnimationType(), lcf::rpg::EventPage::AnimType_step_frame_fix);

	ch.SetStopCount(78);
	REQUIRE_EQ(ch.GetStopCount(), 78);

	ch.SetMaxStopCount(99);
	REQUIRE_EQ(ch.GetMaxStopCount(), 99);

	// FIXME: Test Stop count IsStopCountActive

	ch.SetAnimCount(78);
	REQUIRE_EQ(ch.GetAnimCount(), 78);

	// FIXME: Test anim count basic increment logic
	// FIXME: Test anim count real

	// FIXME: Test movement sequence
	ch.SetRemainingStep(889);
	REQUIRE_EQ(ch.GetRemainingStep(), 889);

	ch.SetJumping(true);
	REQUIRE(ch.IsJumping());
	REQUIRE(!ch.IsMoving());
	REQUIRE(!ch.IsStopping());

	ch.SetBeginJumpX(77);
	REQUIRE_EQ(ch.GetBeginJumpX(), 77);

	ch.SetBeginJumpY(88);
	REQUIRE_EQ(ch.GetBeginJumpY(), 88);

	ch.SetFlying(true);
	REQUIRE(ch.IsFlying());

	// FIXME: Test Clamp
	ch.SetTransparency(3);
	REQUIRE_EQ(ch.GetTransparency(), 3);

	ch.SetProcessed(true);
	REQUIRE(ch.IsProcessed());

	ch.SetPaused(true);
	REQUIRE(ch.IsPaused());

	ch.SetActive(false);
	REQUIRE(!ch.IsActive());

	ch.SetSpriteHidden(true);
	REQUIRE(ch.IsSpriteHidden());

	// FIXME: Visible test
}

static void testBasicSetVehicle(Game_Vehicle::Type vt) {
	Game_Vehicle ch(vt);

	testBasicSet(ch);

	ch.SetOrigSpriteGraphic("VSPRITE", 9);
	REQUIRE_EQ(ch.GetOrigSpriteName(), "VSPRITE");
	REQUIRE_EQ(ch.GetOrigSpriteIndex(), 9);
}

TEST_CASE("BasicSetVehicle") {
	testBasicSetVehicle(Game_Vehicle::Boat);
	testBasicSetVehicle(Game_Vehicle::Ship);
	testBasicSetVehicle(Game_Vehicle::Airship);
}

TEST_CASE("BasicSetPlayer") {
	Game_Player ch;

	testBasicSet(ch);

	ch.SetMenuCalling(true);
	REQUIRE(ch.IsMenuCalling());

	ch.SetEncounterCalling(true);
	REQUIRE(ch.IsEncounterCalling());

	ch.SetTotalEncounterRate(100);
	REQUIRE_EQ(ch.GetTotalEncounterRate(), 100);

	// FIMXE: Test Panning

	// FIXME: Test save counts
}

TEST_CASE("BasicSetEvent") {
	lcf::rpg::Event event;
	Game_Event ch(0, &event);

	testBasicSet(ch);

	ch.SetOriginalMoveRouteIndex(100);
	REQUIRE_EQ(ch.GetOriginalMoveRouteIndex(), 100);
}

TEST_CASE("TransparencyAndOpacity") {
	Game_Player ch;

	ch.SetTransparency(-1);
	REQUIRE_EQ(ch.GetTransparency(), 0);
	REQUIRE_EQ(ch.GetOpacity(), 255);
	REQUIRE(ch.IsVisible());

	ch.SetTransparency(0);
	REQUIRE_EQ(ch.GetTransparency(), 0);
	REQUIRE_EQ(ch.GetOpacity(), 255);
	REQUIRE(ch.IsVisible());

	ch.SetTransparency(1);
	REQUIRE_EQ(ch.GetTransparency(), 1);
	REQUIRE_EQ(ch.GetOpacity(), 223);
	REQUIRE(ch.IsVisible());

	ch.SetTransparency(2);
	REQUIRE_EQ(ch.GetTransparency(), 2);
	REQUIRE_EQ(ch.GetOpacity(), 191);
	REQUIRE(ch.IsVisible());

	ch.SetTransparency(3);
	REQUIRE_EQ(ch.GetTransparency(), 3);
	REQUIRE_EQ(ch.GetOpacity(), 159);
	REQUIRE(ch.IsVisible());

	ch.SetTransparency(4);
	REQUIRE_EQ(ch.GetTransparency(), 4);
	REQUIRE_EQ(ch.GetOpacity(), 127);
	REQUIRE(ch.IsVisible());

	ch.SetTransparency(5);
	REQUIRE_EQ(ch.GetTransparency(), 5);
	REQUIRE_EQ(ch.GetOpacity(), 95);
	REQUIRE(ch.IsVisible());

	ch.SetTransparency(6);
	REQUIRE_EQ(ch.GetTransparency(), 6);
	REQUIRE_EQ(ch.GetOpacity(), 63);
	REQUIRE(ch.IsVisible());

	ch.SetTransparency(7);
	REQUIRE_EQ(ch.GetOpacity(), 31);
	REQUIRE_EQ(ch.GetTransparency(), 7);
	REQUIRE(ch.IsVisible());

	ch.SetTransparency(8);
	REQUIRE_EQ(ch.GetTransparency(), 7);
	REQUIRE_EQ(ch.GetOpacity(), 31);
	REQUIRE(ch.IsVisible());

	ch.SetTransparency(9);
	REQUIRE_EQ(ch.GetTransparency(), 7);
	REQUIRE_EQ(ch.GetOpacity(), 31);
	REQUIRE(ch.IsVisible());
}

TEST_CASE("TileVsCharset") {
	Game_Player ch;

	REQUIRE(ch.HasTileSprite());
	REQUIRE_EQ(ch.GetTileId(), 0);
	REQUIRE_EQ(ch.GetSpriteName(), "");
	REQUIRE_EQ(ch.GetSpriteIndex(), 0);

	ch.SetSpriteGraphic("x", 2);
	REQUIRE(!ch.HasTileSprite());
	REQUIRE_EQ(ch.GetTileId(), 0);
	REQUIRE_EQ(ch.GetSpriteName(), "x");
	REQUIRE_EQ(ch.GetSpriteIndex(), 2);

	ch.SetSpriteGraphic("", 3);
	REQUIRE(ch.HasTileSprite());
	REQUIRE_EQ(ch.GetTileId(), 3);
	REQUIRE_EQ(ch.GetSpriteName(), "");
	REQUIRE_EQ(ch.GetSpriteIndex(), 3);
}

TEST_CASE("Animated") {
	Game_Player ch;

	REQUIRE(!ch.IsAnimPaused());

	ch.SetAnimationType(lcf::rpg::EventPage::AnimType_non_continuous);
	REQUIRE(ch.IsAnimated());
	REQUIRE(!ch.IsContinuous());
	REQUIRE(!ch.IsSpinning());

	ch.SetAnimationType(lcf::rpg::EventPage::AnimType_continuous);
	REQUIRE(ch.IsAnimated());
	REQUIRE(ch.IsContinuous());
	REQUIRE(!ch.IsSpinning());

	ch.SetAnimationType(lcf::rpg::EventPage::AnimType_fixed_non_continuous);
	REQUIRE(ch.IsAnimated());
	REQUIRE(!ch.IsContinuous());
	REQUIRE(!ch.IsSpinning());

	ch.SetAnimationType(lcf::rpg::EventPage::AnimType_fixed_continuous);
	REQUIRE(ch.IsAnimated());
	REQUIRE(ch.IsContinuous());
	REQUIRE(!ch.IsSpinning());

	ch.SetAnimationType(lcf::rpg::EventPage::AnimType_fixed_graphic);
	REQUIRE(!ch.IsAnimated());
	REQUIRE(!ch.IsContinuous());
	REQUIRE(!ch.IsSpinning());

	ch.SetAnimationType(lcf::rpg::EventPage::AnimType_spin);
	REQUIRE(ch.IsAnimated());
	REQUIRE(!ch.IsContinuous());
	REQUIRE(ch.IsSpinning());

	ch.SetAnimationType(lcf::rpg::EventPage::AnimType_step_frame_fix);
	REQUIRE(!ch.IsAnimated());
	REQUIRE(!ch.IsContinuous());
	REQUIRE(!ch.IsSpinning());

	ch.SetAnimPaused(true);
	REQUIRE(ch.IsAnimPaused());
	for (int i = 0; i <= static_cast<int>(lcf::rpg::EventPage::AnimType_step_frame_fix); ++i) {
		ch.SetAnimationType(static_cast<lcf::rpg::EventPage::AnimType>(i));
		REQUIRE(!ch.IsAnimated());
	}
}


static void testVisible(Game_Character& ch) {
	REQUIRE(ch.IsVisible());

	ch.SetActive(false);
	REQUIRE(!ch.IsVisible());
	ch.SetActive(true);
	REQUIRE(ch.IsVisible());

	ch.SetSpriteHidden(true);
	REQUIRE(!ch.IsVisible());
	ch.SetSpriteHidden(false);
	REQUIRE(ch.IsVisible());
}

TEST_CASE("VisiblePlayer") {
	Game_Player ch;

	testVisible(ch);

	// FIXME: Test aboard
}

TEST_CASE("VisibleVehicle") {
	Game_Vehicle ch(Game_Vehicle::Boat);

	// FIXME: Requires map loaded
	// testVisible(ch);

	// FIXME: Test other map
}

TEST_CASE("VisibleEvent") {
	lcf::rpg::Event event;
	Game_Event ch(0, &event);

	// FIXME: Test pages
	//testVisible(ch);

}

TEST_CASE("LockFacingVsAnimType") {
	for (int i = 0; i <= static_cast<int>(lcf::rpg::EventPage::AnimType_step_frame_fix); ++i) {
		auto at = static_cast<lcf::rpg::EventPage::AnimType>(i);

		Game_Player ch;
		ch.SetAnimationType(at);

		if (Game_Character::IsDirectionFixedAnimationType(at)) {
			REQUIRE(ch.IsFacingLocked());
			ch.SetFacingLocked(false);
			REQUIRE(ch.IsFacingLocked());
			ch.SetFacingLocked(true);
			REQUIRE(ch.IsFacingLocked());
		} else {
			REQUIRE(!ch.IsFacingLocked());
			ch.SetFacingLocked(true);
			REQUIRE(ch.IsFacingLocked());
			ch.SetFacingLocked(false);
			REQUIRE(!ch.IsFacingLocked());
		}
	}
}

TEST_SUITE_END();
