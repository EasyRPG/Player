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
#include "transition.h"
#include "game_interpreter.h"
#include "game_system.h"

#ifndef NDEBUG
#define DEBUG_VALIDATE(x) Scene::DebugValidate(x)
#else
#define DEBUG_VALIDATE(x) do {} while(0)
#endif


constexpr int Scene::kStartGameDelayFrames;
constexpr int Scene::kReturnTitleDelayFrames;
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

void Scene::ScheduleTransitionIn(Scene::SceneType prev_scene_type) {
	if (!Transition::instance().IsErasedNotActive()) {
		// Scene could have manually triggered transition earlier
		return;
	}

	// If Start() or Continue() produced an async operation, defer TransitionIn() call until
	// after async completes
	if (async_continuation) {
		AsyncNext([this,fn=std::move(async_continuation),prev_scene_type]() {
					fn();
					ScheduleTransitionIn(prev_scene_type);
				});
	} else {
		AsyncNext([this,prev_scene_type]() { TransitionIn(prev_scene_type); });
	}
}

void Scene::MainFunction() {
	static bool init = false;

	if (IsAsyncPending()) {
		Player::Update(false);
		return;
	} else {
		// This is used to provide a hook for Scene_Map to finish
		// it's PreUpdate() and teleport logic after transition
		// or asynchronous file load.
		OnFinishAsync();
	}

	// The continuation could have caused a new async wait condition, or
	// it could have changed the scene.
	if (!IsAsyncPending() && Scene::instance.get() == this) {
		if (!init) {
			auto prev_scene = Graphics::UpdateSceneCallback();
			auto prev_scene_type = prev_scene ? prev_scene->type : Null;

			// Destroy the previous scene here, before any initialization logic / transition in occurs.
			prev_scene.reset();

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
						Continue(prev_scene_type);
					}
					break;
				default:;
			}

			push_pop_operation = 0;

			ScheduleTransitionIn(prev_scene_type);

			init = true;
			return;
		} else {
			Player::Update();
		}
	}

	if (Scene::instance.get() != this) {
		// Shutdown after scene switch
		assert(Scene::instance == instances.back() &&
			"Don't set Scene::instance directly, use Push instead!");

		Graphics::Update();

		auto next_scene = instance ? instance->type : Null;
		Suspend(next_scene);
		// Scene could have manually triggered transition earlier
		if (!Transition::instance().IsActive()) {
			TransitionOut(next_scene);
		}

		init = false;
	}
}

void Scene::Start() {
}

void Scene::Continue(SceneType /* prev_scene */) {
}

void Scene::Suspend(SceneType /* next_scene */) {
}

void Scene::TransitionIn(SceneType) {
	Transition::instance().InitShow(Transition::TransitionFadeIn, this, 6);
}

void Scene::TransitionOut(SceneType) {
	Transition::instance().InitErase(Transition::TransitionFadeOut, this, 6);
}

void Scene::SetAsyncFromMainLoop() {
	was_async_from_main_loop = true;
}

void Scene::OnFinishAsync() {
	if (async_continuation) {
		// The continuation could set another continuation, so move this 
		// one out of the way first before we call it.
		AsyncContinuation continuation;
		async_continuation.swap(continuation);

		continuation();
	}

	// If we just finished an async operation that was
	// started within the Update() routine, player will
	// tell us to defer incrementing the frame counter
	// until now.
	if (was_async_from_main_loop && !IsAsyncPending()) {
		Player::IncFrame();
	}
}

bool Scene::IsAsyncPending() {
	return Transition::instance().IsActive() || AsyncHandler::IsImportantFilePending()
		|| (instance != nullptr && instance->HasDelayFrames());
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

	DEBUG_VALIDATE("Push");
}

void Scene::Pop() {
	old_instances.push_back(instances.back());
	instances.pop_back();

	if (instances.size() == 0) {
		Push(std::make_shared<Scene>()); // Null-scene
	}

	instance = instances.back();

	push_pop_operation = ScenePopped;

	DEBUG_VALIDATE("Pop");
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
			DEBUG_VALIDATE("PopUntil");
			return;
		}
		++count;
	}

	Output::Warning("The requested scene {} was not on the stack", scene_names[type]);
	DEBUG_VALIDATE("PopUntil");
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

void Scene::DrawBackground(Bitmap& dst) {
	dst.Fill(Game_System::GetBackgroundColor());
}

bool Scene::CheckSceneExit(AsyncOp aop) {
	if (aop.GetType() == AsyncOp::eExitGame) {
		if (Scene::Find(Scene::GameBrowser)) {
			Scene::PopUntil(Scene::GameBrowser);
		} else {
			Player::exit_flag = true;
		}
		return true;
	}

	if (aop.GetType() == AsyncOp::eToTitle) {
		Scene::ReturnToTitleScene();
		return true;
	}

	return false;
}



inline void Scene::DebugValidate(const char* caller) {
	if (instances.size() <= 1) {
		// Scene of size 1 happens before graphics stack is up. Which can
		// cause the following logs to crash.
		return;
	}
	std::bitset<SceneMax> present;
	for (auto& scene: instances) {
		if (present[scene->type]) {
			Output::Debug("Scene Stack after {}:", caller);
			for (auto& s: instances) {
				auto fmt =  (s == scene) ? "--> {} <--" : "  {}";
				Output::Debug(fmt, scene_names[s->type]);
			}
			Output::Error("Multiple scenes of type={} in the Scene instances stack!", scene_names[scene->type]);
		}
		present[scene->type] = true;
	}
	if (instances[0]->type != Null) {
		Output::Error("Scene.instances[0] is of type={} in the Scene instances stack!", scene_names[instances[0]->type]);
	}
}

bool Scene::ReturnToTitleScene() {
	if (Scene::instance && Scene::instance->type == Scene::Title) {
		return false;
	}

	auto title_scene = Scene::Find(Scene::Title);
	if (!title_scene) {
		return false;
	}

	title_scene->SetDelayFrames(Scene::kReturnTitleDelayFrames);
	Scene::PopUntil(Scene::Title);
	return true;
}

void Scene::TransferDrawablesFrom(Scene& prev_scene) {
	drawable_list.TakeFrom(prev_scene.GetDrawableList(),
			[this](auto* draw) { return draw->IsGlobal() || (uses_shared_drawables && draw->IsShared()); });
}

