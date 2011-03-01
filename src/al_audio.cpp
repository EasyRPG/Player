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

#if defined(USE_OPENAL)

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#if defined(__APPLE__) && defined(__MACH__)
	#include <OpenAL/al.h>
	#include <OpenAL/alc.h>
#else
	#include <AL/al.h>
	#include <AL/alc.h>
#endif

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/shared_ptr.hpp>

#include <cassert>
#include <list>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "system.h"
#include "audio.h"
#include "filefinder.h"
#include "output.h"

////////////////////////////////////////////////////////////
namespace Audio { namespace {
		boost::shared_ptr<ALCdevice> device_;
		boost::shared_ptr<ALCcontext> context_;

		void delete_source(ALuint* src)
		{
			alDeleteSources(1, src);
			delete src;
		}
		boost::shared_ptr<ALuint> create_source()
		{
			ALuint ret = AL_INVALID_VALUE;
			alGenSources(1, &ret);
			assert(ret != AL_INVALID_VALUE);
			return boost::shared_ptr<ALuint>(new ALuint(ret), &delete_source);
		}
		void delete_buffer(ALuint* buf)
		{
			alDeleteBuffers(1, buf);
			delete buf;
		}
		boost::shared_ptr<ALuint> create_buffer()
		{
			ALuint ret = AL_INVALID_VALUE;
			alGenBuffers(1, &ret);
			assert(ret != AL_INVALID_VALUE);
			return boost::shared_ptr<ALuint>(new ALuint(ret), &delete_buffer);
		}

		std::map<std::string, boost::shared_ptr<ALuint> > musicPool_;
		std::map<std::string, boost::shared_ptr<ALuint> > soundPool_;

		boost::shared_ptr<ALuint> bgmSource_;
		boost::shared_ptr<ALuint> bgsSource_;
		std::list<boost::shared_ptr<ALuint> > seSource_;
		std::list<boost::shared_ptr<ALuint> > meSource_;
} }

////////////////////////////////////////////////////////////
namespace {
	void initFFmpeg() {
		static bool finished_ = false;
		if(!finished_) {
			av_register_all();
			av_log_set_level(AV_LOG_ERROR);
			finished_ = true;
		}
	}
	struct MyFile {
		AVFormatContext* fmtCtx;
		struct  MyStream {
			struct BufferInfo {
				int channels, bits, rate;

				ALenum toALFormat() const {
					if(bits == 8) {
						if(channels == 1) return AL_FORMAT_MONO8;
						if(channels == 2) return AL_FORMAT_STEREO8;
						if(alIsExtensionPresent("AL_EXT_MCFORMATS")) {
							if(channels == 4) return alGetEnumValue("AL_FORMAT_QUAD8");
							if(channels == 6) return alGetEnumValue("AL_FORMAT_51CHN8");
						}
					}
					if(bits == 16) {
						if(channels == 1) return AL_FORMAT_MONO16;
						if(channels == 2) return AL_FORMAT_STEREO16;
						if(alIsExtensionPresent("AL_EXT_MCFORMATS")) {
							if(channels == 4) return alGetEnumValue("AL_FORMAT_QUAD16");
							if(channels == 6) return alGetEnumValue("AL_FORMAT_51CHN16");
						}
					}

					assert(false);
					return AL_INVALID_ENUM;
				}
			} format;
			AVCodecContext* codecCtx;
			int const streamIndex;
			std::vector<uint8_t> data;

			MyFile& parent;

			MyStream(MyFile& p, int const index) : streamIndex(index), parent(p) {
				this->codecCtx = p.fmtCtx->streams[index]->codec;

				AVCodec* avcodec = avcodec_find_decoder(this->codecCtx->codec_id);
				if(!avcodec || avcodec_open(this->codecCtx, avcodec) < 0) {
					assert(!"codec open error");
				}

				format.rate = codecCtx->sample_rate;
				format.channels = codecCtx->channels;
				format.bits =
					codecCtx->sample_fmt == SAMPLE_FMT_U8? 8 :
					codecCtx->sample_fmt == SAMPLE_FMT_S16? 16 :
					codecCtx->sample_fmt == SAMPLE_FMT_S32? 32 :
					0;
				assert(format.bits);
			}
			~MyStream() { avcodec_close(this->codecCtx); }

