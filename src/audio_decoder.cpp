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
#include <cmath>
#include <cstdint>
#include <cstring>
#include "audio_decoder.h"
#include "audio_decoder_base.h"
#include "audio_midi.h"
#include "audio_resampler.h"
#include "output.h"
#include "system.h"
#include "utils.h"

#include "decoder_mpg123.h"
#include "decoder_oggvorbis.h"
#include "decoder_opus.h"
#include "decoder_libsndfile.h"
#include "decoder_drwav.h"
#include "decoder_xmp.h"

using namespace std::chrono_literals;

class WMAUnsupportedFormatDecoder : public AudioDecoder {
public:
	WMAUnsupportedFormatDecoder() {
		error_message = std::string("WMA audio files are not supported. Reinstall the\n") +
			"game and don't convert them when asked by Windows!\n";
	}
	bool Open(Filesystem_Stream::InputStream) override { return false; }
	bool IsFinished() const override { return true; }
	void GetFormat(int&, Format&, int&) const override {}
	bool Seek(std::streamoff, std::ios_base::seekdir) override { return false; }
	int GetTicks() const override { return 0; }
private:
	int FillBuffer(uint8_t*, int) override { return -1; };
};
const char wma_magic[] = { (char)0x30, (char)0x26, (char)0xB2, (char)0x75 };
const std::array<char[2], 4> aac_magics = { {
	{ (char)0xFF, (char)0xF1 },
	{ (char)0xFF, (char)0xF0 },
	{ (char)0xFF, (char)0xF8 },
	{ (char)0xFF, (char)0xF9 }
}};

std::unique_ptr<AudioDecoderBase> AudioDecoder::Create(Filesystem_Stream::InputStream& stream, bool resample) {
	char magic[4] = { 0 };
	if (!stream.ReadIntoObj(magic)) {
		return nullptr;
	}
	stream.seekg(0, std::ios::beg);

	auto add_resampler = [resample](std::unique_ptr<AudioDecoder> dec) -> std::unique_ptr<AudioDecoderBase> {
#ifdef USE_AUDIO_RESAMPLER
		if (resample)
			return std::make_unique<AudioResampler>(std::move(dec));
#endif
		return dec;
	};

	// Try to use MIDI decoder, use fallback(s) if available
	if (!strncmp(magic, "MThd", 4)) {
		auto midi = MidiDecoder::Create(resample);
		if (midi) {
			return midi;
		}
	}

	// Try to use internal OGG decoder
	if (!strncmp(magic, "OggS", 4)) { // OGG
#ifdef HAVE_OPUS
		stream.seekg(28, std::ios::ios_base::beg);
		if (stream.read(magic, sizeof(magic)).gcount() == 0) {
			return nullptr;
		}
		stream.seekg(0, std::ios::ios_base::beg);

		if (!strncmp(magic, "Opus", 4)) {
			return add_resampler(std::make_unique<OpusAudioDecoder>());
		}
#endif

#if defined(HAVE_TREMOR) || defined(HAVE_OGGVORBIS)
		stream.seekg(29, std::ios::ios_base::beg);
		if (stream.read(magic, sizeof(magic)).gcount() == 0) {
			return nullptr;
		}
		stream.seekg(0, std::ios::ios_base::beg);

		if (!strncmp(magic, "vorb", 4)) {
			return add_resampler(std::make_unique<OggVorbisDecoder>());
		}
#endif
	}

#ifdef WANT_DRWAV
	// Use built-in WAV decoder (dr_wav) for faster WAV decoding
	if (!strncmp(magic, "RIFF", 4)) {
		return add_resampler(std::make_unique<DrWavDecoder>());
	}
#endif

	// Try to use libsndfile for common formats
	if (!strncmp(magic, "RIFF", 4) || // WAV
		!strncmp(magic, "FORM", 4) || // WAV AIFF
		!strncmp(magic, "OggS", 4) || // OGG
		!strncmp(magic, "fLaC", 4)) { // FLAC
#ifdef HAVE_LIBSNDFILE
		return add_resampler(std::make_unique<LibsndfileDecoder>());
#endif
		return nullptr;
	}

	// Inform about WMA issue
	if (!memcmp(magic, wma_magic, 4)) {
		return std::make_unique<WMAUnsupportedFormatDecoder>();
	}

	// Test for tracker modules
#ifdef HAVE_LIBXMP
	if (XMPDecoder::IsModule(stream)) {
		return add_resampler(std::make_unique<XMPDecoder>());
	}
#endif

	// False positive MP3s should be prevented before by checking for common headers
#ifdef HAVE_LIBMPG123
	static bool mpg123_works = true;
	if (mpg123_works) {
		auto mp3dec = add_resampler(std::make_unique<Mpg123Decoder>());
		if (mp3dec->WasInited()) {
			if (strncmp(magic, "ID3", 3) == 0) {
				return mp3dec;
			}

			bool is_aac = std::any_of(aac_magics.begin(), aac_magics.end(), [&](auto& mpeg4_magic) {
				return memcmp(magic, mpeg4_magic, 2);
			});
			if (is_aac) {
				return nullptr;
			}

			// Parsing MP3s seems to be the only reliable way to detect them
			if (Mpg123Decoder::IsMp3(stream)) {
				stream.clear();
				stream.seekg(0, std::ios_base::beg);
				return mp3dec;
			}
		} else {
			mpg123_works = false;
		}
	}
#endif

	stream.clear();
	stream.seekg(0, std::ios::ios_base::beg);
	return nullptr;
}

