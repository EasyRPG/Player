/*start of file animated_bitmap.h*/
#pragma once
#include "bitmap.h"
#include <vector>
#include <chrono>

class AnimatedBitmap;

class AnimationManager {
public:
	static void Register(AnimatedBitmap* bmp);
	static void Unregister(AnimatedBitmap* bmp);
	static void UpdateAll(std::chrono::microseconds delta);
};

class AnimatedBitmap : public Bitmap {
public:
	AnimatedBitmap(const uint8_t* data, unsigned bytes, bool transparent, uint32_t flags);
	~AnimatedBitmap() override;

	void Update(std::chrono::microseconds delta);

	// Add this public method
	int GetCurrentFrame() const { return _current_frame; }

private:
	std::vector<BitmapRef> _frames;
	std::vector<std::chrono::microseconds> _durations;
	int _current_frame = 0;
	std::chrono::microseconds _time_accumulator{ 0 };
};
/*end of file animated_bitmap.h*/
