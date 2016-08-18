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

// Headers
#include <cassert>
#include <cstring>
#include "audio_decoder.h"
#include "filefinder.h"
#include "output.h"
#include "system.h"
#include "utils.h"

#include "decoder_fmmidi.h"
#include "decoder_mpg123.h"
#include "decoder_oggvorbis.h"
#include "decoder_wildmidi.h"
#include "decoder_libsndfile.h"
#include "decoder_wav.h"
#include "audio_resampler.h"

void AudioDecoder::Pause() {
	paused = true;
}

void AudioDecoder::Resume() {
	paused = false;
}

int AudioDecoder::Decode(uint8_t* buffer, int length) {
	return Decode(buffer, length, 0);
}

int AudioDecoder::Decode(uint8_t* buffer, int length, int recursion_depth) {
	if (paused) {
		memset(buffer, '\0', length);
		return length;
	}

	int res = FillBuffer(buffer, length);

	if (res < 0) {
		memset(buffer, '\0', length);
	} else if (res < length) {
		memset(&buffer[res], '\0', length - res);
	}

	if (IsFinished() && looping && recursion_depth < 10) {
		++loop_count;
		Rewind();
		if (length - res > 0) {
			int res2 = Decode(&buffer[res], length - res, ++recursion_depth);
			if (res2 <= 0) {
				return res;
			}
			return res + res2;
		}
	}

	if (recursion_depth == 10 && loop_count < 50) {
		// Only report this a few times in the hope that this is only a temporary problem and to prevent log spamming
		Output::Debug("Audio Decoder: Recursion depth exceeded. Probably stream error.");
	}

	return res;
}

int AudioDecoder::DecodeAsMono(uint8_t* left, uint8_t* right, int size) {
	int freq; Format format; int channels;
	GetFormat(freq, format, channels);

	if (channels == 1) {
		return Decode(left, size);
	}

	if ((int)mono_buffer.size() < size * 2) {
		mono_buffer.resize(size * 2);
	}
	
	int read = Decode(mono_buffer.data(), size * 2);
	if (read < 0) {
		memset(left, '\0', size);
		memset(right, '\0', size);
		return -1;
	}

	int sample_size = GetSamplesizeForFormat(format);

	for (int i = 0; i <= read / 2; i += sample_size) {
		memcpy(&left[i], &mono_buffer.data()[i * channels], sample_size);
		memcpy(&right[i], &mono_buffer.data()[i * channels + sample_size], sample_size);
	}

	if (read < size / 2) {
		memset(&left[read / 2], '\0', size);
		memset(&right[read / 2], '\0', size);
	}

	return read / 2;
}

class WMAUnsupportedFormatDecoder : public AudioDecoder {
public:
	WMAUnsupportedFormatDecoder() {
		error_message = std::string("WMA audio files are not supported. Reinstall the\n") +
			"game and don't convert them when asked by Windows!\n";
	}
	bool Open(FILE*) override { return false; }
	bool IsFinished() const override { return true; }
	void GetFormat(int&, Format&, int&) const override {}
private:
	int FillBuffer(uint8_t*, int) override { return -1; };
};
const char wma_magic[] = { (char)0x30, (char)0x26, (char)0xB2, (char)0x75 };

