/*start of file animated_bitmap.cpp*/
#include "animated_bitmap.h"
#include "image_gif.h"
#include "output.h"

static std::vector<AnimatedBitmap*> active_animations;

void AnimationManager::Register(AnimatedBitmap* bmp) {
	active_animations.push_back(bmp);
}

void AnimationManager::Unregister(AnimatedBitmap* bmp) {
	auto it = std::find(active_animations.begin(), active_animations.end(), bmp);
	if (it != active_animations.end()) {
		active_animations.erase(it);
	}
}

void AnimationManager::UpdateAll(std::chrono::microseconds delta) {
	for (AnimatedBitmap* bmp : active_animations) {
		bmp->Update(delta);
	}
}

AnimatedBitmap::AnimatedBitmap(const uint8_t* data, unsigned bytes, bool transparent, uint32_t flags)
	: Bitmap(0, 0, true) { // Initial dummy construction
	GifDecoder decoder(data, bytes);

	if (!decoder.IsValid() || decoder.GetFrames().empty()) {
		//Output::Warning("Failed to decode GIF or GIF has no frames.");
		// Create a placeholder bitmap
		Init(16, 16, nullptr);
		Fill(Color(255, 0, 255, 255)); // Pink error color
		return;
	}

	// Re-initialize the base Bitmap with correct dimensions
	Init(decoder.GetWidth(), decoder.GetHeight(), nullptr);
	id = "animated_gif";

	for (const auto& frame : decoder.GetFrames()) {
		_frames.push_back(frame.bitmap);
		int delay_ms = frame.delay_cs * 10;
		if (delay_ms < 20) { // Enforce a minimum delay of 20ms
			delay_ms = 100; // Default to 100ms
		}
		_durations.push_back(std::chrono::milliseconds(delay_ms));
	}

	// Blit the first frame
	if (!_frames.empty()) {
		BlitFast(0, 0, *_frames[0], _frames[0]->GetRect(), Opacity::Opaque());
	}

	AnimationManager::Register(this);
}

AnimatedBitmap::~AnimatedBitmap() {
	AnimationManager::Unregister(this);
}

void AnimatedBitmap::Update(std::chrono::microseconds delta) {
	if (_frames.size() <= 1) {
		return;
	}

	_time_accumulator += delta;

	bool frame_changed = false;
	while (_time_accumulator >= _durations[_current_frame]) {
		_time_accumulator -= _durations[_current_frame];
		_current_frame = (_current_frame + 1) % _frames.size();
		frame_changed = true;
	}

	if (frame_changed) {
		BlitFast(0, 0, *_frames[_current_frame], _frames[_current_frame]->GetRect(), Opacity::Opaque());
	}
}
/*end of file animated_bitmap.cpp*/
