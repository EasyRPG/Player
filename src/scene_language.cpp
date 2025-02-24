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
#include "scene_language.h"
#include "scene_logo.h"
#include "audio.h"
#include "bitmap.h"
#include "input.h"
#include "game_system.h"
#include "cache.h"
#include "input_buttons.h"
#include "input_source.h"
#include "keys.h"
#include "main_data.h"
#include "options.h"
#include "player.h"
#include "baseui.h"
#include "output.h"
#include "scene_title.h"
#include "utils.h"
#include "scene_end.h"
#include "window_about.h"
#include "window_command_horizontal.h"
#include "window_help.h"
#include "window_input_settings.h"
#include "window_numberinput.h"
#include "window_selectable.h"
#include "window_settings.h"
#include <memory>

Scene_Language::Scene_Language() {
	Scene::type = Scene::LanguageMenu;
}

void Scene_Language::CreateTitleGraphic() {
	// Load Title Graphic
	if (lcf::Data::system.title_name.empty()) {
		return;
	}
	title = std::make_unique<Sprite>();
	FileRequestAsync* request = AsyncHandler::RequestFile("Title", lcf::Data::system.title_name);
	request->SetGraphicFile(true);
	request_id = request->Bind(&Scene_Language::OnTitleSpriteReady, this);
	request->Start();
}


void Scene_Language::CreateTranslationWindow() {
	// Build a list of 'Default' and all known languages.
	std::vector<std::string> lang_names;
	lang_names.push_back("Default Language");
	lang_dirs.push_back("");
	lang_helps.push_back("Play the game in its original language.");

	// Push menu entries with the display name, but also save the directory location and help text.
	for (const Language& lg : Player::translation.GetLanguages()) {
		lang_names.push_back(lg.lang_name);
		lang_dirs.push_back(lg.lang_dir);
		lang_helps.push_back(lg.lang_desc);
	}

	// Allow overwriting text of the default language
	const Language& def = Player::translation.GetDefaultLanguage();
	if (!def.lang_name.empty()) {
		lang_names.front() = def.lang_name;
	}
	if (!def.lang_desc.empty()) {
		lang_helps.front() = def.lang_desc;
	}

	translate_window = std::make_unique<Window_Command>(lang_names, -1, lang_names.size() > 9 ? 9 : lang_names.size());
	translate_window->UpdateHelpFn = [this](Window_Help& win, int index) {
		if (index >= 0 && index < static_cast<int>(lang_helps.size())) {
			win.SetText(lang_helps[index]);
		}
		else {
			win.SetText("");
		}
	};
	Scene_Title::RepositionWindow(*translate_window, Player::hide_title_flag);

	if (Player::IsRPG2k3E() && lcf::Data::battlecommands.transparency == lcf::rpg::BattleCommands::Transparency_transparent) {
		translate_window->SetBackOpacity(160);
	}

	translate_window->SetVisible(false);
}

void Scene_Language::CreateHelpWindow() {
	help_window.reset(new Window_Help(0, 0, Player::screen_width, 32));

	if (Player::IsRPG2k3E() && lcf::Data::battlecommands.transparency == lcf::rpg::BattleCommands::Transparency_transparent) {
		help_window->SetBackOpacity(160);
	}

	help_window->SetVisible(false);
	translate_window->SetHelpWindow(help_window.get());
}

void Scene_Language::Start() {
	CreateTitleGraphic();
	CreateTranslationWindow();
	CreateHelpWindow();

	translate_window->SetActive(true);
	translate_window->SetVisible(true);
	help_window->SetVisible(true);
}


void Scene_Language::vUpdate() {
	if (shutdown) {
		PopOrTitle();
		return;
	}

	translate_window->Update();
	help_window->Update();


	if (Input::IsTriggered(Input::DECISION)) {
		int index = translate_window->GetIndex();
		ChangeLanguage(lang_dirs.at(index));
	}
	else if (Input::IsTriggered(Input::CANCEL)) {
		Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cancel));

		auto peek_scene = Scene::Peek();
		if (!peek_scene || peek_scene->type == SceneType::Null || peek_scene->type == SceneType::Logo) {
			Transition::instance().InitErase(Transition::TransitionFadeOut, this);
		}
		Scene::Pop();
	}
}

void Scene_Language::OnTranslationChanged() {
	Main_Data::game_system->ReloadSystemGraphic();
}

void Scene_Language::OnTitleSpriteReady(FileRequestResult* result) {
	BitmapRef bitmapRef = Cache::Title(result->file);

	title->SetBitmap(bitmapRef);

	// If the title sprite doesn't fill the screen, center it to support custom resolutions
	if (bitmapRef->GetWidth() < Player::screen_width) {
		title->SetX(Player::menu_offset_x);
	}
	if (bitmapRef->GetHeight() < Player::screen_height) {
		title->SetY(Player::menu_offset_y);
	}
}

void Scene_Language::ChangeLanguage(const std::string& lang_str) {
	Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));

	// No-op?
	if (lang_str == Player::translation.GetCurrentLanguage().lang_dir) {
		PopOrTitle();
		return;
	}

	// First change the language
	Player::translation.SelectLanguage(lang_str);
	Main_Data::game_system->ReloadSystemGraphic();

	// Delay scene shutdown by one frame to allow async requests for language change
	// and system graphic reload to finish before the scene switches
	shutdown = true;
}

void Scene_Language::PopOrTitle() {
	if (!Find(Title)) {
		Scene::Push(std::make_shared<Scene_Title>(), true);
	} else {
		Scene::Pop();
	}
}
