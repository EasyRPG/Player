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

#ifndef EASYRPG_AUDIO_DECODER_XMP_H
#define EASYRPG_AUDIO_DECODER_XMP_H

// Headers
#include <string>
#include <memory>
#ifdef HAVE_XMP
#include "xmp.h"
#endif
#include "audio_decoder.h"

/**
 * Audio decoder for Modules (xm, s3m, it & mod) powered by libXMP-lite
 */
class XMPDecoder : public AudioDecoder {
public:
	XMPDecoder();

	~XMPDecoder();

	// Audio Decoder interface
	bool Open(FILE* file) override;

	bool Seek(size_t offset, Origin origin) override;

	bool IsFinished() const override;

	void GetFormat(int& frequency, AudioDecoder::Format& format, int& channels) const override;

	bool SetFormat(int frequency, AudioDecoder::Format format, int channels) override;

	static bool IsModule(std::string filename);
private:
	int FillBuffer(uint8_t* buffer, int length) override;

#ifdef HAVE_XMP
	xmp_context ctx = nullptr;
#endif
	bool finished = false;

	// defaults
	AudioDecoder::Format format = Format::S16;
	int frequency = 44100;
	int channels = 2;
};

#endif
