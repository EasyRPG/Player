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

#include <cassert>
#include <cmath>
#include <cstring>

#include "audio_sdl_mixer.h"

#if defined(HAVE_SDL_MIXER) && defined(SUPPORT_AUDIO)

#include "audio_secache.h"
#include "baseui.h"
#include "filefinder.h"
#include "output.h"

#ifdef EMSCRIPTEN
#  include <emscripten.h>
#elif defined(_WIN32)
#  include "util_win.h"
#endif

#if WANT_FMMIDI == 2
#  include "decoder_fmmidi.h"
#endif

#define BGS_CHANNEL_NUM 0

namespace {
	void bgm_played_once() {
		if (DisplayUi)
			static_cast<SdlMixerAudio&>(Audio()).BGM_OnPlayedOnce();
	}

	void bgs_played_once(int channel) {
		if (DisplayUi && channel == BGS_CHANNEL_NUM)
			bgm_played_once();
	}

	void callback(void *udata, Uint8 *stream, int stream_size) {
		static std::vector<uint8_t> buffer;

		SdlMixerAudio* audio = static_cast<SdlMixerAudio*>(udata);

		SDL_AudioCVT& cvt = audio->GetAudioCVT();
		int out_len = stream_size;
		if (cvt.needed) {
			// Calculate how many data is needed to fill the buffer after converting it
			double d = out_len / cvt.len_ratio;
			out_len = (int)std::ceil(d);
			out_len += out_len & 1;
		}

		buffer.resize(out_len);

		out_len = audio->GetDecoder()->Decode(buffer.data(), out_len);
		if (out_len == -1) {
			Output::Warning("Couldn't decode BGM. %s", audio->GetDecoder()->GetError().c_str());
			Mix_HookMusic(nullptr, nullptr);
			return;
		}

		if (audio->GetDecoder()->IsFinished()) {
			Mix_HookMusic(nullptr, nullptr);
		} else {
			if (cvt.needed) {
				static std::vector<uint8_t> cvt_buffer;
				cvt_buffer.resize(out_len * cvt.len_mult);
				cvt.buf = cvt_buffer.data();
				cvt.len = out_len;
				memcpy(cvt.buf, buffer.data(), out_len);
				SDL_ConvertAudio(&cvt);
			} else {
				cvt.len_cvt = out_len;
				cvt.buf = buffer.data();
			}

			if (cvt.len_cvt > stream_size) {
				cvt.len_cvt = stream_size;
			}

#if SDL_MIXER_MAJOR_VERSION>1
			SDL_MixAudioFormat(stream, reinterpret_cast<const Uint8*>(cvt.buf), MIX_DEFAULT_FORMAT, cvt.len_cvt, audio->GetDecoder()->GetVolume());
#else
			SDL_MixAudio(stream, reinterpret_cast<const Uint8*>(cvt.buf), cvt.len_cvt, audio->GetDecoder()->GetVolume());
#endif
		}
	}

	int format_to_sdl_format(AudioDecoder::Format format) {
		switch (format) {
		case AudioDecoder::Format::U8:
			return AUDIO_U8;
		case AudioDecoder::Format::S8:
			return AUDIO_S8;
		case AudioDecoder::Format::U16:
			return AUDIO_U16SYS;
		case AudioDecoder::Format::S16:
			return AUDIO_S16SYS;
#if SDL_MIXER_MAJOR_VERSION>1
		case AudioDecoder::Format::S32:
			return AUDIO_S32;
		case AudioDecoder::Format::F32:
			return AUDIO_F32;
#endif
		default:
			assert(false);
		}

		return -1;
	}

	AudioDecoder::Format sdl_format_to_format(Uint16 format) {
		switch (format) {
		case AUDIO_U8:
			return AudioDecoder::Format::U8;
		case AUDIO_S8:
			return AudioDecoder::Format::S8;
		case AUDIO_U16SYS:
			return AudioDecoder::Format::U16;
		case AUDIO_S16SYS:
			return AudioDecoder::Format::S16;
#if SDL_MIXER_MAJOR_VERSION>1
		case AUDIO_S32:
			return AudioDecoder::Format::S32;
		case AUDIO_F32:
			return AudioDecoder::Format::F32;
#endif
		default:
			assert(false);
		}

		return (AudioDecoder::Format)-1;
	}
}

