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

#if defined(HAVE_LIBSPEEXDSP) || defined(HAVE_LIBSAMPLERATE) 

#include <cassert>
#include "audio_resampler.h"
#include "output.h"

#define ERROR -1
#define STANDARD_PITCH 100

/**
 * Utility function used to convert a buffer of a arbitrary AudioDecoder::Format to a float buffer
 * 
 * @param[in] wrapped_decoder The decoder from which audio samples are read
 * @param[inout] buffer The buffer which will receive the converted samples, 
 *			has to be at least amount_of_samples_to_read*sizeof(float) bytes big.
 * @param[in] amount_of_samples_to_read The number of samples to read.
 * @param[in] input_samplesize The size of one sample of the decoder in it's original format - given in bytes
 * @param[in] input_format The original format of the samples
 * 
 * @return The number of converted samples - if this number is smaller than amount_of_samples_to_read the wrapped decoder has reaches it's end.
 *		If the returned value has a negative value an error occured.
 */
inline static int DecodeAndConvertFloat(AudioDecoder * wrapped_decoder,
										uint8_t * buffer, 
										int amount_of_samples_to_read, 
										const int input_samplesize, 
										const AudioDecoder::Format input_format){
	float* bufferAsFloat = (float*)buffer;
	if (wrapped_decoder->IsFinished()) return 0; //Workaround for decoders which don't detect there own end
	int amount_of_samples_read = wrapped_decoder->Decode(buffer, amount_of_samples_to_read*input_samplesize);
	if (amount_of_samples_read <= 0) {
		return amount_of_samples_read; //error occured - or nothing read
	}
	else {
		amount_of_samples_read /= input_samplesize;
	}
	//Convert the read data (amount_of_data_read is at least one at this moment)
	switch (input_format) {
		case AudioDecoder::Format::S8:
			//Convert inplace (the last frames are unused if smaller)
			for (int i = amount_of_samples_read - 1; i >= 0; i--) {
				bufferAsFloat[i] = ((int8_t*)bufferAsFloat)[i] / 128.0;
			}
			break;
		case AudioDecoder::Format::U8:
			//Convert inplace (the last frames are unused if smaller)
			for (int i = amount_of_samples_read - 1; i >= 0; i--) {
				bufferAsFloat[i] = ((uint8_t*)bufferAsFloat)[i] / 128.0 - 1.0;
			}
			break;
		case AudioDecoder::Format::S16:
			//Convert inplace (the last frames are unused if smaller)
			for (int i = amount_of_samples_read - 1; i >= 0; i--) {
				bufferAsFloat[i] = ((int16_t*)bufferAsFloat)[i] / 32768.0;
			}
			break;
		case AudioDecoder::Format::U16:
			//Convert inplace (the last frames are unused if smaller)
			for (int i = amount_of_samples_read - 1; i >= 0; i--) {
				bufferAsFloat[i] = ((uint16_t*)bufferAsFloat)[i] / 32768.0 - 1.0;
			}
			break;
		case AudioDecoder::Format::S32:
			//Convert inplace (same size)
			for (int i = amount_of_samples_read - 1; i >= 0; i--) {
				bufferAsFloat[i] = ((int32_t*)bufferAsFloat)[i] / 2147483648.0;
			}
			break;
		case AudioDecoder::Format::U32:
			//Convert inplace (same size)
			for (int i = amount_of_samples_read - 1; i >= 0; i--) {
				bufferAsFloat[i] = ((uint32_t*)bufferAsFloat)[i] / 2147483648.0 - 1.0;
			}
			break;
		case AudioDecoder::Format::F32:
			//Nothing to convert
			break;
	}
	return amount_of_samples_read;
}

#if defined(HAVE_LIBSPEEXDSP)

/**
 * Utility function used to convert a buffer of a arbitrary AudioDecoder::Format to a int16 buffer
 * 
 * @param[in] wrapped_decoder The decoder from which audio samples are read
 * @param[inout] buffer The buffer which will receive the converted samples, 
 *			has to be at least amount_of_samples_to_read*max(sizeof(int16_t),input_samplesize) bytes big.
 * @param[in] amount_of_samples_to_read The number of samples to read.
 * @param[in] input_samplesize The size of one sample of the decoder in it's original format - given in bytes
 * @param[in] input_format The original format of the samples
 * 
 * @return The number of converted samples - if this number is smaller than amount_of_samples_to_read the wrapped decoder has reaches it's end.
 *		If the returned value has a negative value an error occured.
 */
