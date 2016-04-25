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
		U16
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

	int Decode(uint8_t* buffer, int length);

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
};

#endif
