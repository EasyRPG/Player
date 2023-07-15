#include "packet.h"
#include <bitset>
#include <cassert>

using namespace Multiplayer;

/**
 * Encode
 */

std::string Packet::ToBytes() const {
	return Build();
}

// this function will perform a parse of data with DELIMs
// delimiters will be removed, and returns the copied data
// To The Previous Participant: please please add more helpful comments!!
constexpr std::string_view keywords[] = {
	Packet::PARAM_DELIM,
	Packet::MSG_DELIM,
};
constexpr size_t k_size = sizeof(keywords) / sizeof(std::string_view);
std::string Packet::Sanitize(std::string_view param) {
	std::string r;
	r.reserve(param.size());
	std::bitset<k_size> searching_marks;
	size_t candidate_index{};
	for (size_t i = 0; i != param.size(); ++i) {
		// append data until a potential DELIMs
		if (candidate_index == 0) {
			bool found = false;
			for (size_t j = 0; j != k_size; ++j) {
				assert(!keywords[j].empty());
				// found a delimiter character but not sure it is
				if (keywords[j][0] == param[i]) {
					searching_marks.set(j);
					found = true;
				}
			}

			if (found)
				candidate_index = 1;
			else
				r += param[i];
		// further confirmation to find delimiters
		} else {
			bool found = false;
			bool match = false;
			for (size_t j = 0; j != k_size; ++j) {
				if (searching_marks.test(j)) {
					// found sequential delims but it's not complete yet
					if (keywords[j][candidate_index] == param[i]) {
						found = true;
						// the length not only 3 (such an \uFFFE), it can be more
						// equal length means a complete match
						if (keywords[j].size() == candidate_index + 1) {
							match = true;
							break;
						}
					} else {
						// I'm sure it's not, do characters append
						searching_marks.reset(j);
					}
				}
			}

			// the i-char data is sub delimiter. skip the append
			if (match) {
				candidate_index = 0;
			// incomplete match, keep trying
			} else if (found) {
				++candidate_index;
			} else {
				// append previous pretended DELIMs
				r.append(param.substr(i - candidate_index, candidate_index + 1));
				candidate_index = 0;
			}
		}
	}
	// remaining data are pretended DELIMs. append
	if (candidate_index != 0)
		r.append(param.substr(param.length() - candidate_index));
	return r;
}

/**
 * Decode
 */

template<>
int Packet::Decode(std::string_view s) {
	int r;
	auto e = std::from_chars(s.data(), s.data() + s.size(), r);
	if (e.ec != std::errc())
		throw std::runtime_error("Multiplayer::Packet::Decode<int> failed");
	return r;
}

template<>
bool Packet::Decode(std::string_view s) {
	if (s == "1")
		return true;
	if (s == "0")
		return true;
	throw std::runtime_error("Multiplayer::Packet::Decode<bool> failed");
}
