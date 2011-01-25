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
#ifdef USE_SDL_BITMAP

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <cmath>
#include "sdl_bitmap_screen.h"
#include "sdl_bitmap.h"
#include "sdl_ui.h"
#include "util_macro.h"

////////////////////////////////////////////////////////////
#ifdef USE_ALPHA
	#define SETALPHA_FLAGS SDL_SRCALPHA
#else
	#ifdef USE_RLE
		#define SETALPHA_FLAGS SDL_SRCALPHA | SDL_RLEACCEL
	#else
		#define SETALPHA_FLAGS SDL_SRCALPHA
	#endif
#endif

////////////////////////////////////////////////////////////
SdlBitmapScreen::SdlBitmapScreen(Bitmap* bitmap, bool delete_bitmap) :
	BitmapScreen(bitmap, delete_bitmap),
	bitmap_effects(NULL) {}

////////////////////////////////////////////////////////////
SdlBitmapScreen::~SdlBitmapScreen() {
	if (bitmap_effects != NULL)
		delete bitmap_effects;
}

////////////////////////////////////////////////////////////
void SdlBitmapScreen::SetBitmap(Bitmap* source, bool _delete_bitmap) {
	if (bitmap_effects != NULL) {
		delete bitmap_effects;
		bitmap_effects = NULL;
	}

	if (delete_bitmap && bitmap != NULL) {
		delete bitmap;
		bitmap = NULL;
	} else if (bitmap != NULL)
		bitmap->DetachBitmapScreen(this);

	delete_bitmap = _delete_bitmap;
	bitmap = source;
	needs_refresh = true;

	if (bitmap) {
		bitmap->AttachBitmapScreen(this);

		src_rect_effect = Rect(0, 0, bitmap->width(), bitmap->height());
	} else {
		src_rect_effect = Rect();
	}
}

////////////////////////////////////////////////////////////
void SdlBitmapScreen::BlitScreen(int x, int y) {
	// FIXME: This method needs more comments.
	if (bitmap == NULL || opacity_top_effect <= 0) return;

	Refresh();

	if (bitmap == NULL) return;

	x -= origin_x;
	y -= origin_y;

	SDL_Surface* surface = bitmap_effects != NULL
		? ((SdlBitmap*)bitmap_effects)->bitmap
		: ((SdlBitmap*)bitmap)->bitmap;

	if (bush_effect < surface->h) {
		if (!src_rect_effect_applied) {
			Rect src_rect = src_rect_effect;
			src_rect.height -= bush_effect;

			BlitScreenIntern(surface, x, y, src_rect, opacity_top_effect);
		} else {
			Rect src_rect(0, 0, surface->w, surface->h - bush_effect);

			BlitScreenIntern(surface, x, y, src_rect, opacity_top_effect);
		}
	}

	if (bush_effect > 0) {
		if (!src_rect_effect_applied) {
			Rect src_rect(
				src_rect_effect.x,
				src_rect_effect.y + src_rect_effect.height - bush_effect,
				src_rect_effect.width,
				bush_effect
			);

			BlitScreenIntern(surface, x, y + bush_effect, src_rect, opacity_bottom_effect);
		} else {
			Rect src_rect(0, surface->h - bush_effect, surface->w, bush_effect);

			BlitScreenIntern(surface, x, y + bush_effect, src_rect, opacity_bottom_effect);
		}
	}
}

////////////////////////////////////////////////////////////
void SdlBitmapScreen::BlitScreen(int x, int y, Rect src_rect) {
	// FIXME: This method needs more comments.
	if (bitmap == NULL || opacity_top_effect <= 0) return;

	Refresh();

	x -= origin_x;
	y -= origin_y;

	SDL_Surface* surface = bitmap_effects != NULL
		? ((SdlBitmap*)bitmap_effects)->bitmap
		: ((SdlBitmap*)bitmap)->bitmap;

	if (bush_effect < surface->h) {
		if (!src_rect_effect_applied) {
			Rect blit_rect = src_rect_effect.GetSubRect(src_rect);
			blit_rect.y	-= bush_effect;

			if (!blit_rect.IsOutOfBounds(src_rect_effect))
				BlitScreenIntern(surface, x, y, blit_rect, opacity_top_effect);
		} else {
			Rect blit_rect = src_rect;
			blit_rect.y	-= bush_effect;

			BlitScreenIntern(surface, x, y, blit_rect, opacity_top_effect);
		}
	}

	if (bush_effect > 0) {
		if (!src_rect_effect_applied) {
			Rect blit_rect = src_rect_effect.GetSubRect(src_rect);
			blit_rect.y += src_rect_effect.height - bush_effect;
			blit_rect.height = bush_effect;

			if (!blit_rect.IsOutOfBounds(src_rect_effect))
				BlitScreenIntern(surface, x, y, blit_rect, opacity_bottom_effect);
		} else {
			Rect blit_rect(
				src_rect.x,
				src_rect.y + src_rect.height - bush_effect,
				src_rect.width,
				bush_effect
			);

			BlitScreenIntern(surface, x, y + bush_effect, blit_rect, opacity_bottom_effect);
		}
	}
}