std::unique_ptr<AudioDecoder> AudioDecoder::Create(FILE* file, const std::string& filename) {
	char magic[4] = { 0 };
	fread(magic, 4, 1, file);
	fseek(file, 0, SEEK_SET);

	// Try to use MIDI decoder, use fallback(s) if available
	if (!strncmp(magic, "MThd", 4)) {
#ifndef HAVE_WILDMIDI
		/* WildMidi is currently the only Audio_Decoder that needs the filename passed
		 * directly, this avoids a warning about the possibly unused variable
		 */
		(void)filename;
#else
		static bool wildmidi_works = true;
		if (wildmidi_works) {
			AudioDecoder *mididec = nullptr;
#  ifdef USE_AUDIO_RESAMPLER
			mididec = new AudioResampler(new WildMidiDecoder(filename));
#  else
			mididec = new WildMidiDecoder(filename);
#  endif
			if (mididec) {
				if (mididec->WasInited())
					return std::unique_ptr<AudioDecoder>(mididec);

				delete mididec;
			}
			wildmidi_works = false;
		}
#endif
#if WANT_FMMIDI == 1
#  ifdef USE_AUDIO_RESAMPLER
		return std::unique_ptr<AudioDecoder>(new AudioResampler(new FmMidiDecoder(), true, AudioResampler::Quality::Low));
#  else
		return std::unique_ptr<AudioDecoder>(new FmMidiDecoder());
#  endif
#endif
		// No MIDI decoder available
		return nullptr;
	}

	// Try to use internal OGG decoder
	if (!strncmp(magic, "OggS", 4)) { // OGG
#if defined(HAVE_TREMOR) || defined(HAVE_OGGVORBIS)
#  ifdef USE_AUDIO_RESAMPLER
		return std::unique_ptr<AudioDecoder>(new AudioResampler(new OggVorbisDecoder()));
#  else
		return std::unique_ptr<AudioDecoder>(new OggVorbisDecoder());
#  endif
#endif
	}
	
#ifdef WANT_FASTWAV
	// Try to use a basic decoder for faster wav decoding if not ADPCM
	if (!strncmp(magic, "RIFF", 4)) {
		fseek(file, 20, SEEK_SET);
		uint16_t raw_enc;
		fread(&raw_enc, 2, 1, file);
		Utils::SwapByteOrder(raw_enc);
		fseek(file, 0, SEEK_SET);
		if (raw_enc == 0x01) { // Codec is normal PCM
#  ifdef USE_AUDIO_RESAMPLER
			return std::unique_ptr<AudioDecoder>(new AudioResampler(new WavDecoder()));
#  else
			return std::unique_ptr<AudioDecoder>(new WavDecoder());
#  endif
		}
	}

#endif
	
	// Try to use libsndfile for common formats
	if (!strncmp(magic, "RIFF", 4) || // WAV
		!strncmp(magic, "FORM", 4) || // WAV AIFF
		!strncmp(magic, "OggS", 4) || // OGG
		!strncmp(magic, "fLaC", 4)) { // FLAC
#ifdef HAVE_LIBSNDFILE
#  ifdef USE_AUDIO_RESAMPLER
			return std::unique_ptr<AudioDecoder>(new AudioResampler(new LibsndfileDecoder()));
#  else
			return std::unique_ptr<AudioDecoder>(new LibsndfileDecoder());
#  endif
#endif
		return nullptr;
	}

	// Inform about WMA issue
	if (!memcmp(magic, wma_magic, 4)) {
		return std::unique_ptr<AudioDecoder>(new WMAUnsupportedFormatDecoder());
	}

	// False positive MP3s should be prevented before by checking for common headers
#ifdef HAVE_MPG123
	static bool mpg123_works = true;
	if (mpg123_works) {
		AudioDecoder *mp3dec = nullptr;
		if (strncmp(magic, "ID3", 3) == 0) {
#  ifdef USE_AUDIO_RESAMPLER
			mp3dec = new AudioResampler(new Mpg123Decoder());
#  else
			mp3dec = new Mpg123Decoder();
#  endif
			if (mp3dec) {
				if (mp3dec->WasInited())
					return std::unique_ptr<AudioDecoder>(mp3dec);

				delete mp3dec;
			}
			mpg123_works = false;
			return nullptr;
		}

		// Parsing MP3s seems to be the only reliable way to detect them
		if (Mpg123Decoder::IsMp3(file)) {
			fseek(file, 0, SEEK_SET);
#  ifdef USE_AUDIO_RESAMPLER
			mp3dec = new AudioResampler(new Mpg123Decoder());
#  else
			mp3dec = new Mpg123Decoder();
#  endif
			if (mp3dec) {
				if(mp3dec->WasInited())
					return std::unique_ptr<AudioDecoder>(mp3dec);

				delete mp3dec;
			}
			mpg123_works = false;
			return nullptr;
		}
	}
#endif

	fseek(file, 0, SEEK_SET);
	return nullptr;
}

void AudioDecoder::SetFade(int begin, int end, int duration) {
	fade_time = 0.0;

	if (duration <= 0.0) {
		volume = end;
		return;
	}

	if (begin == end) {
		volume = end;
		return;
	}

	volume = (double)begin;
	fade_end = (double)end;
	fade_time = (double)duration;
	delta_step = (fade_end - volume) / fade_time;
}

void AudioDecoder::Update(int delta) {
	if (fade_time <= 0.0) {
		return;
	}
	
	fade_time -= delta;
	volume += delta * delta_step;

	volume = volume > 100.0 ? 100.0 :
		volume < 0.0 ? 0.0 :
		volume;
}

void AudioDecoder::SetVolume(int volume) {
	this->volume = (double)volume;
}

int AudioDecoder::GetVolume() const {
	return (int)volume;
}

void AudioDecoder::Rewind() {
	if (!Seek(0, Origin::Begin)) {
		// The libs guarantee that Rewind works
		assert(false && "Rewind");
	}
}

bool AudioDecoder::GetLooping() const {
	return looping;
}

void AudioDecoder::SetLooping(bool enable) {
	looping = enable;
}

int AudioDecoder::GetLoopCount() const {
	return loop_count;
}

bool AudioDecoder::WasInited() const {
	return true;
}

std::string AudioDecoder::GetError() const {
	return error_message;
}

std::string AudioDecoder::GetType() const {
	return music_type;
}

bool AudioDecoder::SetFormat(int, Format, int) {
	return false;
}

int AudioDecoder::GetPitch() const {
	return 0;
}

bool AudioDecoder::SetPitch(int) {
	return false;
}

bool AudioDecoder::Seek(size_t, Origin) {
	return false;
}

size_t AudioDecoder::Tell() const {
	return -1;
}

int AudioDecoder::GetTicks() const {
	return 0;
}

int AudioDecoder::GetSamplesizeForFormat(AudioDecoder::Format format) {
	switch (format) {
		case Format::S8:
		case Format::U8:
			return 1;
		case Format::S16:
		case Format::U16:
			return 2;
		case Format::S32:
		case Format::U32:
		case Format::F32:
			return 4;
	}

	assert(false && "Bad format");
	return -1;
}
