/*start of file image_gif.h*/
#pragma once
#include <vector>
#include <cstdint>
#include "bitmap.h"
#include "nsgif.h" // No more extern "C" wrapper here

struct GifFrame {
	BitmapRef bitmap;
	int delay_cs; // Delay in centiseconds
};

class GifDecoder {
public:
	GifDecoder(const uint8_t* data, size_t len);
	~GifDecoder();

	bool IsValid() const { return _valid; }
	int GetWidth() const { return _width; }
	int GetHeight() const { return _height; }
	const std::vector<GifFrame>& GetFrames() const { return _frames; }

private:
	static nsgif_bitmap_t* cb_bitmap_create(int width, int height);
	static void cb_bitmap_destroy(nsgif_bitmap_t* bitmap);
	static uint8_t* cb_bitmap_get_buffer(nsgif_bitmap_t* bitmap);

	bool _valid = false;
	int _width = 0;
	int _height = 0;
	std::vector<GifFrame> _frames;
	nsgif_t* _gif = nullptr;
};
/*end of file image_gif.h*/
