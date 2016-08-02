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

#ifdef HAVE_OPENAL

#include <array>
#include <deque>
#include <cassert>
#include <functional>

#include "audio_al.h"
#include "filefinder.h"
#include "output.h"
#include "sndfile.h"

#include <fluidsynth.h>
#include <fluidsynth/midi.h>
#include <fluidsynth/seq.h>
#include <fluidsynth/seqbind.h>

char const ALAudio::WAVE_OUTPUT_DEVICE[] = "Wave File Writer";
char const ALAudio::NULL_DEVICE[] = "No Output";

namespace {
	/**
	 * @return true if no error
	 */
	bool print_al_error() {
		char const *err = NULL;
		ALenum const err_code = alGetError();
		switch (err_code) {
		case AL_INVALID_NAME:
			err = "invalid name";
			break;
		case AL_INVALID_ENUM:
			err = "invalid enum";
			break;
		case AL_INVALID_VALUE:
			err = "invalid value";
			break;
		case AL_INVALID_OPERATION:
			err = "invalid operation";
			break;
		case AL_OUT_OF_MEMORY:
			err = "out of memory";
			break;
		default:
			err = "unkown error";
			Output::Debug("unkown error code %x", err_code);
			break;

		case AL_NO_ERROR:
			return true;
		}

		Output::Debug("AL error: %s", err);
		return false;
	}

	bool print_alc_error(ALCdevice *const dev) {
		char const *err = NULL;
		ALenum const err_code = alcGetError(dev);
		switch (err_code) {
		case ALC_INVALID_DEVICE:
			err = "invalid device";
			break;
		case ALC_INVALID_ENUM:
			err = "invalid enum";
			break;
		case ALC_INVALID_CONTEXT:
			err = "invalid context";
			break;
		case ALC_OUT_OF_MEMORY:
			err = "out of memory";
			break;
		default:
			err = "unkown error";
			Output::Debug("unkown error code %x", err_code);
			break;

		case ALC_NO_ERROR:
			return true;
		}

		Output::Debug("ALC error: %s", err);
		return false;
	}

	struct set_context {
		set_context(std::shared_ptr<ALCcontext> const &ctx) : prev_ctx(alcGetCurrentContext()) {
			assert(ctx);
			assert(prev_ctx);

			if (ctx.get() != prev_ctx) {
				alcMakeContextCurrent(ctx.get());
				assert(print_alc_error(alcGetContextsDevice(alcGetCurrentContext())));
			}

			assert(print_al_error());
		}
		~set_context() {
			assert(print_al_error());

			if (alcGetCurrentContext() != prev_ctx) {
				alcMakeContextCurrent(prev_ctx);
				assert(print_alc_error(alcGetContextsDevice(alcGetCurrentContext())));
			}
		}

	private:
		ALCcontext *const prev_ctx;
	};

#define SET_CONTEXT(ctx)                 \
	set_context const c_##__LINE__(ctx); \
	(void) c_##__LINE__;

	enum { BUFFER_NUMBER = 3 };

	double const SECOND_PER_BUFFER = 0.5;
}

struct ALAudio::buffer_loader {
	virtual ~buffer_loader() {
	}

	virtual size_t load_buffer(ALuint buffer) = 0;
	virtual bool is_end() const = 0;
	virtual unsigned midi_ticks() const {
		return 0;
	}

	unsigned loop_count_ = 0;
};

struct ALAudio::source {
	source(std::shared_ptr<ALCcontext> const &c, ALuint const s, bool loop)
	    : ctx_(c)
	    , src_(s)
	    , fade_count_(0)
	    , fade_milli_(0)
	    , volume_(1.0f)
	    , is_fade_in_(false)
	    , loop_play_(loop)
	    , ticks_(BUFFER_NUMBER + 1, 0)
	    , buf_sizes_(BUFFER_NUMBER, 0) {
		SET_CONTEXT(c);
		assert(alIsSource(s) == AL_TRUE);

		alGenBuffers(BUFFER_NUMBER, buffers_.data());
	}

