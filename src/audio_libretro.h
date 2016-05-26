#ifndef _EASYRPG_AUDIOLIBRETRO_H_
#define _EASYRPG_AUDIOLIBRETRO_H_

#include "audio.h"
#ifdef USE_LIBRETRO
#include "libretro.h"
#endif

struct LibretroAudio : public AudioInterface {
	public:
	LibretroAudio();
	~LibretroAudio();

	void BGM_Play(std::string const &, int, int, int) override;
	void BGM_Pause() override;
	void BGM_Resume() override;
	void BGM_Stop() override;
	bool BGM_PlayedOnce() override;
	unsigned BGM_GetTicks() override;
	void BGM_Fade(int) override;
	void BGM_Volume(int) override;
	void BGM_Pitch(int) override;
	void SE_Play(std::string const &, int, int) override;
	void SE_Stop() override;
	void Update() override;

#ifdef USE_LIBRETRO
	public:
	static void SetRetroAudioCallback(retro_audio_sample_batch_t cb);
	private:
	/* Renders multiple audio frames in one go.
	 *
	 * One frame is defined as a sample of left and right channels, interleaved.
	 * I.e. int16_t buf[4] = { l, r, l, r }; would be 2 frames.
	 * Only one of the audio callbacks must ever be used.
	 */
	static retro_audio_sample_batch_t RenderAudioFrames;
#endif

}; // class LibretroAudio

#endif //_EASYRPG_AUDIOLIBRETRO_H_