////////////////////////////////////////////////////////////
void SdlBitmapScreen::BlitScreenTiled(Rect src_rect, Rect dst_rect) {
	if (bitmap == NULL || opacity_top_effect <= 0) return;

	Refresh();

	SDL_Surface* surface = bitmap_effects != NULL
		? ((SdlBitmap*)bitmap_effects)->bitmap
		: ((SdlBitmap*)bitmap)->bitmap;

	int y_blits = 1;
	if (src_rect.height < surface->h && src_rect.height != 0) {
		y_blits = (int)ceil((float)surface->h / (float)src_rect.height);
	}

	int x_blits = 1;
	if (src_rect.width < surface->w && src_rect.width != 0) {
		x_blits = (int)ceil((float)surface->w / (float)src_rect.width);
	}

	for (int j = 0; j < y_blits; j++) {
		for (int i = 0; i < x_blits; i++) {
			BlitScreenIntern(surface, i * src_rect.width, j * src_rect.height, src_rect, opacity_top_effect);
		}
	}
}

////////////////////////////////////////////////////////////
void SdlBitmapScreen::BlitScreenIntern(SDL_Surface* surface, int x, int y, Rect src_rect, int opacity) {
	// FIXME: This method needs more comments.
	#ifdef USE_ALPHA
		if (opacity == 255) {
			SDL_Rect src_r = {src_rect.x, src_rect.y, src_rect.width, src_rect.height};
			SDL_Rect dst_r = {x, y, 0, 0};

			SDL_Surface* display = ((SdlBitmap*) DisplaySdlUi->GetDisplaySurface())->bitmap;
			SDL_BlitSurface(surface, &src_r, display, &dst_r);
		} else {
			src_rect.Adjust(surface->w, surface->h);
			if (src_rect.IsOutOfBounds(surface->w, surface->h) )return;

			SDL_Surface* display = ((SdlBitmap*) DisplaySdlUi->GetDisplaySurface())->bitmap;

			int bpp = display->format->BytesPerPixel;

			if SDL_MUSTLOCK(display) SDL_LockSurface(display);
			if SDL_MUSTLOCK(surface) SDL_LockSurface(surface);

			if (bpp == 2) {
				const uint16* src_pixel = ((uint16*)surface->pixels) + src_rect.x + src_rect.y * surface->pitch / bpp;
				uint16* dst_pixel = ((uint16*)display->pixels) + x + y * display->pitch / bpp;

				/*for (int i = 0; i < src_rect.height; i++) {
					for (int j = 0; j < src_rect.width; j++) {
						uint8 srca = src_pixel[3] * opacity / 255;
						dst_pixel[0] = (dst_pixel[0] * (255 - srca) + src_pixel[0] * srca) / 255;
						dst_pixel[1] = (dst_pixel[1] * (255 - srca) + src_pixel[1] * srca) / 255;
						dst_pixel[2] = (dst_pixel[2] * (255 - srca) + src_pixel[2] * srca) / 255;

						src_pixel += 1;
						dst_pixel += 1;
					}
					src_pixel += surface->pitch / bpp - src_rect.width;
					dst_pixel += display->pitch / bpp - src_rect.width;
				}*/
			} else if (bpp == 4) {
				const uint8* src_pixel = (uint8*)surface->pixels + src_rect.x * bpp + src_rect.y * surface->pitch;
				uint8* dst_pixel = (uint8*)display->pixels + x * bpp + y * display->pitch;

				for (int i = 0; i < src_rect.height; i++) {
					for (int j = 0; j < src_rect.width; j++) {
						uint8 srca = src_pixel[3] * opacity / 255;
						dst_pixel[0] = (dst_pixel[0] * (255 - srca) + src_pixel[0] * srca) / 255;
						dst_pixel[1] = (dst_pixel[1] * (255 - srca) + src_pixel[1] * srca) / 255;
						dst_pixel[2] = (dst_pixel[2] * (255 - srca) + src_pixel[2] * srca) / 255;

						src_pixel += bpp;
						dst_pixel += bpp;
					}
					src_pixel += surface->pitch - src_rect.width * bpp;
					dst_pixel += display->pitch - src_rect.width * bpp;
				}
			}

			if SDL_MUSTLOCK(display) SDL_UnlockSurface(display);
			if SDL_MUSTLOCK(surface) SDL_UnlockSurface(surface);
		}
	#else
		SDL_Rect src_r = {(int16)src_rect.x, (int16)src_rect.y, (uint16)src_rect.width, (uint16)src_rect.height};
		SDL_Rect dst_r = {(int16)x, (int16)y, 0, 0};

		if (opacity < 255) SDL_SetAlpha(surface, SETALPHA_FLAGS, (uint8)opacity);

		SDL_Surface* display = ((SdlBitmap*) DisplaySdlUi->GetDisplaySurface())->bitmap;
		SDL_BlitSurface(surface, &src_r, display, &dst_r);

		if (opacity < 255) SDL_SetAlpha(surface, SETALPHA_FLAGS, 255);
	#endif
}