	void init_midi() {
		if (synth) {
			return;
		}

		settings.reset(new_fluid_settings(), &delete_fluid_settings);
		fluid_settings_setstr(settings.get(), "player.timing-source", "sample");
		fluid_settings_setint(settings.get(), "synth.lock-memory", 0);

		synth.reset(new_fluid_synth(settings.get()), &delete_fluid_synth);
		if (fluid_synth_sfload(synth.get(), getenv("DEFAULT_SOUNDFONT"), 1) == FLUID_FAILED)
			Output::Error("Couldn't load soundfont\n%s.", getenv("DEFAULT_SOUNDFONT"));

		double sample_rate = 0;
		fluid_settings_getnum(settings.get(), "synth.sample-rate", &sample_rate);
		assert(sample_rate != 0);
		this->sample_rate = sample_rate;

		seq.reset(new_fluid_sequencer2(false), &delete_fluid_sequencer);
		if (fluid_sequencer_register_fluidsynth(seq.get(), synth.get()) == FLUID_FAILED)
			Output::Error("Couldn't initialize MIDI playback.");
	}

	~source() {
		SET_CONTEXT(ctx_);
		alSourcePlay(src_);
		alDeleteSources(1, &src_);
	}

	int loop_count() {
		return loader_->loop_count_;
	}

	ALuint get() {
		return src_;
	}

	std::shared_ptr<fluid_settings_t> settings;
	std::shared_ptr<fluid_synth_t> synth;
	std::shared_ptr<fluid_player_t> player;
	std::shared_ptr<fluid_sequencer_t> seq;
	unsigned sample_rate;

private:
	std::shared_ptr<ALCcontext> ctx_;
	ALuint src_;
	unsigned fade_count_, fade_milli_;
	ALfloat volume_;
	bool is_fade_in_;
	bool loop_play_;
	std::array<ALuint, BUFFER_NUMBER> buffers_;
	std::shared_ptr<buffer_loader> loader_;
	std::deque<unsigned> ticks_, buf_sizes_;

	unsigned progress_milli() const {
		return (1000 * fade_count_ / 60);
	}
	bool fade_ended() const {
		return (fade_milli_ < progress_milli());
	}
	float current_volume() const {
		return (fade_milli_ == 0)
		           ? 1.0f
		           : (is_fade_in_ ? float(progress_milli()) / float(fade_milli_)
		                          : float(fade_milli_ - progress_milli()) / float(fade_milli_)) *
		                 volume_;
	}

public:
	void set_volume(ALfloat const vol) {
		SET_CONTEXT(ctx_);
		volume_ = vol;
		alSourcef(src_, AL_GAIN, vol);
	}

	void fade_out(unsigned const ms) {
		fade_count_ = 0;
		fade_milli_ = ms;
		is_fade_in_ = false;
	}

	void fade_in(unsigned const ms) {
		fade_count_ = 0;
		fade_milli_ = ms;
		is_fade_in_ = true;
	}

	void update() {
		ALint processed;
		alGetSourceiv(src_, AL_BUFFERS_PROCESSED, &processed);
		std::vector<ALuint> unqueued(processed);
		alSourceUnqueueBuffers(src_, processed, &unqueued.front());
		int queuing_count = 0;
		for (; queuing_count < processed; ++queuing_count) {
			if (!loop_play_ && loader_->is_end()) {
				loader_.reset();
				++queuing_count;
				break;
			}

			if (loader_->is_end()) {
				ticks_.pop_front();
				ticks_.push_back(0);
			}
			buf_sizes_.pop_front();
			buf_sizes_.push_back(loader_->load_buffer(unqueued[queuing_count]));
			ticks_.pop_front();
			ticks_.push_back(loader_->midi_ticks());
		}
		alSourceQueueBuffers(src_, queuing_count, &unqueued.front());

		if (fade_milli_ != 0) {
			SET_CONTEXT(ctx_);
			fade_count_++;

			if (fade_ended()) {
				fade_milli_ = 0;
				if (!is_fade_in_)
					alSourceStop(src_);
			} else {
				alSourcef(src_, AL_GAIN, current_volume());
			}
		}
	}

	void set_buffer_loader(std::shared_ptr<buffer_loader> const &l) {
		SET_CONTEXT(ctx_);
		alSourceStop(src_);
		alSourcei(src_, AL_BUFFER, AL_NONE);

		if (!l) {
			loader_.reset();
			return;
		}

		ALint unqueuing_count;
		alGetSourceiv(src_, AL_BUFFERS_QUEUED, &unqueuing_count);
		std::vector<ALuint> unqueued(unqueuing_count);
		alSourceUnqueueBuffers(src_, unqueuing_count, &unqueued.front());

		loader_ = l;
		int queuing_count = 0;
		assert(!l->is_end());
		ticks_[0] = 0;
		for (; queuing_count < BUFFER_NUMBER; ++queuing_count) {
			buf_sizes_[queuing_count] = loader_->load_buffer(buffers_[queuing_count]);
			ticks_[queuing_count + 1] = loader_->midi_ticks();

			if (loader_->is_end()) {
				queuing_count++;
				break;
			}
		}
		alSourceQueueBuffers(src_, queuing_count, buffers_.data());
		alSourcePlay(src_);
	}

