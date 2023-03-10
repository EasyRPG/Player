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
#include "scene_equip.h"
#include "game_actors.h"
#include "game_party.h"
#include "game_system.h"
#include "game_screen.h"
#include "input.h"
#include "player.h"
#include <lcf/reader_util.h>
#include "scene_menu.h"
#include <lcf/rpg/item.h>

Scene_Equip::Scene_Equip(Game_Actor& actor, int equip_index) :
	actor(actor),
	equip_index(equip_index) {
	type = Scene::Equip;
}

void Scene_Equip::Start() {
	// Create the windows
	int menu_help_height = 32;
	int menu_equip_status_width = 124;
	int menu_equip_status_height = 96;
	int menu_equip_height = 96;

	help_window.reset(new Window_Help(Player::menu_offset_x, Player::menu_offset_y, MENU_WIDTH, menu_help_height));
	equipstatus_window.reset(new Window_EquipStatus(Player::menu_offset_x, Player::menu_offset_y + menu_help_height, menu_equip_status_width, menu_equip_status_height, actor.GetId()));
	equip_window.reset(new Window_Equip(Player::menu_offset_x + menu_equip_status_width, Player::menu_offset_y + menu_help_height, (MENU_WIDTH - menu_equip_status_width), menu_equip_height, actor.GetId()));

	equip_window->SetIndex(equip_index);

	for (int i = 0; i < 5; ++i) {
		item_windows.push_back(std::make_shared<Window_EquipItem>(Player::menu_offset_x, Player::menu_offset_y + menu_help_height + menu_equip_status_height, MENU_WIDTH, MENU_HEIGHT - menu_help_height - menu_equip_status_height,actor.GetId(), i));
	}

	// Assign the help windows
	for (size_t i = 0; i < item_windows.size(); ++i) {
		item_windows[i]->SetVisible((unsigned)equip_window->GetIndex() == i);
		item_windows[i]->SetHelpWindow(help_window.get());
		item_windows[i]->SetActive(false);
		item_windows[i]->Refresh();
	}
	equip_window->SetHelpWindow(help_window.get());
}

void Scene_Equip::vUpdate() {
	help_window->Update();

	UpdateEquipWindow();
	UpdateStatusWindow();
	UpdateItemWindows();

	if (equip_window->GetActive()) {
		UpdateEquipSelection();
	} else if (item_window->GetActive()) {
		UpdateItemSelection();
	}
}

void Scene_Equip::UpdateItemWindows() {
	for (size_t i = 0; i < item_windows.size(); ++i) {
		item_windows[i]->SetVisible((unsigned)equip_window->GetIndex() == i);
		item_windows[i]->Update();
	}

	item_window = item_windows[equip_window->GetIndex()];
}

void Scene_Equip::UpdateEquipWindow() {
	equip_window->Update();
}

