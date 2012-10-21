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

#include "system.h"

#ifdef SUPPORT_JPG

#include <cstdlib>
#include <cstdio>
#include <jpeglib.h>
#include <cassert>
#include "output.h"
#include "image_jpg.h"

void ImageJPG::ReadJPG(FILE * stream, uint8_t *data, unsigned len, int& width,
						int& height, void*& pixels) {

	struct jpeg_decompress_struct cinfo;
	int res;
	int pitch;
	JSAMPARRAY buffer;
	uint8_t* dst;
	bool from_mem;

	pixels = NULL;

	from_mem = (data) ? true : false;

	jpeg_create_decompress(&cinfo);

	if (from_mem) {
		assert(data);
		jpeg_mem_src(&cinfo, data, len);
	} else {
		assert(stream);
		jpeg_stdio_src(&cinfo, stream);
	}

	res = jpeg_read_header(&cinfo, TRUE);

	if (res != JPEG_HEADER_OK) {
		Output::Warning("Not a valid JPEG image.");
		goto clean_exit;
	}

	res = jpeg_start_decompress(&cinfo);

	if (!res) {
		Output::Warning("Corrupted JPEG image.");
		goto clean_exit;
	}

	width = cinfo.output_width;
	height = cinfo.output_height;

	pitch = width * cinfo.output_components;

	buffer = (*cinfo.mem->alloc_sarray)
		((j_common_ptr) &cinfo, JPOOL_IMAGE, pitch, 1);

	pixels = malloc(pitch * height);

	dst = (uint8_t*) pixels;

	while (cinfo.output_scanline < (unsigned) height) {

		res = jpeg_read_scanlines(&cinfo, buffer, 1);
		if (!res) {
			Output::Warning("Corrupted JPEG image.");
			goto clean_exit;
		}

		memcpy(dst, buffer, pitch);
		dst += pitch;
	}

	res = jpeg_finish_decompress(&cinfo);
	if (!res) {
		Output::Warning("Corrupted JPEG image.");
		goto clean_exit;
	}

clean_exit:
	jpeg_destroy_decompress(&cinfo);
	return;
}

#endif // SUPPORT_JPG
