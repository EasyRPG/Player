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

#ifndef EASYRPG_AUDIO_DECODER_OGGVORBIS_H
#define EASYRPG_AUDIO_DECODER_OGGVORBIS_H

// Headers
#include <string>
#include <memory>
#ifdef HAVE_TREMOR
#include <tremor/ivorbiscodec.h>
#include <tremor/ivorbisfile.h>
#elif HAVE_OGGVORBIS
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>
#endif
#include "audio_decoder.h"

/**
 * Audio decoder for Ogg Vorbis powered by libTremor/libOgg+libVorbis
 */
class OggVorbisDecoder : public AudioDecoder {
public:
	OggVorbisDecoder();

	~OggVorbisDecoder();

	// Audio Decoder interface
	bool Open(FILE* file) override;

	bool Seek(size_t offset, Origin origin) override;

	bool IsFinished() const override;

	void GetFormat(int& frequency, AudioDecoder::Format& format, int& channels) const override;

	bool SetFormat(int frequency, AudioDecoder::Format format, int channels) override;
private:
	int FillBuffer(uint8_t* buffer, int length) override;

#if defined(HAVE_TREMOR) || defined(HAVE_OGGVORBIS)
	OggVorbis_File *ovf = NULL;
#endif
	bool finished = false;
	int frequency = 44100;
	int channels = 2;
};

#endif