////////////////////////////////////////////////////////////
void SdlBitmapScreen::Refresh() {
	// FIXME: This method needs more comments.
	if (!needs_refresh) return;

	needs_refresh = false;
	origin_x = 0;
	origin_y = 0;

	if (bitmap == NULL)
		return;

	src_rect_effect.Adjust(bitmap->GetWidth(), bitmap->GetHeight());

	if (src_rect_effect.IsOutOfBounds(bitmap->GetWidth(), bitmap->GetHeight()))
		return;

	if (bitmap_effects != NULL) {
		delete bitmap_effects;
		bitmap_effects = NULL;
	}

	if (tone_effect == Tone() && angle_effect == 0.0 &&
		flipx_effect == false && flipy_effect == false &&
		zoom_x_effect == 1.0 && zoom_y_effect == 1.0 &&
		waver_effect_depth == 0) {

		src_rect_effect_applied = false;

	} else {
		int new_width = src_rect_effect.width;
		int new_height = src_rect_effect.height;

		CalcZoomedSize(new_width, new_height);

		int zoomed_width = new_width;
		int zoomed_height = new_height;

		CalcRotatedSize(new_width, new_height);

		if (new_width > 0 && new_height > 0) {
			Surface* surf_effects = Surface::CreateSurface(bitmap, src_rect_effect, bitmap->GetTransparent());

			surf_effects->ToneChange(tone_effect);
			surf_effects->Flip(flipx_effect, flipy_effect);

			bitmap_effects = surf_effects;

			if (angle_effect != 0.0) {
				Bitmap* temp = bitmap_effects->RotateScale(
					angle_effect * 3.14159 / 180, zoomed_width, zoomed_height);
				origin_x = (temp->width() - zoomed_width) / 2;
				origin_y = (temp->height() - zoomed_height) / 2;
				delete bitmap_effects;
				bitmap_effects = temp;
			}
			else if (zoom_x_effect != 1.0 || zoom_y_effect != 1.0) {
				Bitmap* temp = bitmap_effects->Resample(zoomed_width, zoomed_height, bitmap_effects->GetRect());
				delete bitmap_effects;
				bitmap_effects = temp;
			}
		}

		if (waver_effect_depth != 0) {
			Bitmap* temp = bitmap_effects->Waver(waver_effect_depth, waver_effect_phase);
			delete bitmap_effects;
			bitmap_effects = temp;
		}

		src_rect_effect_applied = true;
	}
}

////////////////////////////////////////////////////////////
void SdlBitmapScreen::CalcRotatedSize(int &width, int &height) {
	if (angle_effect != 0.0) {
		double radians=(2 * 3.1416 * angle_effect) / 360;

		double cosine = (double)cos(radians);
		double sine = (double)sin(radians);

		double p1x = -height * sine;
		double p1y = height * cosine;
		double p2x = width * cosine - height * sine;
		double p2y = height * cosine + width * sine;
		double p3x = width * cosine;
		double p3y = width * sine;

		double minx = min(0.0, min(p1x, min(p2x, p3x)));
		double miny = min(0.0, min(p1y, min(p2y, p3y)));
		double maxx = max(p1x, max(p2x, p3x));
		double maxy = max(p1y, max(p2y, p3y));

		width = (int)ceil(fabs(maxx) - minx);
		height = (int)ceil(fabs(maxy) - miny);
	}
}

////////////////////////////////////////////////////////////
void SdlBitmapScreen::CalcZoomedSize(int &width, int &height) {
	width = (int)(width * zoom_x_effect);
	height = (int)(height * zoom_y_effect);
}

////////////////////////////////////////////////////////////
void SdlBitmapScreen::ClearEffects() {
	BitmapScreen::ClearEffects();
	src_rect_effect_applied = false;
}

void SdlBitmapScreen::SetSrcRect(Rect src_rect) {
	if (src_rect_effect != src_rect) {
		src_rect_effect = src_rect;

		if (tone_effect == Tone() || angle_effect == 0.0 ||
			flipx_effect == false || flipy_effect == false ||
			zoom_x_effect == 1.0 || zoom_y_effect == 1.0 ||
			waver_effect_depth == 0 || src_rect_effect_applied) {
				needs_refresh = true;
		}
	}
}

////////////////////////////////////////////////////////////
#endif

