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

#ifndef _EASYRPG_AUDIO_DECODER_H_
#define _EASYRPG_AUDIO_DECODER_H_

// Headers
#include <string>
#include <vector>
#include <memory>

/**
 * Audio Decoder Namespace
 */
class AudioDecoder {
public:
	enum class Format {
		S8,
		U8,
		S16,
		U16,
		S32,
		U32,
		F32
	};

	enum class Channel {
		Mono = 1,
		Stereo = 2
	};

	enum class Origin {
		Begin = 0,
		Current = 1,
		End = 2
	};

	/**
	 * Writes 'size' bytes in the specified buffer. The data matches the format
	 * reported by GetFormat.
	 * This function will not clear the buffer before (partially) filling it.
	 *
	 * @param buffer Output buffer
	 * @param size Size of the buffer
	 * @return Number of bytes written to the buffer or -1 on error
	 */
	int Decode(uint8_t* buffer, int size);

	/**
	 * Splits stereo into mono and Writes 'size' bytes in each of the buffers.
	 * The data matches the format reported by GetFormat, except that both
	 * buffers will contain Mono audio. When the source format was already mono
	 * the 'right' buffer is not filled.
	 *
	 * @param left Output buffer of the left channel
	 * @param right Output buffer of the right channel (or nothing if source is mono)
	 * @param size Size of both buffers.
	 * @return Number of bytes written in one of the buffers or -1 on error
	 */
	int DecodeAsMono(uint8_t* left, uint8_t* right, int size);

	static std::unique_ptr<AudioDecoder> Create(FILE** file, const std::string& filename);

	void Update(int delta);

	void SetFade(int begin, int end, int duration);
	void SetVolume(int volume);
	int GetVolume() const;

	void Pause();
	void Resume();
	bool Rewind();

	bool GetLooping() const;
	void SetLooping(bool enable);
	int GetLoopCount() const;

	virtual bool Open(FILE* file) = 0;

	virtual bool IsFinished() const = 0;

	virtual std::string GetError() const = 0;

	virtual void GetFormat(int& frequency, Format& format, Channel& channels) const = 0;
	virtual bool SetFormat(int frequency, Format format, Channel channels);

	virtual int GetPitch() const;
	virtual bool SetPitch(int pitch);

	virtual bool Seek(size_t offset, Origin origin);
	virtual size_t Tell();

	virtual int GetTicks();

	static int GetSamplesizeForFormat(AudioDecoder::Format format);
protected:
	virtual int FillBuffer(uint8_t* buffer, int length) = 0;
	int loop_count = 0;

private:
	bool paused = false;
	double volume = 0;
	double fade_end = 0;
	double fade_time = -1;
	double delta_step = 0;

	bool looping = false;
	int loops = 0;

	std::vector<uint8_t> mono_buffer;
};

#endif