inline static int DecodeAndConvertInt16(AudioDecoder * wrapped_decoder, 
										uint8_t * buffer, 
										int amount_of_samples_to_read, 
										const int input_samplesize, 
										const AudioDecoder::Format input_format){
	int16_t* bufferAsInt16 = (int16_t*)buffer;
	if (wrapped_decoder->IsFinished()) return 0; //Workaround for decoders which don't detect there own end
	int amount_of_samples_read = wrapped_decoder->Decode(buffer, amount_of_samples_to_read*input_samplesize);
	if (amount_of_samples_read <= 0) {
		return amount_of_samples_read; //error occured - or nothing read
	}
	else {
		//Convert the number of bytes to the number of samples
		amount_of_samples_read /= input_samplesize;
	}
	//Convert the read data (amount_of_data_read is at least one at this moment)
	switch (input_format) {
		case AudioDecoder::Format::S8:
			//Convert inplace (the last frames are unused if smaller)
			for (int i = amount_of_samples_read - 1; i >= 0; i--) {
				bufferAsInt16[i] = ((int8_t*)bufferAsInt16)[i] << 8;
			}
			break;
		case AudioDecoder::Format::U8:
			//Convert inplace (the last frames are unused if smaller)
			for (int i = amount_of_samples_read - 1; i >= 0; i--) {
				bufferAsInt16[i] = (((int16_t)(((uint8_t*)bufferAsInt16)[i])) - 128) << 8;
			}
			break;
		case AudioDecoder::Format::S16:
			//Nothing to convert
			break;
		case AudioDecoder::Format::U16:
			//Convert unsigned to signed
			for (int i = amount_of_samples_read - 1; i >= 0; i--) {
				bufferAsInt16[i] = (int16_t)(((int32_t)(((uint16_t*)bufferAsInt16)[i])) - 32768);
			}
			break;
		case AudioDecoder::Format::S32:
			//Convert inplace (from front to back to prevent overwriting the buffer)
			for (int i = 0; i < amount_of_samples_read; i++) {
				bufferAsInt16[i] = (int16_t)((((int32_t*)bufferAsInt16)[i]) >> 16);
			}
			break;
		case AudioDecoder::Format::U32:
			//Convert inplace (from front to back to prevent overwriting the buffer)
			for (int i = 0; i < amount_of_samples_read; i++) {
				bufferAsInt16[i] = (int16_t)(((int32_t)((((uint32_t*)bufferAsInt16)[i]) >> 16)) - 32768);
			}
			break;
		case AudioDecoder::Format::F32:
			//Convert inplace (from front to back to prevent overwriting the buffer)
			for (int i = 0; i < amount_of_samples_read; i++) {
				float number = ((((float*)bufferAsInt16)[i])*32768.0);
				bufferAsInt16[i] = (number <= 32767.0) ? ((number >= -32768.0) ? number : -32768) : 32767;
			}
			break;
	}
	return amount_of_samples_read;
}
#endif

AudioResampler::AudioResampler(AudioDecoder * wrapped, bool pitch_handled, AudioResampler::Quality quality)
{
	//There is no need for a standalone resampler decoder
	assert(wrapped != 0);

	wrapped_decoder = wrapped;
	music_type = wrapped->GetType();
	lasterror = 0;
	pitch_handled_by_decoder = pitch_handled;

	#if defined(HAVE_LIBSPEEXDSP)
		switch (quality) {
			case Quality::Low:
				sampling_quality = 1;
				break;
			case Quality::Medium:
				sampling_quality = 3;
				break;
			case Quality::High:
				sampling_quality = 5;
				break;
		}
	#elif defined(HAVE_LIBSAMPLERATE)
		switch (quality) {
			case Quality::Low:
				sampling_quality = SRC_SINC_FASTEST;
				break;
			case Quality::Medium:
				sampling_quality = SRC_SINC_MEDIUM_QUALITY;
				break;
			case Quality::High:
				sampling_quality = SRC_SINC_BEST_QUALITY;
				break;
		}
	#endif

	finished = false;
	pitch = 100;

}

AudioResampler::~AudioResampler() {
	if (conversion_state != 0) {
	#if defined(HAVE_LIBSPEEXDSP)
			speex_resampler_destroy(conversion_state);
	#elif defined(HAVE_LIBSAMPLERATE)
			src_delete(conversion_state);
	#endif
	}
	if (wrapped_decoder != 0) {
		delete wrapped_decoder;
	}
}

