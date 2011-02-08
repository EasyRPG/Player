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

#include "channel.h"
#include "resampler.h"

namespace Audio {

Channel::Channel(uint16 _rate, uint16 _vol, uint8 _balance, pBuf& data, int _bsize):
rate(_rate),
vol(_vol),
balance(_balance),
buffer(data),
bsize(_bsize),
paused(true),
offset(0) {

	resampler = Resampler::GetInstance(rate, 22050); 

}

Channel::~Channel() {
	delete resampler;
}


void Channel::Mix(int16 *stream, int len) {
	uint32 pos = offset*2; // * 2 = stereo
	resampler->merge(buffer+pos, bsize-pos, stream, len, vol, balance);
}

}
