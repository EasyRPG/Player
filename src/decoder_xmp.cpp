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

#ifdef HAVE_XMP

// Headers
#include <cassert>
#include "xmp.h"
#include "audio_decoder.h"
#include "output.h"
#include "decoder_xmp.h"

XMPDecoder::XMPDecoder() {
	music_type = "mod";

	ctx = xmp_create_context();
}

XMPDecoder::~XMPDecoder() {
	if (ctx) {
		xmp_end_player(ctx);
		xmp_release_module(ctx);
		xmp_free_context(ctx);
	}
}

bool XMPDecoder::Open(FILE* file) {
	finished = false;

	if (!ctx)
		return false;

	int res =  xmp_load_module_from_file(ctx, file, 0);
	if (res != 0) {
		error_message = "XMP: Error loading file";
		fclose(file);
		return false;
	}

	xmp_start_player(ctx, frequency, 0);

	int player_interpolation = 0;
	int player_effects = 0;
#if defined(PSP) || defined(_3DS) || defined(GEKKO)
	// disable filtering and use low quality interpolation
	player_interpolation = XMP_INTERP_NEAREST;
#else
	player_interpolation = XMP_INTERP_SPLINE;
	player_effects = XMP_DSP_ALL;
#endif
	xmp_set_player(ctx, XMP_PLAYER_INTERP, player_interpolation);
	xmp_set_player(ctx, XMP_PLAYER_DSP, player_effects);

	return true;
}

bool XMPDecoder::Seek(size_t offset, Origin origin) {
	if (!ctx)
		return false;

	if (offset == 0 && origin == Origin::Begin) {
		xmp_restart_module(ctx);
		finished = false;
		return true;
	}

	// TODO: maybe use xmp_set_position(ctx, int pos) or xmp_seek_time(ctx, int time)
	return false;
}

bool XMPDecoder::IsFinished() const {
	if (!ctx)
		return false;

	return finished;
}

void XMPDecoder::GetFormat(int& freq, AudioDecoder::Format& frmt, int& chans) const {
	freq = frequency;
	frmt = format;
	chans = channels;
}

bool XMPDecoder::SetFormat(int freq, AudioDecoder::Format frmt, int chans) {
	// shortcut
	if (chans == channels && frmt == format && freq == frequency)
		return true;

	// unsupported formats
	if (chans > 2 || frmt > Format::U16)
		return false;

	// restart the player, apply new format flags
	xmp_end_player(ctx);

	channels = chans;
	format = frmt;
	frequency = freq;

	int player_flags = 0;
	if (channels == 1)
		player_flags |= XMP_FORMAT_MONO;
	if (format == Format::U8 || format == Format::U16)
		player_flags |= XMP_FORMAT_UNSIGNED;
	if (format == Format::S8 || format == Format::U8)
		player_flags |= XMP_FORMAT_8BIT;

	return xmp_start_player(ctx, frequency, player_flags) == 0;
}

bool XMPDecoder::IsModule(std::string filename) {
	return xmp_test_module(const_cast<char *>(filename.c_str()), NULL) == 0;
}

int XMPDecoder::FillBuffer(uint8_t* buffer, int length) {
	if (!ctx)
		return -1;

	/* FIXME: `xmp_play_buffer()` is a loop around `xmp_play_frame()` that may add
	 * silence at the end of the buffer, when there is not enough audio data left.
	 * We may need to use the latter directly, to have no gap between two loops.
	 */
	int ret = xmp_play_buffer(ctx, buffer, length, 1);

	// end of file
	if (ret == -XMP_END)
		finished = true;

	// error
	if (ret == -XMP_ERROR_STATE)
		return -1;

	return length;
}

#endif