void AudioDecoder::Pause() {
	paused = true;
}

void AudioDecoder::Resume() {
	paused = false;
}

void AudioDecoder::Update(std::chrono::microseconds delta) {
	if (fade_time <= 0ms) {
		return;
	}

	fade_time -= delta;

	volume += static_cast<float>(std::chrono::duration_cast<std::chrono::microseconds>(delta).count()) * delta_volume_step;
	volume = Utils::Clamp(static_cast<float>(volume), 0.0f, 100.0f);
	ApplyLogVolume();
}

StereoVolume AudioDecoder::GetVolume() const {
	return log_volume;
}

void AudioDecoder::SetVolume(int new_volume) {
	volume = Utils::Clamp(static_cast<float>(new_volume), 0.0f, 100.0f);
	ApplyLogVolume();
}

void AudioDecoder::SetFade(int end, std::chrono::milliseconds duration) {
	fade_time = 0ms;

	if (duration <= 0ms) {
		SetVolume(end);
		return;
	}

	fade_volume_end = end;
	fade_time = duration;
	delta_volume_step = (static_cast<float>(fade_volume_end) - volume) / fade_time.count();
}

void AudioDecoder::SetBalance(int balance) {
	AudioDecoderBase::SetBalance(balance);
	ApplyLogVolume();
}

int AudioDecoder::GetSamplesizeForFormat(AudioDecoderBase::Format format) {
	switch (format) {
		case AudioDecoderBase::Format::S8:
		case AudioDecoderBase::Format::U8:
			return 1;
		case AudioDecoderBase::Format::S16:
		case AudioDecoderBase::Format::U16:
			return 2;
		case AudioDecoderBase::Format::S32:
		case AudioDecoderBase::Format::U32:
		case AudioDecoderBase::Format::F32:
			return 4;
	}

	assert(false && "Bad format");
	return -1;
}

void AudioDecoder::ApplyLogVolume() {
	float base_gain = AdjustVolume(volume);
	int balance = GetBalance();
	float left_gain = 1.f, right_gain = 1.f;
	constexpr float pan_exp = 0.5012f;
	if (balance <= 50) {
		right_gain = std::pow(pan_exp, (50 - balance) / 10.f);
	} else {
		left_gain = std::pow(pan_exp, (balance - 50) / 10.f);
	}
	log_volume.left_volume = base_gain * left_gain;
	log_volume.right_volume = base_gain * right_gain;
}
