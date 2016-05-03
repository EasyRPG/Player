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
#include <cstring>

#include "system.h"

#ifdef HAVE_SDL_MIXER

#include "baseui.h"
#include "audio_sdl.h"
#include "filefinder.h"
#include "output.h"

#ifdef EMSCRIPTEN
#  include <emscripten.h>
#elif defined(_WIN32)
#  include "util_win.h"
#endif

namespace {
	void bgm_played_once() {
		if (DisplayUi)
			static_cast<SdlAudio&>(Audio()).BGM_OnPlayedOnce();
	}

	void bgs_played_once(int channel) {
		if (DisplayUi && channel == static_cast<SdlAudio&>(Audio()).BGS_GetChannel())
			bgm_played_once();
	}

	void callback(void *udata, Uint8 *stream, int len) {
		static std::vector<uint8_t> buffer;
		buffer.resize(len);

		SdlAudio* audio = static_cast<SdlAudio*>(udata);
		len = audio->GetDecoder()->Decode(buffer.data(), len);

		if (audio->GetDecoder()->IsFinished()) {
			Mix_HookMusic(nullptr, nullptr);
		} else {
			SDL_AudioCVT& cvt = audio->GetAudioCVT();
			if (cvt.needed) {
				static std::vector<uint8_t> cvt_buffer;
				cvt_buffer.resize(len * cvt.len_mult);
				cvt.buf = cvt_buffer.data();
				cvt.len = len;
				memcpy(cvt.buf, buffer.data(), len);
				SDL_ConvertAudio(&cvt);
			} else {
				cvt.len_cvt = len;
				cvt.buf = buffer.data();
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
			return AUDIO_U16;
		case AudioDecoder::Format::S16:
			return AUDIO_S16;
		default:
			assert(false);
		}
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
		default:
			assert(false);
		}
	}
}

SdlAudio::SdlAudio() :
	bgm_volume(0),
	bgs_channel(0),
	bgs_playing(false),
	me_channel(0),
	me_stopped_bgm(false)
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

SdlAudio::~SdlAudio() {
	Mix_CloseAudio();
}

void SdlAudio::BGM_OnPlayedOnce() {
#if SDL_MAJOR_VERSION>1
	// SDL2_mixer produces noise when playing wav.
	// Workaround: Use Mix_LoadWAV
	// https://bugzilla.libsdl.org/show_bug.cgi?id=2094
	if (bgs_playing) {
		if (!bgs_stop) {
			played_once = true;
			// Play indefinitely without fade-in
			Mix_PlayChannel(bgs_channel, bgs.get(), -1);
			bgs_channel = -1;
		}
		return;
	}
#endif

	if (!me_stopped_bgm && !bgm_stop) {
		played_once = true;
		// Play indefinitely without fade-in
		if (Mix_GetMusicType(bgm.get()) != MUS_MP3_MAD)
			Mix_PlayMusic(bgm.get(), -1);
	}
}

void SdlAudio::BGM_Play(std::string const& file, int volume, int pitch, int fadein) {
	bgm_stop = false;
	played_once = false;
	std::string const path = FileFinder::FindMusic(file);
	if (path.empty()) {
		Output::Debug("Music not found: %s", file.c_str());
		return;
	}

	FILE* filehandle = FileFinder::fopenUTF8(path, "rb");

	audio_decoder = AudioDecoder::Create(&filehandle, path);

	if (audio_decoder) {
		audio_decoder->Open(filehandle);
		audio_decoder->SetLooping(true);
		bgm_starttick = SDL_GetTicks();

		int audio_rate;
		Uint16 sdl_format;
		int audio_channels;
		if (!Mix_QuerySpec(&audio_rate, &sdl_format, &audio_channels)) {
			Output::Warning("Couldn't query mixer spec.\n %s", Mix_GetError());
			return;
		}
		AudioDecoder::Format audio_format = sdl_format_to_format(sdl_format);

		audio_decoder->SetFormat(audio_rate, audio_format, audio_channels);

		int device_rate;
		AudioDecoder::Format device_format;
		int device_channels;
		audio_decoder->GetFormat(device_rate, device_format, device_channels);

		// Don't care if successful, always build cvt
		SDL_BuildAudioCVT(&cvt, format_to_sdl_format(device_format), (int)device_channels, device_rate, sdl_format, audio_channels, audio_rate);

		audio_decoder->SetFade(0, volume, fadein);
		audio_decoder->SetPitch(pitch);

		Mix_HookMusic(callback, this);

		return;
	}

	SDL_RWops *rw = SDL_RWFromFile(path.c_str(), "rb");

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
#if SDL_MIXER_MAJOR_VERSION>1
		// Try unsupported SDL_mixer ADPCM playback with SDL
		if (strcmp(Mix_GetError(), "Unknown WAVE data format") == 0) {
			bgm_stop = true;
			BGS_Play(file, volume, 0, fadein);
			return;
		}
#endif
		Output::Warning("Couldn't load %s BGM.\n%s", file.c_str(), Mix_GetError());
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
	if (!me_stopped_bgm &&
#ifdef _WIN32
		(mtype == MUS_MID && WindowsUtils::GetWindowsVersion() >= 6
			? Mix_PlayMusic(bgm.get(), 0) : Mix_FadeInMusic(bgm.get(), 0, fadein))
#else
		Mix_FadeInMusic(bgm.get(), 0, fadein)
#endif
		== -1) {
			Output::Warning("Couldn't play %s BGM.\n%s", file.c_str(), Mix_GetError());
			return;
	}

	Mix_HookMusicFinished(&bgm_played_once);
}

