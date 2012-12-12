/////////////////////////////////////////////////////////////////////////////
// This file is part of EasyRPG Player.
//
// EasyRPG Player is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// EasyRPG Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <cassert>

#include "system.h"
#include "al_audio.h"
#include "filefinder.h"
#include "output.h"
#include "sndfile.h"

char const ALAudio::WAVE_OUTPUT_DEVICE[] = "Wave File Writer";
char const ALAudio::NULL_DEVICE[] = "No Output";

struct ALAudio::source {
	source(EASYRPG_SHARED_PTR<ALCcontext> const& c, ALuint s)
		: ctx_(c), src_(s), loop_count_(0), fade_milli_(0)
		, volume_(1.0f), is_fade_in_(false) {}

	~source() {
		alcMakeContextCurrent(ctx_.get());
		alSourcePlay(src_);
		alDeleteSources(1, &src_);
		assert(alGetError() == ALC_NO_ERROR);
	}

	ALuint get() { return src_; }

private:
	EASYRPG_SHARED_PTR<ALCcontext> ctx_;
	ALuint src_;
	unsigned loop_count_, fade_milli_;
	ALfloat volume_;
	bool is_fade_in_;
	EASYRPG_SHARED_PTR<buffer> buf_;

	unsigned progress_milli() const { return(1000 * loop_count_ / 60); }
	bool fade_ended() const { return(fade_milli_ < progress_milli()); }
	float current_volume() const {
		return
			(fade_milli_ == 0)? 1.0f:
			(is_fade_in_
			 ? float(progress_milli()) / float(fade_milli_)
			 : float(fade_milli_ - progress_milli()) / float(fade_milli_)
			 ) * volume_;
	}

public:
	void set_volume(ALfloat const vol) {
		alcMakeContextCurrent(ctx_.get());
		volume_ = vol;
		alSourcef(src_, AL_GAIN, vol);
	}

	void fade_out(unsigned const ms) {
		loop_count_ = 0;
		fade_milli_ = ms;
		is_fade_in_ = false;
	}

	void fade_in(unsigned const ms) {
		loop_count_ = 0;
		fade_milli_ = ms;
		is_fade_in_ = true;
	}

	void play_buffer(EASYRPG_SHARED_PTR<buffer> const& buf);

	void update() {
		if(fade_milli_ != 0){
			alcMakeContextCurrent(ctx_.get());
			loop_count_++;

			if(fade_ended()) {
				alSourceStop(src_);
			} else {
				alSourcef(src_, AL_GAIN, current_volume());
			}
		}
	}
};

struct ALAudio::buffer {
	buffer(EASYRPG_SHARED_PTR<ALCcontext> const& c, ALuint b)
		: ctx_(c), buf_(b) {}

	~buffer() {
		alcMakeContextCurrent(ctx_.get());
		alDeleteBuffers(1, &buf_);
		assert(alGetError() == ALC_NO_ERROR);
	}

	ALuint get() { return buf_; }

private:
	EASYRPG_SHARED_PTR<ALCcontext> ctx_;
	ALuint buf_;
};

void ALAudio::source::play_buffer(EASYRPG_SHARED_PTR<buffer> const& buf) {
	alcMakeContextCurrent(ctx_.get());
	buf_ = buf;
	alSourceStop(src_);
	alSourcei(src_, AL_BUFFER, buf->get());
	alSourcei(src_, AL_SAMPLE_OFFSET, 0);
	alSourcePlay(src_);
}

bool ALAudio::load_sndfile(std::string const& filename, EASYRPG_SHARED_PTR<buffer> const& buf) {
	SF_INFO info;
	EASYRPG_SHARED_PTR<SNDFILE> file(sf_open(filename.c_str(), SFM_READ, &info), sf_close);
	if(! file) { return false; }

	// load data
	std::vector<int16_t> data;
	EASYRPG_ARRAY<int16_t, 4096> read_buf;
	size_t read_size = 0;
	while((read_size = sf_read_short(file.get(), read_buf.data(), read_buf.size())) != 0) {
		data.insert(data.end(), read_buf.begin(), read_buf.begin() + read_size);
	}

	// channel check
	ALsizei const channels =
		(info.channels == 1)? AL_FORMAT_MONO16:
		(info.channels == 2)? AL_FORMAT_STEREO16:
		AL_INVALID_VALUE;
	if(channels == AL_INVALID_VALUE) { return false; }

	alBufferData(buf->get(), channels, &data.front(),
				 data.size() * sizeof(int16_t), info.samplerate);

	return true;
}

EASYRPG_SHARED_PTR<ALAudio::buffer> ALAudio::load_buffer(std::string const& filename) {
	alcMakeContextCurrent(ctx_.get());

	EASYRPG_SHARED_PTR<buffer> buf = create_buffer();

	if(! filename.empty() && load_sndfile(filename, buf)) { return buf; }

	Output::Debug("Failed loading audio file: ", filename.c_str());

	return buf;
}

EASYRPG_SHARED_PTR<ALAudio::buffer> ALAudio::getMusic(std::string const& file) {
	buffer_cache::iterator i = music_pool_.find(file);
	if(i == music_pool_.end() || i->second.expired()) {
		EASYRPG_SHARED_PTR<buffer> const ret = load_buffer(FileFinder::FindMusic(file));
		music_pool_[file] = ret;
		return ret;
	} else {
		return i->second.lock();
	}
}

