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

#ifndef _SCENE_H_
#define _SCENE_H_

////////////////////////////////////////////////////////////
/// Includes
////////////////////////////////////////////////////////////
#include <vector>

////////////////////////////////////////////////////////////
/// Scene virtual class
////////////////////////////////////////////////////////////
class Scene {
public:
	/// Scene types
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
		SceneMax
	};
	
	////////////////////////////////////////////////////////
	/// Constructor.
	////////////////////////////////////////////////////////
	Scene();

	////////////////////////////////////////////////////////
	/// Destructor.
	////////////////////////////////////////////////////////
	virtual ~Scene() {};

	////////////////////////////////////////////////////////
	/// Scene entry point.
	/// The Scene Main-Function manages a stack and always
	/// executes the scene that is currently on the top of
	/// the stack.
	////////////////////////////////////////////////////////
	virtual void MainFunction();

	////////////////////////////////////////////////////////
	/// Start processing.
	/// This function is executed while the screen is faded
	/// out. All objects needed for the scene should be
	/// created here.
	////////////////////////////////////////////////////////
	virtual void Start();

	////////////////////////////////////////////////////////
	/// Post-Start processing.
	/// This function is executed after the fade in.
	////////////////////////////////////////////////////////
	virtual void PostStart();

	////////////////////////////////////////////////////////
	/// Pre-Terminate processing.
	/// This function is executed before the fade out for
	/// the scene change.
	////////////////////////////////////////////////////////
	virtual void PreTerminate();

	////////////////////////////////////////////////////////
	/// Terminate processing.
	/// This function is executed after the fade out for
	/// the scene change. All delete operations should be
	/// done in this func and not in the destructor.
	////////////////////////////////////////////////////////
	virtual void Terminate();

	////////////////////////////////////////////////////////
	/// Does the transition during scene-switching.
	////////////////////////////////////////////////////////
	virtual void PerformTransition();

	////////////////////////////////////////////////////////
	/// Called every frame.
	/// The scene should redraw all elements.
	////////////////////////////////////////////////////////
	virtual void Update();

	////////////////////////////////////////////////////////
	/// Pushes a new scene on the scene execution stack.
	/// @param new_scene New scene
	/// @param pop_stack_top : If the scene that is currently
	/// on the top should be popped
	////////////////////////////////////////////////////////
	static void Push(Scene* new_scene, bool pop_stack_top = false);

	////////////////////////////////////////////////////////
	/// Removes the scene that is on the top of the stack
	////////////////////////////////////////////////////////
	static void Pop();

	////////////////////////////////////////////////////////
	/// Removes scenes from the stack, until a specific one
	/// is reached.
	/// @param type : Type of the scene that is searched
	////////////////////////////////////////////////////////
	static void PopUntil(SceneType type);

	////////////////////////////////////////////////////////
	/// Finds the topmost scene of a specific type on the stack
	/// @param type : Type of the scene that is searched
	/// @return : The scene found, or NULL if no such scene exists
	////////////////////////////////////////////////////////
	static Scene* Find(SceneType type);

	// Don't write to the following values directly when you want to change
	// the scene! Use Push and Pop instead!

	/// Scene type.
	SceneType type;

	/// Current scene.
	static Scene* instance;

	/// Old scenes, temporary save for deleting.
	static std::vector<Scene*> old_instances;

	/// Contains name of the Scenes. For Debug purposes.
	static const char scene_names[SceneMax][12];

private:
	/// Scene stack
	static std::vector<Scene*> instances;
};

#endif
