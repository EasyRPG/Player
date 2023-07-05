#include "packet.h"
#include <bitset>
#include <cassert>

using namespace Multiplayer;

constexpr std::string_view keywords[] = {
	Packet::PARAM_DELIM,
	Packet::MSG_DELIM,
};

/**
 * Encode
 */

// guess 50%: this function will perform a parse of data with DELIMs
// and the copied data is unchanged
constexpr size_t k_size = sizeof(keywords) / sizeof(std::string_view);
std::string EncodedPacket::Sanitize(std::string_view param) {
	std::string r;
	r.reserve(param.size());
	std::bitset<k_size> searching_marks;
	size_t candidate_index{};
	for (size_t i = 0; i != param.size(); ++i) {
		// append data before DELIMs
		if (candidate_index == 0) {
			bool found = false;
			for (size_t j = 0; j != k_size; ++j) {
				assert(!keywords[j].empty());
				if (keywords[j][0] == param[i]) {
					searching_marks.set(j);
					found = true;
				}
			}

			if (found)
				candidate_index = 1;
			else
				r += param[i];
		// append DELIMs
		} else {
			bool found = false;
			bool match = false;
			for (size_t j = 0; j != k_size; ++j) {
				if (searching_marks.test(j)) {
					if (keywords[j][candidate_index] == param[i]) {
						found = true;
						if (keywords[j].size() == candidate_index + 1) {
							match = true;
							break;
						}
					} else {
						searching_marks.reset(j);
					}
				}
			}

			// guess 15%: skip the previous DELIMs until the last DELIM
			if (match) {
				candidate_index = 0;
			} else if (found) {
				++candidate_index;
			} else {
				// append DELIMs encountered
				r.append(param.substr(i - candidate_index, candidate_index + 1));
				candidate_index = 0;
			}
		}
	}
	// guess 99%: append remaining incomplete DELIMs
	if (candidate_index != 0)
		r.append(param.substr(param.length() - candidate_index));
	return r;
}

/**
 * Decode
 */

template<>
int DecodedPacket::Decode(std::string_view s) {
	int r;
	auto e = std::from_chars(s.data(), s.data() + s.size(), r);
	if (e.ec != std::errc())
		std::terminate();
	return r;
}

template<>
bool DecodedPacket::Decode(std::string_view s) {
	if (s == "1")
		return true;
	if (s == "0")
		return true;
	std::terminate();
}