	unsigned midi_ticks() const {
		if (!loader_) {
			return 0;
		}

		ALint offset;
		alGetSourceiv(src_, AL_SAMPLE_OFFSET, &offset);

		return ticks_[0] + (ticks_[1] - ticks_[0]) * offset / buf_sizes_[0];
	}
};

struct ALAudio::sndfile_loader : public ALAudio::buffer_loader {
	static std::shared_ptr<buffer_loader> create(std::string const &filename) {
		SF_INFO info;
		std::shared_ptr<SNDFILE> f(sf_open(filename.c_str(), SFM_READ, &info), sf_close);
		if (!f) {
			return std::shared_ptr<buffer_loader>();
		}
		return std::make_shared<sndfile_loader>(filename, f, info);
	}

	sndfile_loader(std::string const &filename, std::shared_ptr<SNDFILE> f, SF_INFO const &info)
	    : filename_(filename)
	    , file_(f)
	    , info_(info)
	    , format_(info.channels == 1 ? AL_FORMAT_MONO16 : info.channels == 2 ? AL_FORMAT_STEREO16
	                                                                         : AL_INVALID_VALUE)
	    , seek_pos_(0) {
		assert(f);
		assert(format_ != AL_INVALID_VALUE);
	}

	size_t load_buffer(ALuint buf) {
		// seek to beginning if in a end
		if (is_end()) {
			if (info_.seekable) {
				if (sf_seek(file_.get(), 0, SEEK_SET) == -1) {
					Output::Error("libsndfile seek error: %s", sf_strerror(file_.get()));
					return 0;
				}
			} else {
				file_.reset(sf_open(filename_.c_str(), SFM_READ, &info_), sf_close);
				if (!file_) {
					Output::Error("libsndfile open error: %s", sf_strerror(NULL));
					return 0;
				}
			}
			loop_count_++;
			seek_pos_ = 0;
		}

		data_.resize(info_.channels * info_.samplerate * SECOND_PER_BUFFER);
		sf_count_t const read_size =
		    sf_readf_short(file_.get(), &data_.front(), data_.size() / info_.channels);
		alBufferData(buf, format_, &data_.front(), sizeof(int16_t) * data_.size(),
		             info_.samplerate);
		seek_pos_ += read_size;
		return read_size;
	}

	bool is_end() const {
		return seek_pos_ >= info_.frames;
	}

private:
	std::string const filename_;
	std::shared_ptr<SNDFILE> file_;
	SF_INFO info_;
	ALenum const format_;
	sf_count_t seek_pos_;

	std::vector<int16_t> data_;
};

struct ALAudio::midi_loader : public ALAudio::buffer_loader {
	midi_loader(source &src, std::string const &filename) : source_(src), filename_(filename) {
		src.init_midi();
		source_.player.reset(new_fluid_player(source_.synth.get()), &delete_fluid_player);
		if (fluid_player_add(source_.player.get(), filename.c_str()) == FLUID_FAILED)
			Output::Warning("Couldn't load %s midi sound.", filename.c_str());
		if (fluid_player_play(source_.player.get()) == FLUID_FAILED)
			Output::Warning("Couldn't play %s midi sound.", filename.c_str());
	}

	bool is_end() const {
		return fluid_player_get_status(source_.player.get()) != FLUID_PLAYER_PLAYING;
	}

	size_t load_buffer(ALuint buf) {
		if (is_end()) {
			source_.seq.reset(new_fluid_sequencer2(false), &delete_fluid_sequencer);
			if (fluid_sequencer_register_fluidsynth(source_.seq.get(), source_.synth.get()) == FLUID_FAILED)
				Output::Error("Fluidsynth error: %s", fluid_synth_error(source_.synth.get()));

			if (fluid_player_add(source_.player.get(), filename_.c_str()) == FLUID_FAILED)
				Output::Error("Fluidsynth error: %s", fluid_synth_error(source_.synth.get()));

			loop_count_++;
		}

		data_.resize(2 * source_.sample_rate * SECOND_PER_BUFFER);
		if (fluid_synth_write_s16(source_.synth.get(), data_.size() / 2, &data_.front(), 0, 2,
		                          &data_.front(), 1, 2) == FLUID_FAILED) {
			Output::Error("Fluidsynth error: %s", fluid_synth_error(source_.synth.get()));
		}
		alBufferData(buf, AL_FORMAT_STEREO16, &data_.front(), sizeof(int16_t) * data_.size(),
		             source_.sample_rate);

		return data_.size() / 2;
	}

