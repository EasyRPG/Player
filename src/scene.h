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

#ifndef EP_SCENE_H
#define EP_SCENE_H

// Headers
#include "system.h"
#include "async_op.h"
#include "drawable_list.h"
#include "span.h"
#include "window_selectable.h"
#include <vector>
#include <functional>
#include <lcf/rpg/savesystem.h>

class Game_Battler;
class Game_Actor;
class Window_Selectable;

/**
 * Scene virtual class.
 */
class Scene {
public:
	/** Scene types. */
	enum SceneType {
		Null,
		Title,
		Map,
		Menu,
		Item,
		Skill,
		Equip,
		ActorTarget,
		Status,
		File,
		Save,
		Load,
		End,
		Battle,
		Shop,
		Name,
		Gameover,
		Debug,
		Logo,
		Order,
		GameBrowser,
		Teleport,
		Settings,
		SceneMax
	};

	static lcf::rpg::SaveSystem::Scene rpgRtSceneFromSceneType(SceneType);

	static constexpr int kStartGameDelayFrames = 60;
	static constexpr int kReturnTitleDelayFrames = 20;

	/**
	 * Constructor.
	 */
	Scene();

	/**
	 * Destructor.
	 */
	virtual ~Scene() {};

	/**
	 * Scene entry point.
	 * The Scene Main-Function manages a stack and always
	 * executes the scene that is currently on the top of
	 * the stack.
	 */
	void MainFunction();

	/**
	 * Start processing.
	 * This function is executed while the screen is faded
	 * out. All objects needed for the scene should be
	 * created here.
	 */
	virtual void Start();

	/**
	 * Continue processing.
	 * This function is executed when returning from a
	 * nested scene (instead of Start).
	 *
	 * @param prev_scene The previous scene
	 */
	virtual void Continue(SceneType prev_scene);

	/**
	 * Used to configure the transition upon starting or resuming
	 * the scene.
	 *
	 * @param prev_scene the scene we transitioned from
	 */
	virtual void TransitionIn(SceneType prev_scene);

	/**
	 * Used to configure the transition upon ending or suspending
	 * the scene.
	 *
	 * @param next_scene the scene we will transition to
	 */
	virtual void TransitionOut(SceneType next_scene);

	/**
	 * This function is executed after the fade out transition is finished.
	 * Use this for clean up before terminating the scene or switching to
	 * a nested scene.
	 * In case of termination this is called right before the destructor.
	 *
	 * @param next_scene the scene we will transition to
	 */
	virtual void Suspend(SceneType next_scene);

	/**
	 * Called when a transition or async load is finished.
	 */
	void OnFinishAsync();

	/** @returns true if an async operation that would block the main loop is pending */
	static bool IsAsyncPending();

	/**
	 * Called when data was modified from the outside and must be reloaded.
	 */
	virtual void Refresh() {}

	/**
	 * Called every frame.
	 */
	void Update();

	/**
	 * Update graphics in scene stack
	 */
	virtual void UpdateGraphics() {}

	/**
	 * Pushes a new scene on the scene execution stack.
	 *
	 * @param new_scene new scene.
	 * @param pop_stack_top if the scene that is currently
	 *	                  on the top should be popped.
	 */
	static void Push(std::shared_ptr<Scene> const& new_scene, bool pop_stack_top = false);

	/**
	 * Removes the scene that is on the top of the stack.
	 */
	static void Pop();

	/**
	 * Removes scenes from the stack, until a specific one
	 * is reached.
	 *
	 * @param type type of the scene that is searched.
	 */
	static void PopUntil(SceneType type);

	/**
	 * Finds the topmost scene of a specific type on the stack.
	 *
	 * @param type type of the scene that is searched.
	 * @return the scene found, or NULL if no such scene exists.
	 */
	static std::shared_ptr<Scene> Find(SceneType type);

	// Don't write to the following values directly when you want to change
	// the scene! Use Push and Pop instead!

	/** Scene type. */
	SceneType type;

	/** Current scene. */
	static std::shared_ptr<Scene> instance;

	/** Old scenes, temporary save for deleting. */
	static std::vector<std::shared_ptr<Scene> > old_instances;

	/** Contains name of the Scenes. For debug purposes. */
	static const char scene_names[SceneMax][12];

	/**
	 * Called by the graphic system to request drawing of a background, usually a system color background
	 *
	 * @param dst The bitmap to draw the background to.
	 */
	virtual void DrawBackground(Bitmap& dst);

	DrawableList& GetDrawableList();

	/** @return true if the Scene has been initialized */
	bool IsInitialized() const;

