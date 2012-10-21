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

Channel::Channel(uint16_t _rate, uint16_t _vol, uint8_t _balance, AudioStream* _stream):
rate(_rate),
vol(_vol),
balance(_balance),
paused(true),
stream(_stream) {

	resampler = Resampler::GetInstance(rate, 22050); 

}

Channel::~Channel() {
	delete resampler;
}


void Channel::Mix(int16_t *data, int len) {
	resampler->merge(data, len, stream, vol, balance);
}

}
