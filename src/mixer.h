#include <list>
#include "channel.h"

namespace Audio {

class Mixer {
public:

	static const int MAX_CHANNELS = 8;

	typedef std::list<Channel*> tChannelList;
	typedef tChannelList::iterator tChannelListIt;

	void AddChannel(Channel* channel);

	void PauseAll();
	void DestroyAll();
	void Destroy(Channel* channel);

	void callback(void* unused, uint8* stream, int len);

private:

	tChannelList channels;

};


}