void Scene_Equip::UpdateStatusWindow() {
	if (equip_window->GetActive()) {
		equipstatus_window->ClearParameters();
	} else if (item_window->GetActive()) {
		const lcf::rpg::Item* current_item = item_window->GetItem();

		const auto eidx = equip_window->GetIndex();

		auto atk = actor.GetBaseAtk(Game_Battler::WeaponAll, true, false);
		auto def = actor.GetBaseDef(Game_Battler::WeaponAll, true, false);
		auto spi = actor.GetBaseSpi(Game_Battler::WeaponAll, true, false);
		auto agi = actor.GetBaseAgi(Game_Battler::WeaponAll, true, false);

		auto add_item = [&](const lcf::rpg::Item* item, int mod = 1) {
			if (item) {
				atk += item->atk_points1 * mod;
				def += item->def_points1 * mod;
				spi += item->spi_points1 * mod;
				agi += item->agi_points1 * mod;
			}
		};

		for (int i = 1; i <= 5; i++) {
			auto* count_item = actor.GetEquipment(i);
			add_item(count_item, 1);
		}

		auto* old_item = actor.GetEquipment(eidx + 1);
		// If its a weapon or shield, get the other hand
		const lcf::rpg::Item* other_old_item = nullptr;
		if (eidx == 0) {
			other_old_item = actor.GetEquipment(eidx + 2);
		} else if (eidx == 1) {
			other_old_item = actor.GetEquipment(eidx);
		}

		add_item(old_item, -1);
		// If other hand had a two handed weapon, or we considering a 2 handed weapon, remove the other hand.
		if (current_item && other_old_item &&
				((other_old_item->type == lcf::rpg::Item::Type_weapon && other_old_item->two_handed) || (current_item->type == lcf::rpg::Item::Type_weapon && current_item->two_handed))) {
			add_item(other_old_item, -1);
		}
		add_item(current_item, 1);

		int limit = actor.MaxStatBaseValue();

		atk = Utils::Clamp(atk, 1, limit);
		def = Utils::Clamp(def, 1, limit);
		spi = Utils::Clamp(spi, 1, limit);
		agi = Utils::Clamp(agi, 1, limit);

		atk = actor.CalcValueAfterAtkStates(atk);
		def = actor.CalcValueAfterDefStates(def);
		spi = actor.CalcValueAfterSpiStates(spi);
		agi = actor.CalcValueAfterAgiStates(agi);

		equipstatus_window->SetNewParameters(atk, def, spi, agi);

		equipstatus_window->Refresh();
	}

	equipstatus_window->Update();
}

static bool CanRemoveEquipment(const Game_Actor& actor, int index) {
	if (actor.IsEquipmentFixed(true)) {
		return false;
	}
	auto* item = actor.GetEquipment(index + 1);
	if (item && item->cursed) {
		return false;
	}
	return true;
}

void Scene_Equip::UpdateEquipSelection() {
	if (Input::IsTriggered(Input::CANCEL)) {
		Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cancel));
		Scene::Pop();
	} else if (Input::IsTriggered(Input::DECISION)) {
		if (!CanRemoveEquipment(actor, equip_window->GetIndex())) {
			Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Buzzer));
			return;
		}

		Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));
		equip_window->SetActive(false);
		item_window->SetActive(true);
		item_window->SetIndex(0);
	} else if (Main_Data::game_party->GetActors().size() > 1 && Input::IsTriggered(Input::RIGHT)) {
		Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cursor));
		int actor_index = Main_Data::game_party->GetActorPositionInParty(actor.GetId());
		actor_index = (actor_index + 1) % Main_Data::game_party->GetActors().size();
		Scene::Push(std::make_shared<Scene_Equip>((*Main_Data::game_party)[actor_index], equip_window->GetIndex()), true);
	} else if (Main_Data::game_party->GetActors().size() > 1 && Input::IsTriggered(Input::LEFT)) {
		Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cursor));
		int actor_index = Main_Data::game_party->GetActorPositionInParty(actor.GetId());
		actor_index = (actor_index + Main_Data::game_party->GetActors().size() - 1) % Main_Data::game_party->GetActors().size();
		Scene::Push(std::make_shared<Scene_Equip>((*Main_Data::game_party)[actor_index], equip_window->GetIndex()), true);
	}
}

void Scene_Equip::UpdateItemSelection() {
	if (Input::IsTriggered(Input::CANCEL)) {
		Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cancel));
		equip_window->SetActive(true);
		item_window->SetActive(false);
		item_window->SetIndex(-1);
	} else if (Input::IsTriggered(Input::DECISION)) {
		Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));

		const lcf::rpg::Item* current_item = item_window->GetItem();
		int current_item_id = current_item ? current_item->ID : 0;

		actor.ChangeEquipment(
			equip_window->GetIndex() + 1, current_item_id);

		equip_window->SetActive(true);
		item_window->SetActive(false);
		item_window->SetIndex(-1);

		equip_window->Refresh();

		for (size_t i = 0; i < item_windows.size(); ++i) {
			item_windows[i]->Refresh();
		}
	}
}
