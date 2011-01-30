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

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////

#include <zlib.h>
#include "output.h"
#include "image_xyz.h"

////////////////////////////////////////////////////////////
void ImageXYZ::ReadXYZ(const uint8* data, uint len, bool transparent,
					int& width, int& height, void*& pixels) {
	pixels = NULL;

    if (len < 8 || strncmp((char *) data, "XYZ1", 4) != 0) {
		Output::Error("Not a valid XYZ file.");
		return;
    }

    unsigned short w = data[4] + (data[5] << 8);
    unsigned short h = data[6] + (data[7] << 8);

	uLongf src_size = len - 8;
    Bytef* src_buffer = (Bytef*)&data[8];
    uLongf dst_size = 768 + (w * h);
    Bytef* dst_buffer = new Bytef[dst_size];

    int status = uncompress(dst_buffer, &dst_size, src_buffer, src_size);
	if (status != Z_OK) {
		Output::Error("Error decompressing XYZ file.");
		return;
	}
    const uint8 (*palette)[3] = (const uint8(*)[3]) dst_buffer;

	width = w;
	height = h;
	pixels = malloc(w * h * 4);

    uint8* dst = (uint8*) pixels;
    const uint8* src = (const uint8*) &dst_buffer[768];
    for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			uint8 pix = *src++;
			const uint8* color = palette[pix];
			*dst++ = color[0];
			*dst++ = color[1];
			*dst++ = color[2];
			*dst++ = (transparent && pix == 0) ? 0 : 255;
		}
    }

    delete[] dst_buffer;
}

////////////////////////////////////////////////////////////
void ImageXYZ::ReadXYZ(FILE* stream, bool transparent,
					int& width, int& height, void*& pixels) {
    fseek(stream, 0, SEEK_END);
    long size = ftell(stream);
    fseek(stream, 0, SEEK_SET);
	uint8* buffer = new uint8[size];
	fread((void*) buffer, 1, size, stream);
	ReadXYZ(buffer, (uint) size, transparent, width, height, pixels);
	delete[] buffer;
}

////////////////////////////////////////////////////////////

