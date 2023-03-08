/*
 * This file is part of EasyRPG Player.
 *
 * EasyRPG Player is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * EasyRPG Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef EP_DECODER_OPUS_H
#define EP_DECODER_OPUS_H

#ifdef HAVE_OPUS
#include <opus/opusfile.h>
#endif
#include "audio_decoder.h"

/**
 * Audio decoder for Opus powered by opusfile
 *
 * Class renamed from OpusDecoder because of a conflict with the library.
 */
class OpusAudioDecoder : public AudioDecoder {
public:
	OpusAudioDecoder();

	~OpusAudioDecoder();

	// Audio Decoder interface
	bool Open(Filesystem_Stream::InputStream stream) override;

	bool Seek(std::streamoff offset, std::ios_base::seekdir origin) override;

	bool IsFinished() const override;

	void GetFormat(int& frequency, AudioDecoder::Format& format, int& channels) const override;

	bool SetFormat(int frequency, AudioDecoder::Format format, int channels) override;

	int GetTicks() const override;
private:
	int FillBuffer(uint8_t* buffer, int length) override;

#ifdef HAVE_OPUS
	OggOpusFile* oof = nullptr;
#endif
	Filesystem_Stream::InputStream stream;
	bool finished = false;
	int frequency = 48000;
	int channels = 2;

	struct {
		bool looping = false;
		// When true start & end are the same, keeps the stream alive to match Midi loop behaviour
		bool to_end = false;
		int64_t start = -1;
		int64_t end = -1;
	} loop;
};

#endif