SdlMixerAudio::SdlMixerAudio() :
	bgm_volume(0),
	bgs_playing(false)
{
	if (!(SDL_WasInit(SDL_INIT_AUDIO) & SDL_INIT_AUDIO)) {
		if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
			Output::Error("Couldn't initialize audio.\n%s", SDL_GetError());
		}
	}
#ifdef GEKKO
	// Wii's DSP works at 32kHz natively
	int const frequency = 32000;
#elif defined(EMSCRIPTEN)
	// Get preferred sample rate from Browser (-> OS)
	int const frequency = EM_ASM_INT_V({
		var context;
		try {
			context = new AudioContext();
		} catch (e) {
			context = new webkitAudioContext();
		}
		return context.sampleRate;
	});
#else
	int const frequency = 44100;
#endif

#ifdef EMSCRIPTEN
	// FIXME: this requires SDL_mixer => 2.0.2 but not tagged yet (using Hg)
	if (Mix_OpenAudioDevice(frequency, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 2048,
		NULL, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE) < 0)
#else
	if (Mix_OpenAudio(frequency, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 2048) < 0)
#endif
		Output::Error("Couldn't initialize audio mixer.\n%s", Mix_GetError());

	Mix_AllocateChannels(32); // Default is MIX_CHANNELS = 8

	// Reserve the 1st channel for BGS
	Mix_ReserveChannels(1);

	int audio_rate;
	Uint16 audio_format;
	int audio_channels;
	if (Mix_QuerySpec(&audio_rate, &audio_format, &audio_channels)) {
		const char *audio_format_str;
		switch (audio_format) {
			case AUDIO_U8: audio_format_str = "U8"; break;
			case AUDIO_S8: audio_format_str = "S8"; break;
			case AUDIO_U16LSB: audio_format_str = "U16LSB"; break;
			case AUDIO_S16LSB: audio_format_str = "S16LSB"; break;
			case AUDIO_U16MSB: audio_format_str = "U16MSB"; break;
			case AUDIO_S16MSB: audio_format_str = "S16MSB"; break;
#if SDL_MIXER_MAJOR_VERSION>1
			case AUDIO_S32LSB: audio_format_str = "S32LSB"; break;
			case AUDIO_S32MSB: audio_format_str = "S32MSB"; break;
			case AUDIO_F32LSB: audio_format_str = "F32LSB"; break;
			case AUDIO_F32MSB: audio_format_str = "F32MSB"; break;
#endif
			default: audio_format_str = "Unknown"; break;
		}
		Output::Debug("Opened audio at %d Hz (%s), format: %s",
			audio_rate,
			(audio_channels > 2) ? "surround" : (audio_channels > 1) ? "stereo" : "mono",
			audio_format_str);
	} else {
		Output::Debug("Mix_QuerySpec error: %s", Mix_GetError());
	}
}

SdlMixerAudio::~SdlMixerAudio() {
	// Must be reset otherwise Player segfaults when SDL is reinitialized (Android)
	Mix_HookMusic(nullptr, nullptr);

	Mix_CloseAudio();
}

void SdlMixerAudio::BGM_OnPlayedOnce() {
#if SDL_MAJOR_VERSION>1
	// SDL2_mixer produces noise when playing wav.
	// Workaround: Use Mix_LoadWAV
	// https://bugzilla.libsdl.org/show_bug.cgi?id=2094
	if (bgs_playing) {
		if (!bgs_stop) {
			played_once = true;
			// Play indefinitely without fade-in
			Mix_PlayChannel(BGS_CHANNEL_NUM, bgs.get(), -1);
		}
		return;
	}
#endif

	if (!bgm_stop) {
		played_once = true;
		// Play indefinitely without fade-in
		if (Mix_GetMusicType(bgm.get()) != MUS_MP3_MAD)
			Mix_PlayMusic(bgm.get(), -1);
	}
}

