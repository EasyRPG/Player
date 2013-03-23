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
#include <boost/assert.hpp>

#include "system.h"
#include "al_audio.h"
#include "filefinder.h"
#include "output.h"
#include "sndfile.h"

char const ALAudio::WAVE_OUTPUT_DEVICE[] = "Wave File Writer";
char const ALAudio::NULL_DEVICE[] = "No Output";


namespace {
	/**	
	 * @return true if no error
	 */
	bool print_al_error() {
		char const* err = NULL;
		ALenum const err_code = alGetError();
		switch(err_code) {
		case AL_INVALID_NAME: err = "invalid name"; break;
		case AL_INVALID_ENUM: err = "invalid enum"; break;
		case AL_INVALID_VALUE: err = "invalid value"; break;
		case AL_INVALID_OPERATION: err = "invalid operation"; break;
		case AL_OUT_OF_MEMORY: err = "out of memory"; break;
		default:
			err = "unkown error";
			Output::Debug("unkown error code %x", err_code);
			break;

		case AL_NO_ERROR: return true;
		}

		Output::Debug("AL error: %s", err);
		return false;
	}

	bool print_alc_error(ALCdevice* const dev) {
		char const* err = NULL;
		ALenum const err_code = alcGetError(dev);
		switch(err_code) {
		case ALC_INVALID_DEVICE: err = "invalid device"; break;
		case ALC_INVALID_ENUM: err = "invalid enum"; break;
		case ALC_INVALID_CONTEXT: err = "invalid context"; break;
		case ALC_OUT_OF_MEMORY: err = "out of memory"; break;
		default:
			err = "unkown error";
			Output::Debug("unkown error code %x", err_code);
			break;

		case ALC_NO_ERROR: return true;
		}

		Output::Debug("ALC error: %s", err);
		return false;
	}

	struct set_context {
		set_context(EASYRPG_SHARED_PTR<ALCcontext> const& ctx)
			: prev_ctx(alcGetCurrentContext())
		{
			BOOST_ASSERT(ctx);
			BOOST_ASSERT(prev_ctx);

			if(ctx.get() != prev_ctx) {
				alcMakeContextCurrent(ctx.get());
				BOOST_ASSERT(print_alc_error
							 (alcGetContextsDevice(alcGetCurrentContext())));
			}

			BOOST_ASSERT(print_al_error());
		}
		~set_context() {
			BOOST_ASSERT(print_al_error());

			if(alcGetCurrentContext() != prev_ctx) {
				alcMakeContextCurrent(prev_ctx);
				BOOST_ASSERT(print_alc_error
							 (alcGetContextsDevice(alcGetCurrentContext())));
			}
		}

	private:
		ALCcontext* const prev_ctx;
	};

#define SET_CONTEXT(ctx) \
	set_context const c_ ## __LINE__ (ctx); \
	(void) c_ ## __LINE__ ; \

}

struct ALAudio::source {
	source(EASYRPG_SHARED_PTR<ALCcontext> const& c, ALuint const s)
		: ctx_(c), src_(s), loop_count_(0), fade_milli_(0)
		, volume_(1.0f), is_fade_in_(false)
	{
		SET_CONTEXT(c);
		BOOST_ASSERT(alIsSource(s) == AL_TRUE);
	}

