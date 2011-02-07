#include "channel.h"
#include "resampler.h"

namespace Audio {

Channel::Channel(uint16 _rate, uint16 _vol, uint8 _balance, pBuf& data):
rate(_rate),
vol(_vol),
balance(_balance),
buffer(data),
paused(true) {

	// resampler = GetResamplerInstance(rate, global_rate); 

}

Channel::~Channel() {
}


void Channel::Mix(int16 *stream, int len) {
	// resampler->proc(stream, len, vol, balance);

}


}