bool AudioResampler::WasInited() const {
	return wrapped_decoder->WasInited();
}

bool AudioResampler::Open(FILE* file) {
	if (wrapped_decoder->Open(file)) {
		wrapped_decoder->GetFormat(input_rate, input_format, nr_of_channels);

		//determine if the input format is supported by the resampler
		switch (input_format) {
			case Format::F32: output_format = input_format; break;
		#ifdef HAVE_LIBSPEEXDSP
			case Format::S16: output_format = input_format; break;
		#endif
			default: output_format = Format::F32; break;
		}

		//Set input format to output_format if possible
		wrapped_decoder->SetFormat(input_rate, output_format, nr_of_channels);
		//Reread format to get new values
		wrapped_decoder->GetFormat(input_rate, input_format, nr_of_channels);
		output_rate = input_rate;

		#if defined(HAVE_LIBSPEEXDSP)
			conversion_state = speex_resampler_init(nr_of_channels, input_rate, output_rate, sampling_quality, &lasterror);
			conversion_data.ratio_num = input_rate;
			conversion_data.ratio_denom = output_rate;
			speex_resampler_skip_zeros(conversion_state);
		#elif defined(HAVE_LIBSAMPLERATE)
			conversion_state = src_new(sampling_quality, nr_of_channels, &lasterror);
		#endif

		//Init the conversion data structure
		conversion_data.input_frames = 0;
		conversion_data.input_frames_used = 0;
		finished = false;
		return conversion_state != 0;
	}
	else {
		return false;
	}
}

bool AudioResampler::Seek(size_t offset, Origin origin) {
	if (wrapped_decoder->Seek(offset, origin)) {
		//reset conversio data
		conversion_data.input_frames = 0;
		conversion_data.input_frames_used = 0;
		finished = wrapped_decoder->IsFinished();
		#if defined(HAVE_LIBSPEEXDSP)
			speex_resampler_reset_mem(conversion_state);
		#elif defined(HAVE_LIBSAMPLERATE)
			src_reset(conversion_state);
		#endif
		return true;
	}
	return false;

}

size_t AudioResampler::Tell() {
	return wrapped_decoder->Tell();
}

int AudioResampler::GetTicks() {
	return wrapped_decoder->GetTicks();
}

bool AudioResampler::IsFinished() const {
	return finished;
}

void AudioResampler::GetFormat(int& frequency, AudioDecoder::Format& format, int& channels) const {
	frequency = output_rate;
	format = output_format;
	channels = nr_of_channels;
}

bool AudioResampler::SetFormat(int freq, AudioDecoder::Format fmt, int channels) {
	//Check whether requested format is supported by the resampler
	switch (fmt) {
	case Format::F32: output_format = fmt; break;
	#ifdef HAVE_LIBSPEEXDSP
		case Format::S16: output_format = fmt; break;
	#endif
	default: break;
	}
	wrapped_decoder->SetFormat(input_rate, output_format, channels);
	wrapped_decoder->GetFormat(input_rate, input_format, nr_of_channels);
	output_rate = freq;
	return (nr_of_channels == channels&&output_format == fmt);
}

int AudioResampler::GetPitch() const {
	if (pitch_handled_by_decoder) {
		return wrapped_decoder->GetPitch();
	}
	else {
		return pitch;
	}
}

bool AudioResampler::SetPitch(int pitch_) {
	if (pitch_handled_by_decoder) {
		return wrapped_decoder->SetPitch(pitch_);
	}
	else {
		pitch = pitch_;
		return true;
	}
}

int AudioResampler::FillBuffer(uint8_t* buffer, int length) {
	int amount_filled = 0;
	if((input_rate == output_rate) && ((pitch == STANDARD_PITCH) || pitch_handled_by_decoder)) {
		//Do only format conversion
		amount_filled = FillBufferSameRate(buffer, length);
	}
	else {
		if (conversion_state == 0) {
			error_message = "internal error: state pointer is a nullptr";
			amount_filled = ERROR;
		}
		else {
			//Do samplerate conversion
			amount_filled = FillBufferDifferentRate(buffer, length);
		}
	}
	//Clear the remaining buffer as specified in audio_decoder.h
	for (int i = (amount_filled > 0) ? amount_filled : 0; i < length; i++) {
		buffer[i] = 0;
	}
	return amount_filled;
}

