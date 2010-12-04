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
#include "scene.h"
#include "graphics.h"
#include "input.h"
#include "player.h"

#ifndef NULL
#define NULL 0
#endif

////////////////////////////////////////////////////////////
Scene* Scene::instance;
Scene::SceneType Scene::type;
Scene* Scene::old_instance = NULL;

////////////////////////////////////////////////////////////
void Scene::MainFunction() {
	Start();
	PerformTransition();
	PostStart();

	// Scene loop
	while (Scene::instance == this) {
		Player::Update();
		Graphics::Update();
		Input::Update();
		Update();
	}
	Graphics::Update();

	PreTerminate();
	PerformTransition();
	Terminate();
}

////////////////////////////////////////////////////////////
void Scene::Start() {
}

////////////////////////////////////////////////////////////
void Scene::PostStart() {
}

////////////////////////////////////////////////////////////
void Scene::PreTerminate() {
}

////////////////////////////////////////////////////////////
void Scene::Terminate() {
	Scene::old_instance = this;
}

////////////////////////////////////////////////////////////
void Scene::PerformTransition() {
	static bool faded_in = false;
	if (!faded_in) {
		Graphics::Transition(Graphics::FadeIn, 20, true);
		faded_in = true;
	} else {
		Graphics::Transition(Graphics::FadeOut, 20, false);
		faded_in = false;
	}
}

////////////////////////////////////////////////////////////
void Scene::Update() {
}