	~source() {
		SET_CONTEXT(ctx_);
		alSourcePlay(src_);
		alDeleteSources(1, &src_);
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
		SET_CONTEXT(ctx_);
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
			SET_CONTEXT(ctx_);
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
	buffer(EASYRPG_SHARED_PTR<ALCcontext> const& c, ALuint const b)
		: ctx_(c), buf_(b)
	{
		SET_CONTEXT(c);
		BOOST_ASSERT(alIsBuffer(b) == AL_TRUE);
	}

	~buffer() {
		SET_CONTEXT(ctx_);
		alDeleteBuffers(1, &buf_);
	}

	ALuint get() { return buf_; }

private:
	EASYRPG_SHARED_PTR<ALCcontext> ctx_;
	ALuint buf_;
};

void ALAudio::source::play_buffer(EASYRPG_SHARED_PTR<buffer> const& buf) {
	SET_CONTEXT(ctx_);

	buf_ = buf;

	ALenum state = AL_INVALID_VALUE;
	alGetSourcei(src_, AL_SOURCE_STATE, &state);
	if(state != AL_STOPPED) {
		alSourceStop(src_);
	}

	if(buf_) {
		alSourcei(src_, AL_BUFFER, buf->get());
		alSourcei(src_, AL_SAMPLE_OFFSET, 0);
		alSourcePlay(src_);
	} else {
		alSourcei(src_, AL_BUFFER, AL_NONE);
	}
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
	SET_CONTEXT(ctx_);

	EASYRPG_SHARED_PTR<buffer> const buf = create_buffer();

	if(! filename.empty() && load_sndfile(filename, buf)) { return buf; }

	Output::Debug("Failed loading audio file: ", filename.c_str());

	return EASYRPG_SHARED_PTR<buffer>();
}

EASYRPG_SHARED_PTR<ALAudio::buffer> ALAudio::getMusic(std::string const& file) {
	buffer_cache::iterator const i = music_pool_.find(file);
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

ALAudio::ALAudio(char const* const dev_name) {
	dev_.reset(alcOpenDevice(dev_name), &alcCloseDevice);
	BOOST_ASSERT(dev_);

	ctx_.reset(alcCreateContext(dev_.get(), NULL), &alcDestroyContext);
	BOOST_ASSERT(ctx_);

	alcMakeContextCurrent(ctx_.get());

	SET_CONTEXT(ctx_);

	bgm_src_ = create_source();
	bgs_src_ = create_source();
	me_src_ = create_source();
}


EASYRPG_SHARED_PTR<ALAudio::source> ALAudio::create_source() {
	SET_CONTEXT(ctx_);

	ALuint ret = AL_NONE;
	alGenSources(1, &ret);
	print_al_error();
	BOOST_ASSERT(ret != AL_NONE);

	return EASYRPG_MAKE_SHARED<source>(ctx_, ret);
}

EASYRPG_SHARED_PTR<ALAudio::buffer> ALAudio::create_buffer() {
	SET_CONTEXT(ctx_);

	ALuint ret = AL_NONE;
	alGenBuffers(1, &ret);
	BOOST_ASSERT(ret != AL_NONE);

	return EASYRPG_MAKE_SHARED<buffer>(ctx_, ret);
}

void ALAudio::BGM_Play(std::string const& file, int volume, int pitch) {
	SET_CONTEXT(ctx_);

	alSourcef(bgm_src_->get(), AL_PITCH, pitch * 0.01f);
	alSourcei(bgm_src_->get(), AL_LOOPING, AL_TRUE);
	bgm_src_->set_volume(volume * 0.01f);
	bgm_src_->play_buffer(getMusic(file));
}

void ALAudio::BGM_Stop() {
	SET_CONTEXT(ctx_);
	alSourceStop(bgm_src_->get());
}

void ALAudio::BGM_Fade(int fade) {
	SET_CONTEXT(ctx_);
	bgm_src_->fade_out(fade);
}

void ALAudio::BGM_Pause() {
	SET_CONTEXT(ctx_);
	alSourcePause(bgm_src_->get());
}

void ALAudio::BGM_Resume() {
	SET_CONTEXT(ctx_);
	alSourcePlay(bgm_src_->get());
}

void ALAudio::BGS_Play(std::string const& file, int volume, int pitch) {
	SET_CONTEXT(ctx_);

	alSourcef(bgs_src_->get(), AL_PITCH, pitch * 0.01f);
	alSourcei(bgs_src_->get(), AL_LOOPING, AL_TRUE);
	bgm_src_->set_volume(volume * 0.01f);
	bgm_src_->play_buffer(getSound(file));
}

void ALAudio::BGS_Stop() {
	SET_CONTEXT(ctx_);
	alSourceStop(bgs_src_->get());
}

void ALAudio::BGS_Fade(int fade) {
	SET_CONTEXT(ctx_);
	bgs_src_->fade_out(fade);
}

void ALAudio::ME_Play(std::string const& file, int volume, int pitch) {
	SET_CONTEXT(ctx_);

	alSourcef(me_src_->get(), AL_PITCH, pitch * 0.01f);
	alSourcei(me_src_->get(), AL_LOOPING, AL_FALSE);
	me_src_->set_volume(volume * 0.01f);
	me_src_->play_buffer(getMusic(file));
}

void ALAudio::ME_Stop() {
	SET_CONTEXT(ctx_);
	alSourceStop(me_src_->get());
}

void ALAudio::ME_Fade(int fade) {
	SET_CONTEXT(ctx_);
	me_src_->fade_out(fade);
}

void ALAudio::SE_Play(std::string const& file, int volume, int pitch) {
	SET_CONTEXT(ctx_);

	EASYRPG_SHARED_PTR<source> src = create_source();

	alSourcef(src->get(), AL_PITCH, pitch * 0.01f);
	alSourcei(src->get(), AL_LOOPING, AL_FALSE);
	src->set_volume(volume * 0.01f);
	src->play_buffer(getSound(file));

	se_src_.push_back(src);
}

void ALAudio::SE_Stop() {
	SET_CONTEXT(ctx_);
	for(source_list::iterator i = se_src_.begin(); i < se_src_.end(); ++i) {
		alSourceStop(i->get()->get());
	}
	se_src_.clear();
}