void SdlAudio::BGM_Pause() {
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

void SdlAudio::BGM_Resume() {
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

void SdlAudio::BGM_Stop() {
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
	me_stopped_bgm = false;
}

bool SdlAudio::BGM_PlayedOnce() {
	if (audio_decoder) {
		return audio_decoder->GetLoopCount() > 0;
	}

	return played_once;
}

unsigned SdlAudio::BGM_GetTicks() {
	if (audio_decoder) {
		return audio_decoder->GetTicks();
	}

	// TODO: Implement properly. This is an approximation.
	return SDL_GetTicks() - bgm_starttick;
}

void SdlAudio::BGM_Volume(int volume) {
	bgm_volume = volume * MIX_MAX_VOLUME / 100;
	Mix_VolumeMusic(bgm_volume);
}

void SdlAudio::BGM_Pitch(int pitch) {
	if (audio_decoder) {
		audio_decoder->SetPitch(pitch);
	}
}

void SdlAudio::BGM_Fade(int fade) {
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
	me_stopped_bgm = false;
}

void SdlAudio::BGS_Play(std::string const& file, int volume, int /* pitch */, int fadein) {
	std::string const path = FileFinder::FindMusic(file);
	if (path.empty()) {
		Output::Debug("Music not found: %s", file.c_str());
		return;
	}

	bgs.reset(Mix_LoadWAV(path.c_str()), &Mix_FreeChunk);
	if (!bgs) {
		Output::Warning("Couldn't load %s BGS.\n%s", file.c_str(), Mix_GetError());
		return;
	}
	bgs_channel = Mix_FadeInChannel(-1, bgs.get(), 0, fadein);
	Mix_Volume(bgs_channel, volume * MIX_MAX_VOLUME / 100);
	if (bgs_channel == -1) {
		Output::Warning("Couldn't play %s BGS.\n%s", file.c_str(), Mix_GetError());
		return;
	}
	bgs_playing = true;
	bgs_stop = false;

#if SDL_MAJOR_VERSION>1
	Mix_ChannelFinished(bgs_played_once);
#endif
}

void SdlAudio::BGS_Pause() {
	if (Mix_Playing(bgs_channel)) {
		Mix_Pause(bgs_channel);
	}
}

void SdlAudio::BGS_Resume() {
	Mix_Resume(bgs_channel);
}

void SdlAudio::BGS_Stop() {
	if (Mix_Playing(bgs_channel)) {
		bgs_stop = true;
		Mix_HaltChannel(bgs_channel);
		bgs_channel = -1;
		bgs_playing = false;
	}
}

void SdlAudio::BGS_Fade(int fade) {
	bgs_stop = true;
	Mix_FadeOutChannel(bgs_channel, fade);
	bgs_channel = -1;
	bgs_playing = false;
}

int SdlAudio::BGS_GetChannel() const {
	return bgs_channel;
}

void SdlAudio::ME_Play(std::string const& file, int volume, int /* pitch */, int fadein) {
	std::string const path = FileFinder::FindMusic(file);
	if (path.empty()) {
		Output::Debug("Music not found: %s", file.c_str());
		return;
	}
	me.reset(Mix_LoadWAV(path.c_str()), &Mix_FreeChunk);
	if (!me) {
		Output::Warning("Couldn't load %s ME.\n%s", file.c_str(), Mix_GetError());
		return;
	}
	me_channel = Mix_FadeInChannel(-1, me.get(), 0, fadein);
	Mix_Volume(me_channel, volume * MIX_MAX_VOLUME / 100);
	if (me_channel == -1) {
		Output::Warning("Couldn't play %s ME.\n%s", file.c_str(), Mix_GetError());
		return;
	}
	me_stopped_bgm = (Mix_PlayingMusic() == 1);
	// Mix_ChannelFinished(me_finish);
}

void SdlAudio::ME_Stop() {
	if (Mix_Playing(me_channel)) {
		Mix_HaltChannel(me_channel);
	}
}

void SdlAudio::ME_Fade(int fade) {
	Mix_FadeOutChannel(me_channel, fade);
}

void SdlAudio::SE_Play(std::string const& file, int volume, int /* pitch */) {
	std::string const path = FileFinder::FindSound(file);
	if (path.empty()) {
		Output::Debug("Sound not found: %s", file.c_str());
		return;
	}
	std::shared_ptr<Mix_Chunk> sound(Mix_LoadWAV(path.c_str()), &Mix_FreeChunk);
	if (!sound) {
		Output::Warning("Couldn't load %s SE.\n%s", file.c_str(), Mix_GetError());
		return;
	}
	int channel = Mix_PlayChannel(-1, sound.get(), 0);
	Mix_Volume(channel, volume * MIX_MAX_VOLUME / 100);
	if (channel == -1) {
		Output::Warning("Couldn't play %s SE.\n%s", file.c_str(), Mix_GetError());
		return;
	}
	sounds[channel] = sound;
}

void SdlAudio::SE_Stop() {
	for (sounds_type::iterator i = sounds.begin(); i != sounds.end(); ++i) {
		if (Mix_Playing(i->first)) Mix_HaltChannel(i->first);
	}
	sounds.clear();
}

void SdlAudio::Update() {
	if (audio_decoder && bgm_starttick > 0) {
		int t = DisplayUi->GetTicks();
		audio_decoder->Update(t - bgm_starttick);
		bgm_starttick = t;
	}
}

AudioDecoder* SdlAudio::GetDecoder() {
	return audio_decoder.get();
}

SDL_AudioCVT& SdlAudio::GetAudioCVT() {
	return cvt;
}

#endif
