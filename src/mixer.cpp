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