	unsigned midi_ticks() const {
		return fluid_sequencer_get_tick(source_.seq.get());
	}

private:
	source &source_;
	std::string const filename_;

	std::vector<int16_t> data_;
};

std::shared_ptr<ALAudio::buffer_loader>
ALAudio::create_loader(source &src, std::string const &filename) const {
	SET_CONTEXT(ctx_);

	if (filename.empty()) {
		Output::Error("Failed loading audio file: %s", filename.c_str());
	}

	std::shared_ptr<buffer_loader> snd = sndfile_loader::create(filename);
	return snd ? snd : std::make_shared<midi_loader>(std::ref(src), filename);
}

std::shared_ptr<ALAudio::buffer_loader>
ALAudio::getMusic(source &src, std::string const &file) const {
	return create_loader(src, file);
}

std::shared_ptr<ALAudio::buffer_loader>
ALAudio::getSound(source &src, std::string const &file) const {
	return create_loader(src, file);
}

void ALAudio::Update() {
	bgm_src_->update();

	for (source_list::iterator i = se_src_.begin(); i < se_src_.end(); ++i) {
		i->get()->update();

		ALenum state = AL_INVALID_VALUE;
		alGetSourcei(i->get()->get(), AL_SOURCE_STATE, &state);
		if (state == AL_STOPPED) {
			i = se_src_.erase(i);
		}
	}
}

ALAudio::ALAudio(char const *const dev_name) {
	dev_.reset(alcOpenDevice(dev_name), &alcCloseDevice);
	assert(dev_);

	ctx_.reset(alcCreateContext(dev_.get(), NULL), &alcDestroyContext);
	assert(ctx_);

	alcMakeContextCurrent(ctx_.get());

	SET_CONTEXT(ctx_);

	bgm_src_ = create_source(true);

	if (!getenv("DEFAULT_SOUNDFONT")) {
		Output::Error("Default sound font not found.");
	}
}

std::shared_ptr<ALAudio::source> ALAudio::create_source(bool loop) const {
	SET_CONTEXT(ctx_);

	ALuint ret = AL_NONE;
	alGenSources(1, &ret);
	print_al_error();
	assert(ret != AL_NONE);

	return std::make_shared<source>(ctx_, ret, loop);
}

void ALAudio::BGM_Play(std::string const &file, int volume, int pitch, int fadein) {
	SET_CONTEXT(ctx_);

	alSourcef(bgm_src_->get(), AL_PITCH, pitch * 0.01f);
	bgm_src_->set_volume(volume * 0.01f);
	bgm_src_->set_buffer_loader(getMusic(*bgm_src_, file));
	bgm_src_->fade_in(fadein);
}

void ALAudio::BGM_Stop() {
	SET_CONTEXT(ctx_);
	alSourceStop(bgm_src_->get());
}

bool ALAudio::BGM_PlayedOnce() const {
	SET_CONTEXT(ctx_);
	return bgm_src_->loop_count() > 0;
}

bool ALAudio::BGM_IsPlaying() const {
	SET_CONTEXT(ctx_);

	ALenum state;
	alGetSourcei(bgm_src_->get(), AL_SOURCE_STATE, &state);

	return (state == AL_PLAYING);
}

unsigned ALAudio::BGM_GetTicks() const {
	SET_CONTEXT(ctx_);
	return bgm_src_->midi_ticks();
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

void ALAudio::BGM_Volume(int volume) {
	SET_CONTEXT(ctx_);
	bgm_src_->set_volume(volume * 0.01f);
}

void ALAudio::BGM_Pitch(int pitch) {
	SET_CONTEXT(ctx_);
	alSourcef(bgm_src_->get(), AL_PITCH, pitch * 0.01f);
}

void ALAudio::SE_Play(std::string const &file, int volume, int pitch) {
	SET_CONTEXT(ctx_);

	std::shared_ptr<source> src = create_source(false);

	alSourcef(src->get(), AL_PITCH, pitch * 0.01f);
	src->set_volume(volume * 0.01f);
	src->set_buffer_loader(getSound(*src, file));

	se_src_.push_back(src);
}

void ALAudio::SE_Stop() {
	SET_CONTEXT(ctx_);
	for (source_list::iterator i = se_src_.begin(); i < se_src_.end(); ++i) {
		alSourceStop(i->get()->get());
	}
	se_src_.clear();
}

#endif
