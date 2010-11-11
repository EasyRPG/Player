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
#include "game_actor.h"
#include "main_data.h"
#include "cache.h"

////////////////////////////////////////////////////////////
/// Constructor
////////////////////////////////////////////////////////////
Game_Actor::Game_Actor(int actorId)
{
	Setup(actorId);
}

////////////////////////////////////////////////////////////
/// Destructor
////////////////////////////////////////////////////////////
Game_Actor::~Game_Actor()
{
	delete face;
}

////////////////////////////////////////////////////////////
/// Setup
////////////////////////////////////////////////////////////
void Game_Actor::Setup(int actorId)
{
	// Stub

	// ActorId starts with 1 but arrayindex with 0
	name = Main_Data::data_actors[actorId-1].name;

	// Get the Faceset Graphic
	Bitmap* faceset = Cache::Faceset(Main_Data::data_actors[actorId-1].face_name);
	int face_index = Main_Data::data_actors[actorId-1].face_index;
	face = new Bitmap(48, 48);
	face->Blit(0, 0, faceset, Rect((face_index % 4) * 48, face_index/4 * 48, 48, 48), 255);
}
