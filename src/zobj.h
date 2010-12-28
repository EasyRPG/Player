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

#ifndef _ZOBJ_H_
#define _ZOBJ_H_

///////////////////////////////////////////////////////////
/// Headers
///////////////////////////////////////////////////////////
#include "system.h"

///////////////////////////////////////////////////////////
/// ZObj class
///////////////////////////////////////////////////////////
class ZObj {
public:
	ZObj(int z, uint32 creation, uint32 id);

	void SetZ(int z);
	int GetZ() const ;
	uint32 GetCreation() const;
	uint32 GetId() const;

private:
	int z;
	uint32 creation;
	uint32 ID;
};

#endif
