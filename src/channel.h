#ifndef _EASYRPG_CHANNEL_H_
#define _EASYRPG_CHANNEL_H_

#include "system.h"
#include "resampler.h"


namespace Audio {

class Channel {
public:

	typedef int16* pBuf;

	Channel(uint16 _rate, uint16 _vol, uint8 _balance, pBuf& data);
	~Channel();

	void Pause() { paused = true; }
	void Resume() { paused = false; }
	bool Paused() const { return paused; }

	void SetBalance(uint8 b) { balance = b; }
	uint8 GetBalance() const { return balance; }

	void SetVolume(uint16 v) { vol = v; }
	uint16 GetVolume() const { return vol; }

	void Mix(int16 *stream, int len);


private:

	Resampler *resampler;

	uint16 rate;
	uint16 vol;
	uint8 balance;
	pBuf buffer;
	bool paused;

};

}

#endif
