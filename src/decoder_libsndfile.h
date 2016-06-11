#ifndef _EASYRPG_AUDIO_DECODER_LIBSNDFILE_H_
#define _EASYRPG_AUDIO_DECODER_LIBSNDFILE_H_

// Headers
#include "audio_decoder.h"
#include <string>
#ifdef HAVE_LIBSNDFILE
#include <sndfile.h>
#endif
#include <memory>

/**
 * Audio decoder for WAV powered by libsndfile
 */
class LibsndfileDecoder : public AudioDecoder {
public:
	LibsndfileDecoder();

	~LibsndfileDecoder();

	bool Open(FILE* file) override;

	bool Seek(size_t offset, Origin origin) override;

	bool IsFinished() const override;

	void GetFormat(int& frequency, AudioDecoder::Format& format, int& channels) const override;

	bool SetFormat(int frequency, AudioDecoder::Format format, int channels) override;

private:
	int FillBuffer(uint8_t* buffer, int length) override;
	Format output_format;
	FILE * file_;
	bool finished;

	#ifdef HAVE_LIBSNDFILE
	SNDFILE *soundfile;
	SF_INFO soundinfo;
	#endif
};

#endif