void SdlMixerAudio::BGM_Play(std::string const& file, int volume, int pitch, int fadein) {
	FILE* filehandle = FileFinder::fopenUTF8(file, "rb");
	if (!filehandle) {
		Output::Warning("Music not readable: %s", FileFinder::GetPathInsideGamePath(file).c_str());
		return;
	}
	audio_decoder = AudioDecoder::Create(filehandle, file);
	if (audio_decoder) {
		SetupAudioDecoder(filehandle, file, volume, pitch, fadein);
		return;
	}
	fclose(filehandle);

	SDL_RWops *rw = SDL_RWFromFile(file.c_str(), "rb");

	bgm_stop = false;
	played_once = false;

#if SDL_MIXER_MAJOR_VERSION>1
	bgm.reset(Mix_LoadMUS_RW(rw, 0), &Mix_FreeMusic);
#else
	bgm.reset(Mix_LoadMUS_RW(rw), &Mix_FreeMusic);
#endif

#if SDL_MIXER_MAJOR_VERSION>1
	// SDL2_mixer bug, see above
	if (bgs_playing) {
		BGS_Stop();
	}
#endif

	if (!bgm) {
#if WANT_FMMIDI == 2
		// Fallback to FMMIDI when SDL Midi failed
		char magic[4] = { 0 };
		filehandle = FileFinder::fopenUTF8(file, "rb");
		if (!filehandle) {
			Output::Warning("Music not readable: %s", FileFinder::GetPathInsideGamePath(file).c_str());
			return;
		}
		if (fread(magic, 4, 1, filehandle) != 1)
			return;
		fseek(filehandle, 0, SEEK_SET);
		if (!strncmp(magic, "MThd", 4)) {
			Output::Debug("FmMidi fallback: %s", file.c_str());
			audio_decoder.reset(new FmMidiDecoder());
			SetupAudioDecoder(filehandle, file, volume, pitch, fadein);
			return;
		}
#endif

#if SDL_MIXER_MAJOR_VERSION>1
		// Try unsupported SDL_mixer ADPCM playback with SDL
		if (strcmp(Mix_GetError(), "Unknown WAVE data format") == 0) {
			bgm_stop = true;
			BGS_Play(file, volume, 0, fadein);
			return;
		}
#endif
		Output::Warning("Couldn't load %s BGM. %s", FileFinder::GetPathInsideGamePath(file).c_str(), Mix_GetError());
		return;
	}

	bgm_starttick = SDL_GetTicks();

	Mix_MusicType mtype = Mix_GetMusicType(bgm.get());

#if SDL_MAJOR_VERSION>1
	if (mtype == MUS_WAV || mtype == MUS_OGG) {
		BGM_Stop();
		BGS_Play(file, volume, 0, fadein);
		return;
	}
#endif

	BGM_Volume(volume);
	if (
#ifdef _WIN32
		(mtype == MUS_MID && WindowsUtils::GetWindowsVersion() >= 6
			? Mix_PlayMusic(bgm.get(), 0) : Mix_FadeInMusic(bgm.get(), 0, fadein))
#else
		Mix_FadeInMusic(bgm.get(), 0, fadein)
#endif
		== -1) {
			Output::Warning("Couldn't play %s BGM. %s", FileFinder::GetPathInsideGamePath(file).c_str(), Mix_GetError());
			return;
	}

	Mix_HookMusicFinished(&bgm_played_once);
}

void SdlMixerAudio::SetupAudioDecoder(FILE* handle, const std::string& file, int volume, int pitch, int fadein) {
	if (!audio_decoder->Open(handle)) {
		Output::Warning("Couldn't play %s BGM. %s", FileFinder::GetPathInsideGamePath(file).c_str(), audio_decoder->GetError().c_str());
		audio_decoder.reset();
		return;
	}

	// Can't use BGM_Stop here because it destroys the audio_decoder
#if SDL_MAJOR_VERSION>1
	// SDL2_mixer bug, see above
	if (bgs_playing) {
		BGS_Stop();
	} else {
		Mix_HaltMusic();
	}
#else
	Mix_HaltMusic();
#endif

	audio_decoder->SetLooping(true);
	bgm_starttick = SDL_GetTicks();

	int audio_rate;
	Uint16 sdl_format;
	int audio_channels;
	if (!Mix_QuerySpec(&audio_rate, &sdl_format, &audio_channels)) {
		Output::Warning("Couldn't query mixer spec. %s", Mix_GetError());
		return;
	}
	AudioDecoder::Format audio_format = sdl_format_to_format(sdl_format);

	int target_rate = audio_rate;
	if (audio_decoder->GetType() == "midi") {
		// FM Midi is very CPU heavy and the difference between 44100 and 22050
		// is not hearable for MIDI
		target_rate /= 2;
	}
	audio_decoder->SetFormat(target_rate, audio_format, audio_channels);

	int device_rate;
	AudioDecoder::Format device_format;
	int device_channels;
	audio_decoder->GetFormat(device_rate, device_format, device_channels);

	// Don't care if successful, always build cvt
	SDL_BuildAudioCVT(&cvt, format_to_sdl_format(device_format), (int)device_channels, device_rate, sdl_format, audio_channels, audio_rate);

	audio_decoder->SetFade(0, volume, fadein);
	audio_decoder->SetPitch(pitch);

	Mix_HookMusic(callback, this);
}

