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

#include "mixer.h"
#include "output.h"
#include <cassert>
namespace Audio {

void Mixer::AddChannel(Channel* channel) {
	assert(channel);

	if (channels.size() >= MAX_CHANNELS) {
		Output::Error("Reached channel limit.");
		// delete channel;
		return;
	}

	channels.push_back(channel);
}

void Mixer::PauseAll() {
	tChannelListIt it;
	for (it = channels.begin(); it != channels.end(); ++it)
		(*it)->Pause();
}

void Mixer::DestroyAll() {
	tChannelListIt it;

	for (it = channels.begin(); it != channels.end(); ++it)
		delete *it;

	channels.clear();
}

void Mixer::Destroy(Channel* channel) {
	tChannelListIt it;
	for (it = channels.begin(); it != channels.end(); ++it)
		if (*it == channel) {
			delete *it;
			channels.erase(it);
			break;
		}
}

void Mixer::callback(void* unused, uint8* stream, int len) {
	int16* buf = (int16*) stream;

	len /= 2;

	tChannelListIt it;
	for (it = channels.begin(); it != channels.end(); ++it)
		(*it)->Mix(buf, len);
}

}