int AudioResampler::FillBufferSameRate(uint8_t* buffer, int length) {
	const int input_samplesize = GetSamplesizeForFormat(input_format);
	const int output_samplesize = GetSamplesizeForFormat(output_format);
	//The buffer size has to be a multiple of a frame
	const int buffer_size=sizeof(internal_buffer) - sizeof(internal_buffer)%(nr_of_channels*input_samplesize);
	
	int total_output_frames = length / (output_samplesize*nr_of_channels);
	int amount_of_data_to_read = 0;
	int amount_of_data_read = total_output_frames*nr_of_channels;
	
	int decoded = 0;

	if (input_samplesize > output_samplesize) {
		//It is necessary to use the internal_buffer to convert the samples.
		while (total_output_frames > 0) {
			amount_of_data_to_read = buffer_size / input_samplesize;
			
			//limit amount_of_data_to_read in the last loop 
			amount_of_data_to_read = (amount_of_data_to_read > total_output_frames) ? total_output_frames : amount_of_data_to_read;

			switch (output_format) {
				case AudioDecoder::Format::F32: 
				amount_of_data_read = DecodeAndConvertFloat(wrapped_decoder, internal_buffer, amount_of_data_to_read, input_samplesize, input_format); 
				break;
			#ifdef HAVE_LIBSPEEXDSP
				case AudioDecoder::Format::S16: 
				amount_of_data_read = DecodeAndConvertInt16(wrapped_decoder, internal_buffer, amount_of_data_to_read, input_samplesize, input_format); 
				break;
			#endif
				default: error_message = "internal error: output_format is not convertable"; return ERROR;
			}
			if (amount_of_data_read < 0) {
				error_message = wrapped_decoder->GetError();
				return amount_of_data_read; //error occured
			}

			//Copy the converted samples
			for (int i = 0; i < amount_of_data_read*output_samplesize; i++) {
				buffer[i] = internal_buffer[i];
			}
			//Prepare next loop
			total_output_frames -= amount_of_data_read;
			decoded += amount_of_data_read;
			buffer += amount_of_data_read*output_samplesize;

			//If the end of the decoder is reached (it has finished)
			if (amount_of_data_read < amount_of_data_to_read) {
				break;
			}

		}
	}
	else {
		//It is possible to work inplace as length is specified for the output samplesize.
		switch (output_format) {
			case AudioDecoder::Format::F32: 
			decoded = DecodeAndConvertFloat(wrapped_decoder, buffer, amount_of_data_read, input_samplesize, input_format); 
			break;
		#ifdef HAVE_LIBSPEEXDSP
			case AudioDecoder::Format::S16: 
			decoded = DecodeAndConvertInt16(wrapped_decoder, buffer, amount_of_data_read, input_samplesize, input_format); 
			break;
		#endif
			default: error_message = "internal error: output_format is not convertable"; return ERROR;
		}
	}


	finished = wrapped_decoder->IsFinished();
	if (decoded < 0) {
		error_message = wrapped_decoder->GetError();
		return decoded;
	}
	else {
		return decoded*output_samplesize;
	}
}