			boost::shared_ptr<ALuint> createBuffer() const {
				boost::shared_ptr<ALuint> ret = Audio::create_buffer();
				alBufferData(*ret, format.toALFormat(), &data.front(), data.size(), format.rate);
				return ret;
			}
		};
		boost::ptr_vector<MyStream> streams;

		MyFile(char const* fname) {
			initFFmpeg();


			if(av_open_input_file(&this->fmtCtx, fname, NULL, 0, NULL) != 0) {
				Output::Error("file open error %s", fname);
			}
			for(int i = 0; i < this->fmtCtx->nb_streams; i++) {
				if(this->fmtCtx->streams[i]->codec->codec_type != CODEC_TYPE_AUDIO)
					continue;

				streams.push_back(new MyStream(*this, i));
			}

			AVPacket packet;
			while(av_read_frame(this->fmtCtx, &packet) >= 0) {
				MyStream& i = streams[packet.stream_index];

				int outSize = AVCODEC_MAX_AUDIO_FRAME_SIZE;
				size_t dstIndex = 0; i.data.resize(outSize);
				while(avcodec_decode_audio3(i.codecCtx,
											reinterpret_cast<int16_t*>(&i.data[dstIndex]),
											&outSize, &packet) > 0)
				{
					dstIndex += AVCODEC_MAX_AUDIO_FRAME_SIZE - outSize;
					i.data.resize(dstIndex + AVCODEC_MAX_AUDIO_FRAME_SIZE);
					outSize = AVCODEC_MAX_AUDIO_FRAME_SIZE;
				}
				i.data.resize(dstIndex);
			}
			av_free_packet(&packet);
		}
		~MyFile() { av_close_input_file(this->fmtCtx); }
	};

	ALuint const& getMusic(std::string const& fname)
	{
		using namespace Audio;
		std::map<std::string, boost::shared_ptr<ALuint> >::iterator i = musicPool_.find(fname);
		if(i == musicPool_.end()) {
			return *musicPool_.insert(std::make_pair(fname,
													 MyFile(FileFinder::FindMusic(fname).c_str()).streams[0].createBuffer())).first->second;
		} else return *i->second;
	}
	ALuint const& getSound(std::string const& fname)
	{
		using namespace Audio;
		std::map<std::string, boost::shared_ptr<ALuint> >::iterator i = soundPool_.find(fname);
		if(i == musicPool_.end()) {
			return *soundPool_.insert(std::make_pair(fname,
													 MyFile(FileFinder::FindSound(fname).c_str()).streams[0].createBuffer())).first->second;
		} else return *i->second;
	}

	void eraseStopped(std::list<boost::shared_ptr<ALuint> >& target) {
		for(std::list<boost::shared_ptr<ALuint> >::iterator i = target.begin(); i != target.end(); ++i) {
			ALint stopped; alGetSourcei(*(*i), AL_STOPPED, &stopped);
			if(stopped == AL_TRUE) { i = target.erase(i); }
		}
	}
}

void boost::throw_exception(std::exception const& exp)
{
	Output::Error("exception: %s", exp.what());
	assert(false);
}

////////////////////////////////////////////////////////////
void Audio::Init()
{
	device_.reset(alcOpenDevice(NULL), &alcCloseDevice);
	assert(device_);
	context_.reset(alcCreateContext(device_.get(), NULL), &alcDestroyContext);
	assert(context_);
	alcMakeContextCurrent(context_.get());

	bgmSource_ = create_source();
}

