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

#ifdef HAVE_LIBXMP

// Headers
#include "xmp.h"
#include "audio_decoder.h"
#include "decoder_xmp.h"

static unsigned long xmp_vio_read_impl(void* ptr, unsigned long size, unsigned long count, void* userdata){
	auto* f = reinterpret_cast<Filesystem_Stream::InputStream*>(userdata);
	char* ptrc = reinterpret_cast<char*>(ptr);
	for (unsigned long i = 0; i < count; ++i) {
		f->read(reinterpret_cast<char*>(ptrc + i * size), size);
		if (f->gcount() != static_cast<std::streamsize>(size)) {
			return i;
		}
	}
	return count;
}

static int xmp_vio_seek_impl(void* userdata, long offset, int seek_type) {
	auto* f = reinterpret_cast<Filesystem_Stream::InputStream*>(userdata);
	if (f->eof()) f->clear(); //emulate behaviour of fseek

	f->seekg(offset, Filesystem_Stream::CSeekdirToCppSeekdir(seek_type));

	return 0;
}

static long xmp_vio_tell_impl(void* userdata){
	auto* f = reinterpret_cast<Filesystem_Stream::InputStream*>(userdata);
	return f->tellg();
}

struct xmp_callbacks vio = {
	xmp_vio_read_impl,
	xmp_vio_seek_impl,
	xmp_vio_tell_impl,
	nullptr
};

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

bool XMPDecoder::Open(Filesystem_Stream::InputStream stream) {
	finished = false;
	frame_bytes_left = 0;

	if (!ctx)
		return false;

	int res = xmp_load_module_from_callbacks(ctx, &stream, vio);
	if (res != 0) {
		error_message = "XMP: Error loading file";
		return false;
	}

	xmp_start_player(ctx, frequency, 0);

	int player_interpolation = 0;
	int player_effects = 0;
#if defined(PSP) || defined(__3DS__) || defined(__wii__)
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

bool XMPDecoder::Seek(std::streamoff offset, std::ios_base::seekdir origin) {
	if (!ctx)
		return false;

	if (offset == 0 && origin == std::ios_base::beg) {
		xmp_restart_module(ctx);
		frame_bytes_left = 0;
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
	frame_bytes_left = 0;
	finished = false;

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

int XMPDecoder::GetTicks() const {
	if (!ctx) {
		return 0;
	}

	xmp_frame_info info;
	xmp_get_frame_info(ctx, &info);

	return info.time / 1000;
}

bool XMPDecoder::IsModule(Filesystem_Stream::InputStream& stream) {
	int res = xmp_test_module_from_callbacks(&stream, vio, nullptr);
	stream.clear();
	stream.seekg(0, std::ios_base::beg);
	return res == 0;
}

int XMPDecoder::FillBuffer(uint8_t* buffer, int length) {
	if (!ctx)
		return -1;

	if (finished)
		return 0;

	int filled = 0;

	// xmp_play_frame renders a single frame of the module
	// xmp_play_buffer is not used here as this inserts silence at the end
	// Also gives more control about the loop handling
	while (filled < length) {
		if (frame_bytes_left == 0) {
			int ret = xmp_play_frame(ctx);

			// error
			if (ret == -XMP_ERROR_STATE)
				return -1;

			// end of module
			if (ret < 0) {
				finished = true;
				break;
			}

			xmp_frame_info info;
			xmp_get_frame_info(ctx, &info);

			// libxmp handles jump commands (built-in loops) automatically
			// When looping is enabled for the AudioDecoder manually increment
			// our own loop counter
			if (info.loop_count != GetLoopCount()) {
				if (!GetLooping()) {
					// Play the module once, then report it as finished
					finished = true;
					break;
				}

				// Manually set the loop count as IsFinished which handles this
				// normally will never return true
				loop_count = info.loop_count;
			}

			frame_buffer = reinterpret_cast<const uint8_t*>(info.buffer);
			frame_bytes_left = info.buffer_size;
		}

		int copy_size = length - filled;
		if (copy_size > frame_bytes_left)
			copy_size = frame_bytes_left;

		memcpy(buffer + filled, frame_buffer, copy_size);

		frame_buffer += copy_size;
		frame_bytes_left -= copy_size;
		filled += copy_size;
	}

	return filled;
}

#endif