int AudioResampler::FillBufferDifferentRate(uint8_t* buffer, int length) {
	const int input_samplesize = GetSamplesizeForFormat(input_format);
	const int output_samplesize = GetSamplesizeForFormat(output_format);
	//The buffer size has to be a multiple of a frame
	const int buffer_size=sizeof(internal_buffer) - sizeof(internal_buffer)%(nr_of_channels*((input_samplesize>output_samplesize) ? input_samplesize : output_samplesize));
	
	int total_output_frames = length / (output_samplesize*nr_of_channels);
	int amount_of_samples_to_read = 0;
	int amount_of_samples_read = 0;
	
	uint8_t * advanced_input_buffer = internal_buffer;
	int unused_frames = 0;
	int empty_buffer_space = 0;
	int error = 0;
	
	#ifdef HAVE_LIBSPEEXDSP
		spx_uint32_t numerator = 0;
		spx_uint32_t denominator = 0;
	#endif

	while (total_output_frames > 0) {
		//Calculate how much frames of the last cycle are unused - to reuse them
		unused_frames = conversion_data.input_frames - conversion_data.input_frames_used;
		empty_buffer_space = buffer_size / output_samplesize - unused_frames*nr_of_channels;
		
		advanced_input_buffer = internal_buffer;

		//If there is still unused data in the input_buffer order it to the front
		for (int i = 0; i < unused_frames*nr_of_channels*output_samplesize; i++) {
			*advanced_input_buffer = *(advanced_input_buffer + empty_buffer_space*output_samplesize);
			advanced_input_buffer++;
		}
		//advanced_input_buffer is now offset to the first frame of new data!

		//ensure that the input buffer is not able to overrun
		amount_of_samples_to_read = (input_samplesize > output_samplesize) ? (empty_buffer_space*output_samplesize) / input_samplesize : empty_buffer_space;

		//Read as many frames as needed to refill the buffer (filled after the conversion to float)
		if (amount_of_samples_to_read != 0) {
			switch (output_format) {
				case AudioDecoder::Format::F32: amount_of_samples_read = DecodeAndConvertFloat(wrapped_decoder, advanced_input_buffer, amount_of_samples_to_read, input_samplesize, input_format); break;
			#ifdef HAVE_LIBSPEEXDSP
				case AudioDecoder::Format::S16:  amount_of_samples_read = DecodeAndConvertInt16(wrapped_decoder, advanced_input_buffer, amount_of_samples_to_read, input_samplesize, input_format); break;
			#endif
				default: error_message = "internal error: output_format is not convertable"; return ERROR;
			}
			if (amount_of_samples_read < 0) {
				error_message = wrapped_decoder->GetError();
				return amount_of_samples_read; //error occured
			}
		}
		//Now we have a prepared full buffer of converted values


		//Prepare the source data
		conversion_data.input_frames = amount_of_samples_read / nr_of_channels + unused_frames;
		conversion_data.output_frames = total_output_frames;

		#if defined(HAVE_LIBSPEEXDSP)
			conversion_data.input_frames_used = conversion_data.input_frames;
			conversion_data.output_frames_gen = conversion_data.output_frames;

			//libspeexdsp defines a sample rate conversion with a fraction (input/output)
			numerator = input_rate*pitch;
			denominator = output_rate * STANDARD_PITCH;
			if (pitch_handled_by_decoder) {
				numerator = input_rate;
				denominator = output_rate;
			}
			if (conversion_data.ratio_num != numerator || conversion_data.ratio_denom != denominator) {
				int err=speex_resampler_set_rate_frac(conversion_state, numerator, denominator, input_rate, output_rate);
				conversion_data.ratio_num = numerator;
				conversion_data.ratio_denom = denominator;
			}
			
			//A pitfall from libspeexdsp if the output buffer is defined to big - everything stutters -achieved good values with the same size as the input buffer for a maximum
			conversion_data.output_frames_gen=(conversion_data.input_frames<conversion_data.output_frames_gen) ? conversion_data.input_frames :conversion_data.output_frames_gen;
			
			switch (output_format) {
			case Format::F32:
				error = speex_resampler_process_interleaved_float(conversion_state, (float*)internal_buffer, &conversion_data.input_frames_used, (float*)buffer, &conversion_data.output_frames_gen);
				break;
			case Format::S16:
				error = speex_resampler_process_interleaved_int(conversion_state, (spx_int16_t*)internal_buffer, &conversion_data.input_frames_used, (spx_int16_t*)buffer, &conversion_data.output_frames_gen);
				break;
			default: error_message = "internal error: output_format is not convertable"; return ERROR;
			}
			
			if (error != 0) {
				error_message = speex_resampler_strerror(error);
				return ERROR;
			}
		#elif defined(HAVE_LIBSAMPLERATE)
			conversion_data.data_in = (float*)internal_buffer;
			conversion_data.data_out = (float*)buffer;
			if (pitch_handled_by_decoder) {
				conversion_data.src_ratio = (output_rate*1.0) / input_rate;
			}
			else {
				conversion_data.src_ratio = (output_rate*STANDARD_PITCH *1.0) / (input_rate*pitch*1.0);
			}
			conversion_data.end_of_input = (wrapped_decoder->IsFinished()) ? 1 : 0;

			//Now let libsamplerate filter the data
			error = src_process(conversion_state, &conversion_data);

			if (error != 0) {
				error_message = src_strerror(error);
				return ERROR;
			}
		#endif

		total_output_frames -= conversion_data.output_frames_gen;
		buffer += conversion_data.output_frames_gen*nr_of_channels*output_samplesize;

		if ((conversion_data.input_frames == 0 && conversion_data.output_frames_gen <= conversion_data.output_frames) || conversion_data.output_frames_gen == 0) {
			finished = true;
			//There is nothing left to convert - return how much samples (in bytes) are converted! 
			return length - total_output_frames*(output_samplesize*nr_of_channels);
		}
	}
	return length;
}

#endif
