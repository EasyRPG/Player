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

#include "image_gif.h"
#include "output.h"

// This struct is used temporarily by the nsgif callbacks.
struct gif_bitmap {
	BitmapRef bmp;
};

nsgif_bitmap_t* GifDecoder::cb_bitmap_create(int width, int height) {
	auto* bitmap = new gif_bitmap();
	bitmap->bmp = Bitmap::Create(width, height, true);
	return reinterpret_cast<nsgif_bitmap_t*>(bitmap);
}

void GifDecoder::cb_bitmap_destroy(nsgif_bitmap_t* bitmap) {
	delete reinterpret_cast<gif_bitmap*>(bitmap);
}

uint8_t* GifDecoder::cb_bitmap_get_buffer(nsgif_bitmap_t* bitmap) {
	return reinterpret_cast<uint8_t*>(reinterpret_cast<gif_bitmap*>(bitmap)->bmp->pixels());
}

GifDecoder::GifDecoder(const uint8_t* data, size_t len) {
	nsgif_bitmap_cb_vt callbacks;
	callbacks.create = cb_bitmap_create;
	callbacks.destroy = cb_bitmap_destroy;
	callbacks.get_buffer = cb_bitmap_get_buffer;
	// Set other optional callbacks to NULL if not used
	callbacks.set_opaque = nullptr;
	callbacks.test_opaque = nullptr;
	callbacks.modified = nullptr;
	callbacks.get_rowspan = nullptr;

	if (nsgif_create(&callbacks, NSGIF_BITMAP_FMT_ARGB8888, &_gif) != NSGIF_OK) {
		Output::Warning("Could not create GIF decoder context.");
		return;
	}

	if (nsgif_data_scan(_gif, len, data) != NSGIF_OK) {
		Output::Warning("Error scanning GIF data.");
		nsgif_destroy(_gif);
		_gif = nullptr;
		return;
	}
	nsgif_data_complete(_gif);

	const nsgif_info_t* info = nsgif_get_info(_gif);
	_width = info->width;
	_height = info->height;

	// Decode all frames
	for (uint32_t i = 0; i < info->frame_count; ++i) {
		nsgif_bitmap_t* decoded_bmp_ptr = nullptr;
		const nsgif_frame_info_t* frame_info = nsgif_get_frame_info(_gif, i);

		if (nsgif_frame_decode(_gif, i, &decoded_bmp_ptr) != NSGIF_OK) {
			Output::Warning("Error decoding GIF frame %d.", i);
			continue;
		}

		// nsgif reuses its internal buffer. We must copy the frame data.
		BitmapRef source_frame_bitmap = reinterpret_cast<gif_bitmap*>(decoded_bmp_ptr)->bmp;
		BitmapRef frame_copy = Bitmap::Create(*source_frame_bitmap, source_frame_bitmap->GetRect(), true);

		// If the frame has no explicit transparency, check if we should apply it manually.
		if (!frame_info->transparency) {
			uint32_t palette[NSGIF_MAX_COLOURS];
			size_t entries;

			// Get the appropriate palette (local or global)
			if (!nsgif_local_palette(_gif, i, palette, &entries)) {
				nsgif_global_palette(_gif, palette, &entries);
			}

			if (entries > 0) {
				// The color at index 0 is our target for transparency.
				// nsgif outputs fully opaque pixels, so we need to match the opaque version of the color.
				uint32_t transparent_color = palette[0] | 0xFF000000; // Force alpha to full for comparison

				uint32_t* pixels = reinterpret_cast<uint32_t*>(frame_copy->pixels());
				int pixel_count = frame_copy->GetWidth() * frame_copy->GetHeight();

				for (int p = 0; p < pixel_count; ++p) {
					if (pixels[p] == transparent_color) {
						pixels[p] = 0; // Set to transparent black
					}
				}
			}
		}

		_frames.push_back({ frame_copy, (int)frame_info->delay });
	}

	_valid = true;
}

GifDecoder::~GifDecoder() {
	if (_gif) {
		nsgif_destroy(_gif);
	}
}
