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
#include <cassert>
#include "async_handler.h"
#include "scene.h"
#include "graphics.h"
#include "input.h"
#include "player.h"
#include "output.h"
#include "audio.h"

std::shared_ptr<Scene> Scene::instance;
std::vector<std::shared_ptr<Scene> > Scene::old_instances;
std::vector<std::shared_ptr<Scene> > Scene::instances;
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
	"Logo",
	"Order",
	"GameBrowser",
	"Teleport"
};

enum PushPopOperation {
	ScenePushed = 1,
	ScenePopped
};

int Scene::push_pop_operation = 0;

Scene::Scene() {
	type = Scene::Null;
}

void Scene::MainFunction() {
	static bool init = false;

	if (AsyncHandler::IsImportantFilePending() || Graphics::IsTransitionPending()) {
		Player::Update(false);
	} else if (!init) {
		// Initialization after scene switch
		switch (push_pop_operation) {
		case ScenePushed:
			Start();
			initialized = true;
			break;
		case ScenePopped:
			if (!initialized) {
				Start();
				initialized = true;
			} else {
				Continue();
			}
			break;
		default:;
		}

		push_pop_operation = 0;

		TransitionIn();
		Resume();

		init = true;

		return;
	} else {
		Player::Update();
	}

	if (Scene::instance.get() != this) {
		// Shutdown after scene switch
		assert(Scene::instance == instances.back() &&
			"Don't set Scene::instance directly, use Push instead!");

		Graphics::Update(true);

		Suspend();
		TransitionOut();

		switch (push_pop_operation) {
		case ScenePushed:
			Graphics::Push(Scene::instance->DrawBackground());
			break;
			// Graphics::Pop done in Player Loop
		default:;
		}

		init = false;
	}
}

void Scene::Start() {
}

void Scene::Continue() {
}

void Scene::Resume() {
}

void Scene::Suspend() {
}

void Scene::TransitionIn() {
	Graphics::Transition(Graphics::TransitionFadeIn, 6);
}

void Scene::TransitionOut() {
	Graphics::Transition(Graphics::TransitionFadeOut, 6, true);
}

void Scene::Update() {
}

void Scene::Push(std::shared_ptr<Scene> const& new_scene, bool pop_stack_top) {
	if (pop_stack_top) {
		old_instances.push_back(instances.back());
		instances.pop_back();
	}

	instances.push_back(new_scene);
	instance = new_scene;

	push_pop_operation = ScenePushed;

	/*Output::Debug("Scene Stack after Push:");
	for (size_t i = 0; i < instances.size(); ++i) {
		Output::Debug(scene_names[instances[i]->type]);
	}*/
}

void Scene::Pop() {
	old_instances.push_back(instances.back());
	instances.pop_back();

	if (instances.size() == 0) {
		Push(std::make_shared<Scene>()); // Null-scene
	} else {
		instance = instances.back();
	}

	push_pop_operation = ScenePopped;

	/*Output::Debug("Scene Stack after Pop:");
	for (size_t i = 0; i < instances.size(); ++i) {
		Output::Debug(scene_names[instances[i]->type]);
	}*/
}

void Scene::PopUntil(SceneType type) {
	int count = 0;

	for (int i = (int)instances.size() - 1 ; i >= 0; --i) {
		if (instances[i]->type == type) {
			for (i = 0; i < count; ++i) {
				old_instances.push_back(instances.back());
				instances.pop_back();
			}
			instance = instances.back();
			push_pop_operation = ScenePopped;
			return;
		}
		++count;
	}

	Output::Warning("The requested scene %s was not on the stack", scene_names[type]);
}

std::shared_ptr<Scene> Scene::Find(SceneType type) {
	std::vector<std::shared_ptr<Scene> >::const_reverse_iterator it;
	for (it = instances.rbegin() ; it != instances.rend(); ++it) {
		if ((*it)->type == type) {
			return *it;
		}
	}

	return std::shared_ptr<Scene>();
}

bool Scene::DrawBackground() {
	return true;
}