////////////////////////////////////////////////////////////
void Audio::Quit()
{
	soundPool_.clear();
	musicPool_.clear();

	bgsSource_.reset();
	bgmSource_.reset();
	seSource_.clear();
	meSource_.clear();

	context_.reset();
	device_.reset();
}

////////////////////////////////////////////////////////////
void Audio::BGM_Play(std::string const& file, int volume, int pitch)
{
	BGM_Stop();

	alSourceRewind(*bgmSource_);
	alSourcei(*bgmSource_, AL_BUFFER, getMusic(file));
	alSourcef(*bgmSource_, AL_GAIN, volume * 0.01f);
	alSourcei(*bgmSource_, AL_PITCH, pitch * 0.01f);
	alSourcei(*bgmSource_, AL_LOOPING, AL_TRUE);
	alSourcePlay(*bgmSource_);
}

////////////////////////////////////////////////////////////
void Audio::BGM_Stop()
{
	alSourceStop(*bgmSource_);
}

////////////////////////////////////////////////////////////
void Audio::BGM_Fade(int fade)
{
	// TODO
}

////////////////////////////////////////////////////////////
void Audio::BGM_Pause()
{
	alSourcePause(*bgmSource_);
}

////////////////////////////////////////////////////////////
void Audio::BGM_Resume()
{
	alSourcePlay(*bgmSource_);
}

////////////////////////////////////////////////////////////
void Audio::BGS_Play(std::string const& file, int volume, int pitch)
{
	BGS_Stop();

	alSourceRewind(*bgsSource_);
	alSourcei(*bgsSource_, AL_BUFFER, getSound(file));
	alSourcef(*bgsSource_, AL_GAIN, volume * 0.01f);
	alSourcei(*bgsSource_, AL_PITCH, pitch * 0.01f);
	alSourcei(*bgsSource_, AL_LOOPING, AL_TRUE);
	alSourcePlay(*bgsSource_);
}

////////////////////////////////////////////////////////////
void Audio::BGS_Stop()
{
	alSourceStop(*bgsSource_);
}

////////////////////////////////////////////////////////////
void Audio::BGS_Fade(int fade)
{
	// TODO
}

////////////////////////////////////////////////////////////
void Audio::ME_Play(std::string const& file, int volume, int pitch)
{
	eraseStopped(seSource_);

	boost::shared_ptr<ALuint> src = create_source();

	alSourcei(*src, AL_BUFFER, getMusic(file));
	alSourcef(*src, AL_GAIN, volume * 0.01f);
	alSourcei(*src, AL_PITCH, pitch * 0.01f);
	alSourcei(*src, AL_LOOPING, AL_FALSE);
	alSourcePlay(*src);

	meSource_.push_back(src);
}

////////////////////////////////////////////////////////////
void Audio::ME_Stop()
{
	for(std::list<boost::shared_ptr<ALuint> >::iterator i = meSource_.begin(); i != meSource_.end(); ++i) {
		alSourceStop(*(*i));
	}
	meSource_.clear();
}

////////////////////////////////////////////////////////////
void Audio::ME_Fade(int fade)
{
	// TODO
}

////////////////////////////////////////////////////////////
void Audio::SE_Play(std::string const& file, int volume, int pitch)
{
	eraseStopped(seSource_);

	boost::shared_ptr<ALuint> src = create_source();

	alSourcei(*src, AL_BUFFER, getSound(file));
	alSourcef(*src, AL_GAIN, volume * 0.01f);
	alSourcei(*src, AL_PITCH, pitch * 0.01f);
	alSourcei(*src, AL_LOOPING, AL_FALSE);
	alSourcePlay(*src);

	seSource_.push_back(src);
}

////////////////////////////////////////////////////////////
void Audio::SE_Stop()
{
	for(std::list<boost::shared_ptr<ALuint> >::iterator i = seSource_.begin(); i != seSource_.end(); ++i) {
		alSourceStop(*(*i));
	}
	seSource_.clear();
}

#endif