	/**
	 * @return the scene requested by events
	 * @post the internally stored requested scene is cleared
	 */
	std::shared_ptr<Scene> TakeRequestedScene();

	/** @return the type of the requested scene, or Null if there is none */
	SceneType GetRequestedSceneType() const;

	/** @return true if a scene is being requested */
	bool HasRequestedScene() const;

	/**
	 * Sets the requested scene to change to
	 *
	 * @param scene the scene to call
	 */
	void SetRequestedScene(std::shared_ptr<Scene> scene);

	/**
	 * Check if the async operation wants to end the scene.
	 *
	 * @param aop The pending async operation
	 */
	static bool CheckSceneExit(AsyncOp aop);

	/**
	 * Set number of frames to wait before start/continue the scene
	 *
	 * @frames number of frames to wait
	 */
	void SetDelayFrames(int frames);

	/** @return true if there are >0 number of frames to wait before scene start/continue */
	bool HasDelayFrames() const;

	/** Decrement delay frames by 1 if we're waiting */
	void UpdateDelayFrames();

	/**
	 * Pops the stack until the title screen and sets proper delay.
	 *
	 * @return false if there is no title scene in the stack, or we're already on the title scene
	 */
	static bool ReturnToTitleScene();

	/**
	 * Transfer drawables from the previous scene. This is called
	 * when we do a scene change.
	 *
	 * @param prev_scene the scene to take from.
	 */
	void TransferDrawablesFrom(Scene& prev_scene);

	/** @return true if this scene uses shared drawables */
	bool UsesSharedDrawables() const;

	virtual void OnPartyChanged(Game_Actor* actor, bool add);
	virtual void OnEventHpChanged(Game_Battler* battler, int hp);
	virtual void OnTranslationChanged();

protected:
	using AsyncContinuation = std::function<void(void)>;
	AsyncContinuation async_continuation;

	/**
	 * Called every frame from Update.
	 * The scene should redraw all elements.
	 */
	virtual void vUpdate() {};

	/**
	 * Set whether or not this scene will use shared drawables.
	 * If true, when started the scene will transfer all shared
	 * drawables from the previous scene.
	 *
	 * @param value whether to use shared drawables.
	 */
	void SetUseSharedDrawables(bool value);

	/** @return A list of window selectables suitable for mouse cursor selection */
	virtual Span<Window_Selectable*> GetWindowSelectables() { return {}; };

	/**
	 * If no async operation is pending, call f() now. Otherwise
	 * defer f until async operations are done.
	 */
	template <typename F> void AsyncNext(F&& f);

private:
	void ScheduleTransitionIn(Scene::SceneType prev_scene_type);

	/** Scene stack. */
	static std::vector<std::shared_ptr<Scene> > instances;

	static int push_pop_operation;

	DrawableList drawable_list;
	/**
	 * true if Start() was called. For handling the special case that two
	 * or more scenes are pushed. In that case only the last calls start, the
	 * other Continue(). This enforces calling Start().
	 */
	bool initialized = false;
	bool uses_shared_drawables = false;

	static void DebugValidate(const char* caller);

	std::shared_ptr<Scene> request_scene;
	int delay_frames = 0;
};

inline bool Scene::IsInitialized() const {
	return initialized;
}

inline std::shared_ptr<Scene> Scene::TakeRequestedScene() {
	auto ptr = std::move(request_scene);
	request_scene.reset();
	return ptr;
}

inline Scene::SceneType Scene::GetRequestedSceneType() const {
	return request_scene ? request_scene->type : Null;
}

inline bool Scene::HasRequestedScene() const {
	return request_scene != nullptr;
}

inline void Scene::SetRequestedScene(std::shared_ptr<Scene> scene) {
	request_scene = std::move(scene);
}

inline void Scene::SetDelayFrames(int frames) {
	delay_frames = frames;
}

inline bool Scene::HasDelayFrames() const {
	return delay_frames > 0;
}

inline void Scene::UpdateDelayFrames() {
	if (HasDelayFrames()) {
		--delay_frames;
	}
}

inline DrawableList& Scene::GetDrawableList() {
	return drawable_list;
}

inline void Scene::SetUseSharedDrawables(bool value) {
	uses_shared_drawables = value;
}

template <typename F>
inline void Scene::AsyncNext(F&& f) {
	if (IsAsyncPending()) {
		async_continuation = std::forward<F>(f);
	} else {
		f();
	}
}

inline bool Scene::UsesSharedDrawables() const {
	return uses_shared_drawables;
}

#endif
