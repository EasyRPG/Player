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
#include <cassert>
#include "scene.h"
#include "graphics.h"
#include "input.h"
#include "player.h"
#include "output.h"

#ifndef NULL
#define NULL 0
#endif

////////////////////////////////////////////////////////////
Scene* Scene::instance;
std::vector<Scene*> Scene::old_instances;
std::vector<Scene*> Scene::instances;
const char Scene::scene_names[SceneMax][12] =
{
	"Null",
	"Title",
	"Map",
	"Menu",
	"Item",
	"Skill",
	"Equip",
	"ActorTarget",
	"Status",
	"File",
	"Save",
	"Load",
	"End",
	"Battle",
	"Shop",
	"Name",
	"Gameover",
	"Debug",
	"Logo"
};

////////////////////////////////////////////////////////////
Scene::Scene() {
	type = Scene::Null;
}

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

#ifdef _DEBUG
	assert(Scene::instance == instances[instances.size() - 1] &&
		"Don't set Scene::instance directly, use Push instead!");
#endif

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
}

////////////////////////////////////////////////////////////
void Scene::PerformTransition() {
	static bool faded_in = false;
	if (!faded_in) {
		Graphics::Transition(Graphics::TransitionFadeIn, 24);
		faded_in = true;
	} else {
		Graphics::Transition(Graphics::TransitionFadeOut, 24, true);
		faded_in = false;
	}
}

////////////////////////////////////////////////////////////
void Scene::Update() {
}

////////////////////////////////////////////////////////////
void Scene::Push(Scene* new_scene, bool pop_stack_top) {
	if (pop_stack_top) {
		old_instances.push_back(instances[instances.size() - 1]);
		instances.pop_back();
	}

	instances.push_back(new_scene);
	instance = new_scene;

	/*Output::Debug("Scene Stack after Push:");
	for (size_t i = 0; i < instances.size(); ++i) {
		Output::Debug(scene_names[instances[i]->type]);
	}*/
}

////////////////////////////////////////////////////////////
void Scene::Pop() {
	old_instances.push_back(instances[instances.size() - 1]);
	instances.pop_back();

	if (instances.size() == 0) {
		Push(new Scene()); // Null-scene
	} else {
		instance = instances[instances.size() - 1];
	}

	/*Output::Debug("Scene Stack after Pop:");
	for (size_t i = 0; i < instances.size(); ++i) {
		Output::Debug(scene_names[instances[i]->type]);
	}*/
}

////////////////////////////////////////////////////////////
void Scene::PopUntil(SceneType type) {
	int count = 0;

	for (int i = (int)instances.size() - 1 ; i >= 0; --i) {
		if (instances[i]->type == type) {
			for (i = 0; i < count; ++i) {
				old_instances.push_back(instances[instances.size() - 1]);
				instances.pop_back();
			}
			instance = instances[instances.size() - 1];
			return;
		}
		++count;
	}

	Output::Warning("The requested scene %s was not on the stack", scene_names[type]);
}

////////////////////////////////////////////////////////////
Scene* Scene::Find(SceneType type) {
	std::vector<Scene*>::const_reverse_iterator it;
	for (it = instances.rbegin() ; it != instances.rend(); it++)
		if ((*it)->type == type)
			return *it;

	Output::Warning("The searched scene %s was not on the stack", scene_names[type]);
	return NULL;
}
