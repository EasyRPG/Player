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
#include "scene_settings.h"
#include "audio.h"
#include "input.h"
#include "game_system.h"
#include "cache.h"
#include "main_data.h"
#include "player.h"
#include "baseui.h"
#include "output.h"
#include "utils.h"
#include "window_help.h"
#include "window_numberinput.h"
#include "window_settings.h"

constexpr int option_window_num_items = 10;

Scene_Settings::Scene_Settings() {
	Scene::type = Scene::Settings;
}

void Scene_Settings::CreateTitleGraphic() {
	// Load Title Graphic
	if (lcf::Data::system.title_name.empty()) {
		return;
	}
	title = std::make_unique<Sprite>();
	FileRequestAsync* request = AsyncHandler::RequestFile("Title", lcf::Data::system.title_name);
	request->SetGraphicFile(true);
	request_id = request->Bind(&Scene_Settings::OnTitleSpriteReady, this);
	request->Start();
}

void Scene_Settings::CreateMainWindow() {
	std::vector<std::string> options = {
		"Video",
		"Audio",
		"Input",
		"License",
		"<Save>"
	};
	main_window = std::make_unique<Window_Command>(std::move(options), 96);
	main_window->SetHeight(176);
	main_window->SetY(32);
	main_window->SetX((SCREEN_TARGET_WIDTH - main_window->GetWidth()) / 2);
}

void Scene_Settings::CreateOptionsWindow() {
	help_window.reset(new Window_Help(0, 0, SCREEN_TARGET_WIDTH, 32));
	options_window = std::make_unique<Window_Settings>(32, 32, SCREEN_TARGET_WIDTH - 64, 176);
	options_window->SetHelpWindow(help_window.get());
}

void Scene_Settings::Start() {
	CreateTitleGraphic();
	CreateMainWindow();
	CreateOptionsWindow();

	options_window->Push(Window_Settings::eMain);
	SetMode(Window_Settings::eMain);
}

void Scene_Settings::SetMode(Window_Settings::UiMode new_mode) {
	if (new_mode == mode) {
		return;
	}
	mode = new_mode;

	main_window->SetActive(false);
	main_window->SetVisible(false);
	options_window->SetActive(false);
	options_window->SetVisible(false);
	help_window->SetVisible(false);

	picker_window.reset();

	switch (mode) {
		case Window_Settings::eMain:
			main_window->SetActive(true);
			main_window->SetVisible(true);
			break;
		default:
			help_window->SetVisible(true);
			options_window->SetActive(true);
			options_window->SetVisible(true);
			break;
	}
}

void Scene_Settings::Update() {
	main_window->Update();
	help_window->Update();
	options_window->Update();

	SetMode(options_window->GetMode());
	if (mode != Window_Settings::eInputRemap && Input::IsTriggered(Input::CANCEL)) {
		Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Game_System::SFX_Cancel));

		if (number_window) {
			number_window.reset();
			auto& option = options_window->GetCurrentOption();
			option.current_value = option.original_value;
			option.action();
			options_window->SetActive(true);
			return;
		}

		if (picker_window) {
			picker_window.reset();
			auto& option = options_window->GetCurrentOption();
			option.current_value = option.original_value;
			option.action();
			options_window->SetActive(true);
			return;
		}

		options_window->Pop();
		SetMode(options_window->GetMode());
		if (mode == Window_Settings::eNone) {
			Scene::Pop();
		}
	}

	switch (options_window->GetMode()) {
		case Window_Settings::eNone:
			break;
		case Window_Settings::eMain:
			UpdateMain();
			break;
		case Window_Settings::eInput:
		case Window_Settings::eVideo:
		case Window_Settings::eAudio:
		case Window_Settings::eLicense:
		case Window_Settings::eInputButton:
		case Window_Settings::eInputMapping:
			UpdateOptions();
			break;
		case Window_Settings::eInputRemap:
			options_window->UpdateMode();
			break;
		case Window_Settings::eLastMode:
			assert(false);
	}
}

void Scene_Settings::OnTitleSpriteReady(FileRequestResult* result) {
	title->SetBitmap(Cache::Title(result->file));
}

void Scene_Settings::UpdateMain() {
	const auto modes = Utils::MakeArray(
		Window_Settings::eVideo,
		Window_Settings::eAudio,
		Window_Settings::eInput,
		Window_Settings::eLicense,
		Window_Settings::eSave
	);

	if (Input::IsTriggered(Input::DECISION)) {
		Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Game_System::SFX_Decision));
		auto idx = main_window->GetIndex();

		if (modes[idx] == Window_Settings::eSave) {
			UpdateSave();
			return;
		}

		SetMode(modes[idx]);
		options_window->Push(modes[idx]);
	}
}

