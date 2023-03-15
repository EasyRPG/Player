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
} works;

std::unique_ptr<AudioDecoderBase> MidiDecoder::Create(bool resample) {
	std::unique_ptr<AudioDecoderBase> mididec;

	mididec = CreateFluidsynth(resample);
	if (!mididec) {
		mididec = CreateWildMidi(resample);
		if (!mididec) {
			mididec = CreateFmMidi(resample);
		}
	}

	return mididec;
}

std::unique_ptr<AudioDecoderBase> MidiDecoder::CreateFluidsynth(bool resample) {
	std::unique_ptr<AudioDecoderBase> mididec;

#if defined(HAVE_FLUIDSYNTH) || defined(HAVE_FLUIDLITE)
	std::string error_message;
	if (works.fluidsynth && FluidSynthDecoder::Initialize(error_message)) {
		auto dec = std::make_unique<FluidSynthDecoder>();
		mididec = std::make_unique<AudioDecoderMidi>(std::move(dec));
	}
	else if (!mididec && works.fluidsynth) {
		Output::Debug("{}", error_message);
		works.fluidsynth = false;
	}
#endif

#ifdef USE_AUDIO_RESAMPLER
	if (mididec && resample) {
		mididec = std::make_unique<AudioResampler>(std::move(mididec));
	}
#endif

	return mididec;
}

std::unique_ptr<AudioDecoderBase> MidiDecoder::CreateWildMidi(bool resample) {
	std::unique_ptr<AudioDecoderBase> mididec;

#ifdef HAVE_LIBWILDMIDI
	std::string error_message;
	if (!mididec && works.wildmidi && WildMidiDecoder::Initialize(error_message)) {
		auto dec = std::make_unique<WildMidiDecoder>();
		mididec = std::make_unique<AudioDecoderMidi>(std::move(dec));
	}
	else if (!mididec && works.wildmidi) {
		Output::Debug("{}", error_message);
		works.wildmidi = false;
	}
#endif

#ifdef USE_AUDIO_RESAMPLER
	if (mididec && resample) {
		mididec = std::make_unique<AudioResampler>(std::move(mididec));
	}
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
#endif

	return mididec;
}
