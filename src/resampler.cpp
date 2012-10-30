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

static const int32_t ST_SAMPLE_MAX = 0x7fff;
static const int32_t ST_SAMPLE_MIN = (-ST_SAMPLE_MAX - 1);

inline void clip_add(int16_t& a, int b) {
	int32_t val = a + b;
	a = (int16_t) std::max(ST_SAMPLE_MIN, std::min(ST_SAMPLE_MAX, val));
}

class SimpleResampler : public Resampler {
public:
	SimpleResampler(uint16_t _inrate, uint16_t _outrate) : inrate(_inrate), outrate(_outrate) {

	}
	void merge(int16_t* data, int d_len, AudioStream* stream, uint16_t vol, uint8_t balance);

private:
	uint16_t inrate;
	uint16_t outrate;
};

class CopyResampler : public Resampler {
public:
	CopyResampler() {

	}
	void merge(int16_t* data, int d_len, AudioStream* stream, uint16_t vol, uint8_t balance);
};

void SimpleResampler::merge(int16_t* /* data */, int /* d_len */, AudioStream* /* stream */, uint16_t /* vol */, uint8_t /* balance */) {

}

void CopyResampler::merge(int16_t* /* data */, int /* d_len */, AudioStream* /* stream */, uint16_t /* vol */, uint8_t /* balance */) {

}

Resampler* Resampler::GetInstance(uint16_t inrate, uint16_t outrate) {
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
