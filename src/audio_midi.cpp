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
#include "audio_midi.h"
#include "audio_decoder_midi.h"
#include "audio.h"
#include "decoder_fluidsynth.h"
#include "decoder_fmmidi.h"
#include "decoder_wildmidi.h"
#include "output.h"

#ifdef USE_AUDIO_RESAMPLER
#include "audio_resampler.h"
#endif

void MidiDecoder::GetFormat(int& freq, AudioDecoderBase::Format& format, int& channels) const {
	freq = frequency;
	format = AudioDecoderBase::Format::S16;
	channels = 2;
}

bool MidiDecoder::SetFormat(int frequency, AudioDecoderBase::Format format, int channels) {
	if (frequency != EP_MIDI_FREQ || channels != 2 || format != AudioDecoderBase::Format::S16)
		return false;

	return true;
}

static struct {
	bool fluidsynth = true;
	bool wildmidi = true;
	std::string fluidsynth_status;
	std::string wildmidi_status;
} works;

std::unique_ptr<AudioDecoderBase> MidiDecoder::Create(bool resample) {
	std::unique_ptr<AudioDecoderBase> mididec;

	if (Audio().GetFluidsynthEnabled()) {
		mididec = CreateFluidsynth(resample);
	}

	if (!mididec && Audio().GetWildMidiEnabled()) {
		mididec = CreateWildMidi(resample);
	}

	if (!mididec) {
		mididec = CreateFmMidi(resample);
	}

	return mididec;
}

std::unique_ptr<AudioDecoderBase> MidiDecoder::CreateFluidsynth(bool resample) {
	std::unique_ptr<AudioDecoderBase> mididec;

#if defined(HAVE_FLUIDSYNTH) || defined(HAVE_FLUIDLITE)
	if (works.fluidsynth && FluidSynthDecoder::Initialize(works.fluidsynth_status)) {
		auto dec = std::make_unique<FluidSynthDecoder>();
		mididec = std::make_unique<AudioDecoderMidi>(std::move(dec));
	}
	else if (!mididec && works.fluidsynth) {
		Output::Debug("Fluidsynth: {}", works.fluidsynth_status);
		works.fluidsynth = false;
	}
#endif

#ifdef USE_AUDIO_RESAMPLER
	if (mididec && resample) {
		mididec = std::make_unique<AudioResampler>(std::move(mididec));
	}
#else
	(void)resample;
#endif

	return mididec;
}

std::unique_ptr<AudioDecoderBase> MidiDecoder::CreateWildMidi(bool resample) {
	std::unique_ptr<AudioDecoderBase> mididec;

#ifdef HAVE_LIBWILDMIDI
	if (!mididec && works.wildmidi && WildMidiDecoder::Initialize(works.wildmidi_status)) {
		auto dec = std::make_unique<WildMidiDecoder>();
		mididec = std::make_unique<AudioDecoderMidi>(std::move(dec));
	}
	else if (!mididec && works.wildmidi) {
		Output::Debug("WildMidi: {}", works.wildmidi_status);
		works.wildmidi = false;
	}
#endif

#ifdef USE_AUDIO_RESAMPLER
	if (mididec && resample) {
		mididec = std::make_unique<AudioResampler>(std::move(mididec));
	}
#else
	(void)resample;
#endif

	return mididec;
}

std::unique_ptr<AudioDecoderBase> MidiDecoder::CreateFmMidi(bool resample) {
	std::unique_ptr<AudioDecoderBase> mididec;

#if WANT_FMMIDI
	if (!mididec) {
		auto dec = std::make_unique<FmMidiDecoder>();
		mididec = std::make_unique<AudioDecoderMidi>(std::move(dec));
	}
#endif

#ifdef USE_AUDIO_RESAMPLER
	if (mididec && resample) {
		mididec = std::make_unique<AudioResampler>(std::move(mididec));
	}
#else
	(void)resample;
#endif

	return mididec;
}

bool MidiDecoder::CheckFluidsynth(std::string& status_message) {
	if (works.fluidsynth && works.fluidsynth_status.empty()) {
		CreateFluidsynth(true);
	}

	status_message = works.fluidsynth_status;
	return works.fluidsynth;
}

void MidiDecoder::ChangeFluidsynthSoundfont(StringView sf_path) {
	if (!works.fluidsynth || works.fluidsynth_status.empty()) {
		// Fluidsynth was not initialized yet or failed, will use the path from the config automatically
		works.fluidsynth = true;
		CreateFluidsynth(true);
		return;
	}

#if defined(HAVE_FLUIDSYNTH) || defined(HAVE_FLUIDLITE)
	// Was initialized before
	works.fluidsynth = FluidSynthDecoder::ChangeGlobalSoundfont(sf_path, works.fluidsynth_status);
	Output::Debug("Fluidsynth: {}", works.fluidsynth_status);
#else
	(void)sf_path;
#endif
}

bool MidiDecoder::CheckWildMidi(std::string &status_message) {
	if (works.wildmidi && works.wildmidi_status.empty()) {
		CreateWildMidi(true);
	}

	status_message = works.wildmidi_status;
	return works.wildmidi;
}

void MidiDecoder::Reset() {
	works.fluidsynth = true;
	works.wildmidi = true;

#ifdef HAVE_LIBWILDMIDI
	WildMidiDecoder::ResetState();
#endif

#if defined(HAVE_FLUIDSYNTH) || defined(HAVE_FLUIDLITE)
	FluidSynthDecoder::ResetState();
#endif
}