void SdlMixerAudio::BGM_Pause() {
	if (audio_decoder) {
		audio_decoder->Pause();
		return;
	}

	// Midi pause is not supported... (for some systems -.-)
#if SDL_MAJOR_VERSION>1
	// SDL2_mixer bug, see above
	if (bgs_playing) {
		BGS_Pause();
		return;
	}
#endif
	Mix_PauseMusic();
}

void SdlMixerAudio::BGM_Resume() {
	if (audio_decoder) {
		bgm_starttick = SDL_GetTicks();
		audio_decoder->Resume();
		return;
	}

#if SDL_MAJOR_VERSION>1
	// SDL2_mixer bug, see above
	if (bgs_playing) {
		BGS_Resume();
		return;
	}
#endif
	Mix_ResumeMusic();
}

void SdlMixerAudio::BGM_Stop() {
	Mix_HookMusic(nullptr, nullptr);
	audio_decoder.reset();

#if SDL_MAJOR_VERSION>1
	// SDL2_mixer bug, see above
	if (bgs_playing) {
		BGS_Stop();
		return;
	}
#endif
	bgm_stop = true;
	Mix_HaltMusic();
}

bool SdlMixerAudio::BGM_PlayedOnce() const {
	if (audio_decoder) {
		return audio_decoder->GetLoopCount() > 0;
	}

	return played_once;
}

bool SdlMixerAudio::BGM_IsPlaying() const {
	return audio_decoder || !bgm_stop || !bgs_stop;
}

unsigned SdlMixerAudio::BGM_GetTicks() const {
	if (audio_decoder) {
		return audio_decoder->GetTicks();
	}

	// TODO: Implement properly. This is an approximation.
	return SDL_GetTicks() - bgm_starttick;
}

void SdlMixerAudio::BGM_Volume(int volume) {
	if (audio_decoder) {
		audio_decoder->SetVolume(volume);
		return;
	}

#if SDL_MAJOR_VERSION>1
	// SDL2_mixer bug, see above
	if (bgs_playing) {
		BGS_Volume(volume);
		return;
	}
#endif

	bgm_volume = volume * MIX_MAX_VOLUME / 100;
	Mix_VolumeMusic(bgm_volume);
}

void SdlMixerAudio::BGM_Pitch(int pitch) {
	if (audio_decoder) {
		audio_decoder->SetPitch(pitch);
	}

	// Not supported by SDL
}

void SdlMixerAudio::BGM_Fade(int fade) {
	if (audio_decoder) {
		bgm_starttick = DisplayUi->GetTicks();
		audio_decoder->SetFade(audio_decoder->GetVolume(), 0, fade);
		return;
	}

#ifdef _WIN32
	// FIXME: Because of design change in Vista and higher reducing Midi volume
	// alters volume of whole application and mutes it forever when restarted.
	// Fading out midi music was disabled for Windows.
	if (Mix_GetMusicType(bgm.get()) == MUS_MID &&
		WindowsUtils::GetWindowsVersion() >= 6) {
		BGM_Stop();
		return;
	}
#endif

	bgm_stop = true;

#if SDL_MAJOR_VERSION>1
	// SDL2_mixer bug, see above
	if (bgs_playing) {
		BGS_Fade(fade);
		return;
	}
#endif

	Mix_FadeOutMusic(fade);
}

