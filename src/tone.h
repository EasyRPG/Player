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

#ifndef _TONE_H_
#define _TONE_H_

////////////////////////////////////////////////////////////
/// Tone class
////////////////////////////////////////////////////////////
class Tone {
public:
	Tone();
	Tone(int ired, int igreen, int iblue, int igray);
	~Tone();

	bool operator!=(const Tone& other) const;

	void Set(int nred, int ngreen, int nblue, int ngray);

	int red;
	int green;
	int blue;
	int gray;
};

#endif