EASYRPG_SHARED_PTR<ALAudio::buffer> ALAudio::getSound(std::string const& file) {
	buffer_cache::iterator i = sound_pool_.find(file);
	if(i == sound_pool_.end() || i->second.expired()) {
		EASYRPG_SHARED_PTR<buffer> const ret = load_buffer(FileFinder::FindSound(file));
		sound_pool_[file] = ret;
		return ret;
	} else {
		return i->second.lock();
	}
}

void ALAudio::Update() {
	bgm_src_->update();
	bgs_src_->update();
	me_src_->update();

	for(source_list::iterator i = se_src_.begin(); i < se_src_.end(); ++i) {
		i->get()->update();

		ALenum state = AL_INVALID_VALUE;
		alGetSourcei(i->get()->get(), AL_SOURCE_STATE, &state);
		if(state == AL_STOPPED) {
			i = se_src_.erase(i);
		}
	}
}

////////////////////////////////////////////////////////////
ALAudio::ALAudio(char const* const dev_name)
{
	dev_.reset(alcOpenDevice(dev_name), &alcCloseDevice);
	assert(dev_);

	ctx_.reset(alcCreateContext(dev_.get(), NULL), &alcDestroyContext);
	assert(ctx_);

	alcMakeContextCurrent(ctx_.get());

	bgm_src_ = create_source();
}


EASYRPG_SHARED_PTR<ALAudio::source> ALAudio::create_source()
{
	alcMakeContextCurrent(ctx_.get());

	ALuint ret = AL_INVALID_VALUE;
	alGenSources(1, &ret);
	assert(ret != AL_INVALID_VALUE);
	return EASYRPG_MAKE_SHARED<source>(ctx_, ret);
}

EASYRPG_SHARED_PTR<ALAudio::buffer> ALAudio::create_buffer()
{
	alcMakeContextCurrent(ctx_.get());

	ALuint ret = AL_INVALID_VALUE;
	alGenBuffers(1, &ret);
	assert(ret != AL_INVALID_VALUE);
	return EASYRPG_MAKE_SHARED<buffer>(ctx_, ret);
}

////////////////////////////////////////////////////////////
void ALAudio::BGM_Play(std::string const& file, int volume, int pitch)
{
	alcMakeContextCurrent(ctx_.get());

	alSourcei(bgm_src_->get(), AL_PITCH, pitch * 0.01f);
	alSourcei(bgm_src_->get(), AL_LOOPING, AL_TRUE);
	bgm_src_->set_volume(volume * 0.01f);
	bgm_src_->play_buffer(getMusic(file));
}

////////////////////////////////////////////////////////////
void ALAudio::BGM_Stop()
{
	alcMakeContextCurrent(ctx_.get());
	alSourceStop(bgm_src_->get());
}

////////////////////////////////////////////////////////////
void ALAudio::BGM_Fade(int fade)
{
	alcMakeContextCurrent(ctx_.get());
	bgm_src_->fade_out(fade);
}

////////////////////////////////////////////////////////////
void ALAudio::BGM_Pause()
{
	alcMakeContextCurrent(ctx_.get());
	alSourcePause(bgm_src_->get());
}

////////////////////////////////////////////////////////////
void ALAudio::BGM_Resume()
{
	alcMakeContextCurrent(ctx_.get());
	alSourcePlay(bgm_src_->get());
}

////////////////////////////////////////////////////////////
void ALAudio::BGS_Play(std::string const& file, int volume, int pitch)
{
	alcMakeContextCurrent(ctx_.get());

	alSourcei(bgs_src_->get(), AL_PITCH, pitch * 0.01f);
	alSourcei(bgs_src_->get(), AL_LOOPING, AL_TRUE);
	bgm_src_->set_volume(volume * 0.01f);
	bgm_src_->play_buffer(getSound(file));
}

////////////////////////////////////////////////////////////
void ALAudio::BGS_Stop()
{
	alcMakeContextCurrent(ctx_.get());
	alSourceStop(bgs_src_->get());
}

////////////////////////////////////////////////////////////
void ALAudio::BGS_Fade(int fade)
{
	alcMakeContextCurrent(ctx_.get());
	bgs_src_->fade_out(fade);
}

////////////////////////////////////////////////////////////
void ALAudio::ME_Play(std::string const& file, int volume, int pitch)
{
	alcMakeContextCurrent(ctx_.get());

	alSourcei(me_src_->get(), AL_PITCH, pitch * 0.01f);
	alSourcei(me_src_->get(), AL_LOOPING, AL_FALSE);
	me_src_->set_volume(volume * 0.01f);
	me_src_->play_buffer(getMusic(file));
}

////////////////////////////////////////////////////////////
void ALAudio::ME_Stop()
{
	alcMakeContextCurrent(ctx_.get());
	alSourceStop(me_src_->get());
}

////////////////////////////////////////////////////////////
void ALAudio::ME_Fade(int fade)
{
	alcMakeContextCurrent(ctx_.get());
	me_src_->fade_out(fade);
}

////////////////////////////////////////////////////////////
void ALAudio::SE_Play(std::string const& file, int volume, int pitch)
{
	alcMakeContextCurrent(ctx_.get());

	EASYRPG_SHARED_PTR<source> src = create_source();

	alSourcei(src->get(), AL_PITCH, pitch * 0.01f);
	alSourcei(src->get(), AL_LOOPING, AL_FALSE);
	src->set_volume(volume * 0.01f);
	src->play_buffer(getSound(file));

	se_src_.push_back(src);
}

////////////////////////////////////////////////////////////
void ALAudio::SE_Stop()
{
	alcMakeContextCurrent(ctx_.get());
	for(source_list::iterator i = se_src_.begin(); i < se_src_.end(); ++i) {
		alSourceStop(i->get()->get());
	}
	se_src_.clear();
}
