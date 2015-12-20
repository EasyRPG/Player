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
#include <boost/scoped_ptr.hpp>
#include <cstdlib>
#include "main_data.h"
#include "game_actors.h"
#include "game_party.h"
#include "game_enemyparty.h"
#include "game_player.h"
#include "game_screen.h"
#include "game_map.h"
#include "game_variables.h"
#include "game_switches.h"
#include "font.h"

#ifdef __ANDROID__
	#include <jni.h>
	#include <SDL_system.h>
#endif

#ifdef GEKKO
	#include <unistd.h>
#endif

// Global variables.

Game_Variables_Class Game_Variables(Main_Data::game_data.system.variables);
Game_Switches_Class Game_Switches(Main_Data::game_data.system.switches);

static std::string GetProjectPath() {
#ifdef __ANDROID__
	// Invoke "String getProjectPath()" in EasyRPG Activity via JNI
	JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
	jobject sdl_activity = (jobject)SDL_AndroidGetActivity();
	jclass cls = env->GetObjectClass(sdl_activity);
	jmethodID jni_getProjectPath = env->GetMethodID(cls , "getProjectPath", "()Ljava/lang/String;");
	jstring return_string = (jstring)env->CallObjectMethod(sdl_activity, jni_getProjectPath);
	
	const char *js = env->GetStringUTFChars(return_string, NULL);
	std::string cs(js);

	env->ReleaseStringUTFChars(return_string, js);
	env->DeleteLocalRef(sdl_activity);
	env->DeleteLocalRef(cls);

	return cs;
#elif GEKKO
	// Wii doesn't provide a correct working directory before mounting
	char gekko_dir[256];
	getcwd(gekko_dir, 255);
	return std::string(gekko_dir);
#else
	return std::string(".");
#endif
}

namespace Main_Data {
	// Dynamic Game Data
	boost::scoped_ptr<Game_Screen> game_screen;
	boost::scoped_ptr<Game_Player> game_player;
	boost::scoped_ptr<Game_Party> game_party;
	boost::scoped_ptr<Game_EnemyParty> game_enemyparty;

	RPG::Save game_data;
	std::string project_path;
}

void Main_Data::Init() {
	project_path =
		getenv("RPG_TEST_GAME_PATH")? getenv("RPG_TEST_GAME_PATH"):
		getenv("RPG_GAME_PATH")? getenv("RPG_GAME_PATH"):
		GetProjectPath();
}

void Main_Data::Cleanup() {
	Game_Map::Quit();
	Game_Actors::Dispose();

	game_screen.reset();
	game_player.reset();
	game_party.reset();
	game_enemyparty.reset();

	game_data = RPG::Save();
}