void SdlMixerAudio::BGS_Play(std::string const& file, int volume, int /* pitch */, int fadein) {
	bgs.reset(Mix_LoadWAV(file.c_str()), &Mix_FreeChunk);
	if (!bgs) {
		Output::Warning("Couldn't load %s BGS. %s", FileFinder::GetPathInsideGamePath(file).c_str(), Mix_GetError());
		return;
	}

	Mix_Volume(BGS_CHANNEL_NUM, volume * MIX_MAX_VOLUME / 100);
	int channel = Mix_FadeInChannel(BGS_CHANNEL_NUM, bgs.get(), 0, fadein);
	if (channel != 0) {
		Output::Warning("Couldn't play %s BGS. %s", FileFinder::GetPathInsideGamePath(file).c_str(), Mix_GetError());
		return;
	}
	bgs_playing = true;
	bgs_stop = false;

#if SDL_MAJOR_VERSION>1
	Mix_ChannelFinished(bgs_played_once);
#endif
}

void SdlMixerAudio::BGS_Pause() {
	if (Mix_Playing(BGS_CHANNEL_NUM)) {
		Mix_Pause(BGS_CHANNEL_NUM);
	}
}

void SdlMixerAudio::BGS_Resume() {
	Mix_Resume(BGS_CHANNEL_NUM);
}

void SdlMixerAudio::BGS_Stop() {
	if (Mix_Playing(BGS_CHANNEL_NUM)) {
		bgs_stop = true;
		Mix_HaltChannel(BGS_CHANNEL_NUM);
		bgs_playing = false;
	}
}

void SdlMixerAudio::BGS_Fade(int fade) {
	bgs_stop = true;
	Mix_FadeOutChannel(BGS_CHANNEL_NUM, fade);
	bgs_playing = false;
}

void SdlMixerAudio::BGS_Volume(int volume) {
	Mix_Volume(BGS_CHANNEL_NUM, volume * MIX_MAX_VOLUME / 100);
}

void SdlMixerAudio::SE_Play(std::string const& file, int volume, int pitch) {
	std::unique_ptr<AudioSeCache> cache = AudioSeCache::Create(file);
	std::shared_ptr<Mix_Chunk> sound;
	AudioSeRef se_ref = nullptr;

	if (cache) {
		int audio_rate;
		Uint16 sdl_format;
		int audio_channels;
		if (!Mix_QuerySpec(&audio_rate, &sdl_format, &audio_channels)) {
			Output::Warning("Couldn't query mixer spec. %s", Mix_GetError());
			return;
		}
		AudioDecoder::Format audio_format = sdl_format_to_format(sdl_format);

		// When this fails the resampler is probably not compiled in and output will be garbage, just use SDL
		if (cache->SetFormat(audio_rate, audio_format, audio_channels)) {
			cache->SetPitch(pitch);

			se_ref = cache->Decode();

			sound.reset(Mix_QuickLoad_RAW(se_ref->buffer.data(), se_ref->buffer.size()), &Mix_FreeChunk);

			if (!sound) {
				Output::Warning("Couldn't load %s SE. %s", FileFinder::GetPathInsideGamePath(file).c_str(), Mix_GetError());
			}
		}
	}

	if (!sound) {
		sound.reset(Mix_LoadWAV(file.c_str()), &Mix_FreeChunk);
		if (!sound) {
			Output::Warning("Couldn't load %s SE. %s", FileFinder::GetPathInsideGamePath(file).c_str(), Mix_GetError());
			return;
		}
	}

	int channel = Mix_PlayChannel(-1, sound.get(), 0);
	Mix_Volume(channel, volume * MIX_MAX_VOLUME / 100);
	if (channel == -1) {
		Output::Warning("Couldn't play %s SE. %s", FileFinder::GetPathInsideGamePath(file).c_str(), Mix_GetError());
		return;
	}
	sounds[channel].first = sound;
	sounds[channel].second = se_ref;
}

void SdlMixerAudio::SE_Stop() {
	for (sounds_type::iterator i = sounds.begin(); i != sounds.end(); ++i) {
		if (Mix_Playing(i->first)) Mix_HaltChannel(i->first);
	}
	sounds.clear();
}

void SdlMixerAudio::Update() {
	if (audio_decoder && bgm_starttick > 0) {
		int t = DisplayUi->GetTicks();
		audio_decoder->Update(t - bgm_starttick);
		bgm_starttick = t;
	}
}

AudioDecoder* SdlMixerAudio::GetDecoder() {
	return audio_decoder.get();
}

SDL_AudioCVT& SdlMixerAudio::GetAudioCVT() {
	return cvt;
}

#endif
