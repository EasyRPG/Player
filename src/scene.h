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
		Logo
	};
	
	////////////////////////////////////////////////////////
	/// Destructor.
	////////////////////////////////////////////////////////
	virtual ~Scene() {};

	////////////////////////////////////////////////////////
	/// Scene entry point.
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
	/// done in the destructor and not in this func.
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

	/// Current scene.
	static Scene* instance;

	/// Current scene type.
	static SceneType type;

	/// Old scene, temporary save for deleting.
	static Scene* old_instance;
};

#endif
