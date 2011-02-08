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

#ifndef _EASYRPG_RESAMPLER_H_
#define _EASYRPG_RESAMPLER_H_

#include "system.h"

class Resampler {
public:
	virtual void merge(int16* data, int d_len, int16* stream, int s_len, uint16 vol, uint8 balance) = 0;

	static Resampler* GetInstance(uint16 inrate, uint16 outrate);
};

#endif