void Scene_Settings::UpdateOptions() {
	options_window->UpdateMode();

	if (number_window) {
		number_window->Update();
		auto& option = options_window->GetCurrentOption();
		option.current_value = Utils::Clamp(number_window->GetNumber(), option.min_value, option.max_value);
		option.action();

		if (Input::IsTriggered(Input::DECISION)) {
			options_window->Refresh();
			number_window.reset();
			options_window->SetActive(true);
		}
		return;
	} else if (picker_window) {
		picker_window->Update();
		auto& option = options_window->GetCurrentOption();
		option.current_value = picker_window->GetIndex();
		option.action();

		if (Input::IsTriggered(Input::DECISION)) {
			options_window->Refresh();
			picker_window.reset();
			options_window->SetActive(true);
		}
		return;
	}

	if (Input::IsTriggered(Input::DECISION)) {
		if (options_window->IsCurrentActionEnabled()) {
			Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Game_System::SFX_Decision));
			auto& option = options_window->GetCurrentOption();
			if (option.mode == Window_Settings::eOptionNone) {
				option.action();
				options_window->Refresh();
			} else if (option.mode == Window_Settings::eOptionRangeInput) {
				number_window.reset(new Window_NumberInput(100, 100, 128, 32));
				number_window->SetNumber(option.current_value);
				number_window->SetMaxDigits(std::log10(option.max_value) + 1);
				number_window->SetZ(options_window->GetZ() + 1);
				number_window->SetOpacity(255);
				number_window->SetActive(true);
				help_window->SetText(fmt::format("Input a value from {} to {}", option.min_value, option.max_value));
				options_window->SetActive(false);
			} else if (option.mode == Window_Settings::eOptionPicker) {
				picker_window.reset(new Window_Command(option.options_text));
				picker_window->SetX(100);
				picker_window->SetY(100);
				picker_window->SetZ(options_window->GetZ() + 1);
				picker_window->SetIndex(option.current_value);
				picker_window->SetHelpWindow(help_window.get());
				picker_window->SetActive(true);
				options_window->SetActive(false);
				picker_window->UpdateHelpFn = [this](Window_Help& win, int index) {
					win.SetText(options_window->GetCurrentOption().options_help[index]);
				};
			}
		} else {
			Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Game_System::SFX_Buzzer));
		}
	}

	if (Input::IsTriggered(Input::LEFT) || Input::IsRepeated(Input::LEFT)) {
		Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Game_System::SFX_Cursor));
		if (options_window->IsCurrentActionEnabled()) {
			auto& option = options_window->GetCurrentOption();
			if (option.mode == Window_Settings::eOptionRangeInput) {
				--option.current_value;
				if (option.current_value < option.min_value) {
					option.current_value = option.max_value;
				}
				option.action();
			} else if (option.mode == Window_Settings::eOptionPicker) {
				--option.current_value;
				if (option.current_value < 0) {
					option.current_value = static_cast<int>(option.options_text.size() - 1);
				}
			}
			option.action();
			options_window->Refresh();
		}
	}

	if (Input::IsTriggered(Input::RIGHT) || Input::IsRepeated(Input::RIGHT)) {
		Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Game_System::SFX_Cursor));
		if (options_window->IsCurrentActionEnabled()) {
			auto& option = options_window->GetCurrentOption();
			if (option.mode == Window_Settings::eOptionRangeInput) {
				++option.current_value;
				if (option.current_value > option.max_value) {
					option.current_value = option.min_value;
				}
				option.action();
			} else if (option.mode == Window_Settings::eOptionPicker) {
				++option.current_value;
				if (option.current_value >= static_cast<int>(option.options_text.size())) {
					option.current_value = 0;
				}
			}
			option.action();
			options_window->Refresh();
		}
	}
}

void Scene_Settings::UpdateSave() {
	auto cfg_out = Game_Config::GetGlobalConfigFileOutput();

	if (!cfg_out) {
		Output::Warning("Saving configuration file failed!");
		return;
	}

	Game_Config cfg;
	cfg.video = DisplayUi->GetConfig();
	cfg.audio = DisplayUi->GetAudio().GetConfig();
	cfg.input = Input::GetInputSource()->GetConfig();
	cfg.player = Player::player_config;
	cfg.WriteToStream(cfg_out);

	Output::Info("Configuration saved");
}

void Scene_Settings::DrawBackground(Bitmap& dst) {
	if (!title || !title->GetBitmap()) {
		Scene::DrawBackground(dst);
	}
}

