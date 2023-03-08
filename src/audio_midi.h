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

#ifndef EP_AUDIO_MIDI_H
#define EP_AUDIO_MIDI_H

class AudioDecoderMidi;

#include "audio_decoder_base.h"

#if defined(GEKKO) || defined(__3DS__)
#  define EP_MIDI_FREQ 22050
#else
#  define EP_MIDI_FREQ 44100
#endif

/**
 * The MidiDecoder is an abstraction over Midi libraries.
 *
 * Create will instantiate a proper Midi decoder and calling Decode will
 * fill a buffer with audio data which must be passed to the audio hardware.
 */
class MidiDecoder {
public:
	enum MidiEvent {
		MidiEvent_NoteOff = 0x80,
		MidiEvent_NoteOn = 0x90,
		MidiEvent_KeyPressure = 0xA0,
		MidiEvent_Controller = 0xB0,
		MidiEvent_ProgramChange = 0xC0,
		MidiEvent_ChannelPressure = 0xD0,
		MidiEvent_PitchBend = 0xE0
	};

	MidiDecoder() = default;
	MidiDecoder(MidiDecoder&&) = delete;
	MidiDecoder(const MidiDecoder&) = delete;
	MidiDecoder& operator=(const MidiDecoder&) = delete;
	MidiDecoder& operator=(MidiDecoder&&) = delete;
	virtual ~MidiDecoder() = default;

	/*
	 * All libraries must implement:
	 * - FillBuffer
	 * - GetName
	 *
	 * The library must implement the following commands:
	 * - SendMidiMessage
	 * - SendSysExMessage (nice to have)
	 *
	 * When Midi messages are not supported (library uses own sequencer)
	 * - Open
	 * - Seek
	 * - Tell
	 * - SetPitch - Return "false" for resampling
	 * - SupportsMidiMessages - Must return "false"
	 */

	/**
	 * Passes a buffer containing a Midi file to the Midi Decoder
	 * @param data Midi Data
	 * @return true when the decoder supports this data
	 */
	virtual bool Open(std::vector<uint8_t>& data) {
		(void)data;
		return true;
	}

	/**
	 * Seeks in the midi stream. The value of offset is implementation
	 * defined.
	 *
	 * @param offset Offset to seek to
	 * @param origin Position to seek from
	 * @return Whether seek was successful
	 */
	virtual bool Seek(std::streamoff offset, std::ios_base::seekdir origin) {
		(void)offset;
		(void)origin;
		return true;
	};

	/**
	 * Sets the pitch multiplier.
	 * 100 = normal speed
	 * 200 = double speed and so on
	 * Not all audio decoders support this. Using the audio hardware is
	 * recommended.
	 *
	 * @param pitch Pitch multiplier to use
	 * @return true if pitch was set, false otherwise
	 */
	virtual bool SetPitch(int pitch) {
		(void)pitch;
		return true;
	}

	/**
	 * Retrieves the format of the Midi decoder.
	 * It is guaranteed that these settings will stay constant the whole time.
	 *
	 * @param frequency Filled with the audio frequency
	 * @param format Filled with the audio format
	 * @param channels Filled with the amount of channels
	 */
	virtual void GetFormat(int& freq, AudioDecoderBase::Format& format, int& channels) const;

	/**
	 * Requests a preferred format from the audio decoder. Not all decoders
	 * support everything and it's recommended to use the audio hardware
	 * for audio processing.
	 * When false is returned use GetFormat to get the real format of the
	 * output data.
	 *
	 * @param frequency Audio frequency
	 * @param format Audio format
	 * @param channels Number of channels
	 * @return true when all settings were set, otherwise false (use GetFormat)
	 */
	virtual bool SetFormat(int frequency, AudioDecoderBase::Format format, int channels);

	/**
	 * Forwards a Midi message to the Midi decoder
	 *
	 * @param message Midi message
	 */
	virtual void SendMidiMessage(uint32_t message) {
		(void)message;
	}

	/**
	 * Forwards a SysEx Message to the Midi Decoder
	 *
	 * @param data sysex content
	 * @param size length of sysex content (data)
	 */
	virtual void SendSysExMessage(const uint8_t* data, size_t size) {
		(void)data;
		(void)size;
	}

	/**
	 * Called when the synthesizer shall write data in a buffer.
	 *
	 * @param buffer Buffer to fill
	 * @param size Buffer size
	 * @return number of bytes read or -1 on error
	 */
	virtual int FillBuffer(uint8_t* buffer, int length) {
		(void)buffer;
		(void)length;
		return -1;
	};

	/**
	 * Returns the unique name of the Midi decoder.
	 *
	 * @return decoder name
	 */
	virtual std::string GetName() = 0;

	/**
	 * @return False if the Library is sequencer based
	*/
	virtual bool SupportsMidiMessages() {
		return true;
	}

	/**
	 * Attempts to initialize a Midi library for processing the Midi data.
	 *
	 * @param resample Whether the decoder shall be wrapped into a resampler (if supported)
	 * @return A Midi decoder instance when the Midi data is supported, otherwise null
	 */
	static std::unique_ptr<AudioDecoderBase> Create(bool resample);

	static std::unique_ptr<AudioDecoderBase> CreateFluidsynth(bool resample);

	static std::unique_ptr<AudioDecoderBase> CreateWildMidi(bool resample);

	static std::unique_ptr<AudioDecoderBase> CreateFmMidi(bool resample);

protected:
	int frequency = EP_MIDI_FREQ;
};

#endif
