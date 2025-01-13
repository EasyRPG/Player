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

#include "system.h"
#include "audio.h"
#include "decoder_fluidsynth.h"

#if defined(HAVE_FLUIDSYNTH) || defined(HAVE_FLUIDLITE)

#include <cassert>
#include "filefinder.h"
#include "output.h"

#if FLUIDSYNTH_VERSION_MAJOR >= 3 || (FLUIDSYNTH_VERSION_MAJOR == 2 && FLUIDSYNTH_VERSION_MINOR >= 2)
#define FLUIDSYNTH_22_OR_NEWER
#endif

#ifdef HAVE_FLUIDSYNTH
static void* vio_open(const char* filename) {
#else
static void* vio_open(fluid_fileapi_t*, const char* filename) {
#endif
	auto is = FileFinder::Game().OpenInputStream(filename);
	if (!is) {
		is = FileFinder::Root().OpenInputStream(filename);
		if (!is) {
			return nullptr;
		}
	}
	return new Filesystem_Stream::InputStream { std::move(is) };
}

#ifdef FLUIDSYNTH_22_OR_NEWER
static int vio_read(void *ptr, fluid_long_long_t count, void* userdata) {
#else
static int vio_read(void *ptr, int count, void* userdata) {
#endif
	auto* f = reinterpret_cast<Filesystem_Stream::InputStream*>(userdata);
	return f->read(reinterpret_cast<char*>(ptr), count).gcount();
}

#ifdef FLUIDSYNTH_22_OR_NEWER
static int vio_seek(void* userdata, fluid_long_long_t offset, int origin) {
#else
static int vio_seek(void* userdata, long offset, int origin) {
#endif
	auto* f = reinterpret_cast<Filesystem_Stream::InputStream*>(userdata);
	if (f->eof()) f->clear(); // emulate behaviour of fseek

	f->seekg(offset, Filesystem_Stream::CSeekdirToCppSeekdir(origin));

	return f->tellg();
}

#ifdef FLUIDSYNTH_22_OR_NEWER
static fluid_long_long_t vio_tell(void* userdata) {
#else
static long vio_tell(void* userdata) {
#endif
	auto* f = reinterpret_cast<Filesystem_Stream::InputStream*>(userdata);
	return f->tellg();
}

static int vio_close(void* userdata) {
	auto stream_ref = reinterpret_cast<Filesystem_Stream::InputStream*>(userdata);
	delete stream_ref;

	return 0;
}

#ifdef HAVE_FLUIDLITE
static fluid_fileapi_t fluidlite_vio = {
	nullptr,
	nullptr,
	vio_open,
	vio_read,
	vio_seek,
	vio_close,
	vio_tell
};
#endif

struct FluidSynthDeleter {
	void operator()(fluid_settings_t* s) const {
		delete_fluid_settings(s);
	}

	void operator()(fluid_synth_t* s) const {
		delete_fluid_synth(s);
	}
};

namespace {
	bool once = false;
	bool init = false;

	std::unique_ptr<fluid_settings_t, FluidSynthDeleter> global_settings;
	std::unique_ptr<fluid_synth_t, FluidSynthDeleter> global_synth;
	std::unique_ptr<fluid_synth_t, FluidSynthDeleter> pending_global_synth;

	int instances = 0;
}

static bool load_default_sf(std::string& status_message, fluid_synth_t* syn) {
	// Attempt loading a soundfont
	std::vector<std::string> sf_paths;
	std::string preferred_soundfont = Audio().GetFluidsynthSoundfont();
	if (!preferred_soundfont.empty()) {
		sf_paths.emplace_back(preferred_soundfont);
	}
	sf_paths.emplace_back("easyrpg.soundfont");

#if FLUIDSYNTH_VERSION_MAJOR >= 2
	char* default_sf = nullptr;
	if (fluid_settings_dupstr(global_settings.get(), "synth.default-soundfont", &default_sf) == 0) {
		if (default_sf != nullptr && default_sf[0] != '\0') {
			sf_paths.emplace_back(default_sf);
		}
	}
	fluid_free(default_sf);
#endif

	if (getenv("SDL_SOUNDFONTS")) {
		auto sdl_sfs = Utils::Tokenize(getenv("SDL_SOUNDFONTS"), [](char32_t t) {
#ifdef _WIN32
			return t == ';';
#else
			return t == ':' || t == ';';
#endif
		});
		sf_paths.insert(sf_paths.end(), sdl_sfs.begin(), sdl_sfs.end());
	}

#ifdef SYSTEM_DESKTOP_LINUX_BSD_MACOS
	auto sf_files = {"FluidR3_GM.sf2"};
	for (const auto& sf_file: sf_files) {
		sf_paths.emplace_back(FileFinder::MakePath("/usr/share/soundfonts", sf_file));
		sf_paths.emplace_back(FileFinder::MakePath("/usr/share/sounds/sf2", sf_file));
	}
#endif

	bool sf_load_success = false;
	for (const auto& sf_name: sf_paths) {
		if (fluid_synth_sfload(syn, sf_name.c_str(), 1) != FLUID_FAILED) {
			sf_load_success = true;
			status_message = fmt::format("Using soundfont {}", sf_name);
			break;
		}
	}

	if (!sf_load_success) {
		status_message = "Could not load soundfont.";
		return false;
	}

	return true;
}

static fluid_synth_t* create_synth(std::string& status_message) {
	fluid_synth_t* syn = new_fluid_synth(global_settings.get());
	if (!syn) {
		status_message = "new_fluid_synth failed";
		return nullptr;
	}

#if defined(HAVE_FLUIDSYNTH) && FLUIDSYNTH_VERSION_MAJOR > 1
	// Fluidsynth 1.x does not support VIO API for soundfonts
	// owned by fluid_synth
	auto* loader = new_fluid_defsfloader(global_settings.get());
	fluid_sfloader_set_callbacks(loader,
			vio_open, vio_read, vio_seek, vio_tell, vio_close);
	fluid_synth_add_sfloader(syn, loader);
#endif

	fluid_synth_set_interp_method(syn, -1, FLUID_INTERP_LINEAR);

	return syn;
}

FluidSynthDecoder::FluidSynthDecoder() {
	++instances;

	// Optimisation: Only create the soundfont once and share the synth
	// Sharing is only not possible when a Midi is played as a SE (unlikely)
	if (instances > 1) {
		std::string error_message;
		local_synth = create_synth(error_message);
		if (!local_synth) {
			// unlikely, the SF was already allocated once
			Output::Debug("FluidSynth failed: {}", error_message);
		}
	} else {
		use_global_synth = true;
		fluid_synth_program_reset(global_synth.get());
	}
}

FluidSynthDecoder::~FluidSynthDecoder() {
	--instances;
	assert(instances >= 0);

	if (use_global_synth) {
		// Exhaust the internal synth buffer
		// Prevents that old samples play when a new Midi song starts (even when there was a longer break between them)
		std::array<uint8_t, 64 * 4> buffer;
		fluid_synth_write_s16(global_synth.get(), buffer.size() / 4, buffer.data(), 0, 2, buffer.data(), 1, 2);
	} else {
		delete_fluid_synth(local_synth);
	}
}

bool FluidSynthDecoder::Initialize(std::string& status_message) {
	// only initialize once until a new game starts
	if (once) {
		if (!init && global_settings && !global_synth) {
			global_synth.reset(create_synth(status_message));
			if (global_synth) {
				if (!load_default_sf(status_message, global_synth.get())) {
					global_synth.reset();
				}
			}

			init = (global_synth != nullptr);
		}
		return init;
	}

	once = true;

#ifdef HAVE_FLUIDLITE
	fluid_set_default_fileapi(&fluidlite_vio);
#endif

	global_settings.reset(new_fluid_settings());
	if (!global_settings) {
		return false;
	}
	fluid_settings_setstr(global_settings.get(), "player.timing-source", "sample");
	fluid_settings_setint(global_settings.get(), "synth.lock-memory", 0);

	fluid_settings_setnum(global_settings.get(), "synth.gain", 0.6);
	fluid_settings_setnum(global_settings.get(), "synth.sample-rate", EP_MIDI_FREQ);
	fluid_settings_setint(global_settings.get(), "synth.polyphony", 256);

#if defined(HAVE_FLUIDSYNTH) && FLUIDSYNTH_VERSION_MAJOR > 1
	fluid_settings_setint(global_settings.get(), "synth.reverb.active", 0);
	fluid_settings_setint(global_settings.get(), "synth.chorus.active", 0);
#else
	fluid_settings_setstr(global_settings.get(), "synth.reverb.active", "no");
	fluid_settings_setstr(global_settings.get(), "synth.chorus.active", "no");
#endif

	global_synth.reset(create_synth(status_message));
	if (!global_synth) {
		return false;
	}

	if (!load_default_sf(status_message, global_synth.get())) {
		global_synth.reset();
		return false;
	}

	init = true;

	return init;
}

void FluidSynthDecoder::ResetState() {
	once = false;
	init = false;

	global_synth.reset();
	global_settings.reset();
	pending_global_synth.reset();
}

bool FluidSynthDecoder::ChangeGlobalSoundfont(StringView sf_path, std::string& status_message) {
	if (!global_synth) {
		return false;
	}

	pending_global_synth.reset(create_synth(status_message));

	if (!pending_global_synth) {
		return false;
	}

	if (sf_path.empty()) {
		return load_default_sf(status_message, pending_global_synth.get());
	}

	if (fluid_synth_sfload(pending_global_synth.get(), ToString(sf_path).c_str(), 1) != FLUID_FAILED) {
		status_message = fmt::format("Using soundfont {}", sf_path);
		return true;
	}

	pending_global_synth.reset();
	status_message = "Could not load soundfont.";
	return false;
}

int FluidSynthDecoder::FillBuffer(uint8_t* buffer, int length) {
	auto* instance_synth = GetSynthInstance();

	if (!instance_synth) {
		return -1;
	}

	if (fluid_synth_write_s16(instance_synth, length / 4, buffer, 0, 2, buffer, 1, 2) == FLUID_FAILED) {
		return -1;
	}

	return length;
}

void FluidSynthDecoder::SendMidiMessage(uint32_t message) {
	auto* instance_synth = GetSynthInstance();

	if (!instance_synth) {
		return;
	}

	unsigned int event = message & 0xF0;
	int channel = message & 0x0F;
	int param1 = (message >> 8) & 0x7F;
	int param2 = (message >> 16) & 0x7F;

	switch (event) {
		case MidiEvent_NoteOff:
			fluid_synth_noteoff(instance_synth, channel, param1);
			break;
		case MidiEvent_NoteOn:
			fluid_synth_noteon(instance_synth, channel, param1, param2);
			break;
		case MidiEvent_KeyPressure:
#if defined(HAVE_FLUIDSYNTH) && FLUIDSYNTH_VERSION_MAJOR == 1
			// unsupported
			return;
#else
			fluid_synth_key_pressure(instance_synth, channel, param1, param2);
#endif
			break;
		case MidiEvent_Controller:
			fluid_synth_cc(instance_synth, channel, param1, param2);
			break;
		case MidiEvent_ProgramChange:
			fluid_synth_program_change(instance_synth, channel, param1);
			break;
		case MidiEvent_ChannelPressure:
			fluid_synth_channel_pressure(instance_synth, channel, param1);
			break;
		case MidiEvent_PitchBend:
			fluid_synth_pitch_bend(instance_synth, channel, ((param2 & 0x7F) << 7) | (param1 & 0x7F));
			break;
		default:
			break;
	}
}

void FluidSynthDecoder::SendSysExMessage(const uint8_t* data, std::size_t size) {
	// FmMidi has F0 and F7 in SysEx, Fluidsynth wants them removed
	if (size <= 2) {
		return;
	}

	auto* instance_synth = GetSynthInstance();

	fluid_synth_sysex(instance_synth, reinterpret_cast<const char*>(data + 1), static_cast<int>(size - 2),
		nullptr, nullptr, nullptr, 0);
}

fluid_synth_t *FluidSynthDecoder::GetSynthInstance() {
	if (use_global_synth) {
		return global_synth.get();
	} else {
		return local_synth;
	}
}

void FluidSynthDecoder::OnNewMidi() {
	if (pending_global_synth) {
		global_synth = std::move(pending_global_synth);
	}
}

bool FluidSynthDecoder::NeedsSoftReset() {
	return true;
}

#endif
