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

#include "resampler.h"
#include "output.h"
#include <algorithm>
#include <cassert>

namespace Audio {

static const int32 ST_SAMPLE_MAX = 0x7fff;
static const int32 ST_SAMPLE_MIN = (-ST_SAMPLE_MAX - 1);

inline void clip_add(int16& a, int b) {
	int32 val = a + b;
	a = (int16) std::max(ST_SAMPLE_MIN, std::min(ST_SAMPLE_MAX, val));
}

class SimpleResampler : public Resampler {
public:
	SimpleResampler(uint16 _inrate, uint16 _outrate) : inrate(_inrate), outrate(_outrate) {

	}
	void merge(int16* data, int d_len, AudioStream* stream, uint16 vol, uint8 balance);

private:
	uint16 inrate;
	uint16 outrate;
};

class CopyResampler : public Resampler {
public:
	CopyResampler() {

	}
	void merge(int16* data, int d_len, AudioStream* stream, uint16 vol, uint8 balance);
};

void SimpleResampler::merge(int16* data, int d_len, AudioStream* stream, uint16 vol, uint8 balance) {

}

void CopyResampler::merge(int16* data, int d_len, AudioStream* stream, uint16 vol, uint8 balance) {

}

Resampler* Resampler::GetInstance(uint16 inrate, uint16 outrate) {
	if (inrate == outrate) {
		return new CopyResampler();
	} else {
		if ((inrate % outrate) == 0) {
			return new SimpleResampler(inrate, outrate);
		} else {
			// TODO
			//return new LinearResampler(inrate, outrate);
			return 0;
		}
	}
}

}
