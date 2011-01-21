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
#include <cmath>
#include "bitmap_screen.h"
#include "util_macro.h"

#if defined(USE_SDL_BITMAP)
	#include "sdl_bitmap_screen.h"
#elif defined(USE_OPENGL)
	#include "gl_bitmap_screen.h"
#elif defined(USE_SOFT_BITMAP)
	#include "soft_bitmap_screen.h"
#elif defined(USE_PIXMAN_BITMAP)
	#include "pixman_bitmap_screen.h"
#else
	#error "No bitmap implementation selected"
#endif

////////////////////////////////////////////////////////////
BitmapScreen* BitmapScreen::CreateBitmapScreen(Bitmap* source) {
	#if defined(USE_SDL_BITMAP)
		return (BitmapScreen*)new SdlBitmapScreen(source);
	#elif defined(USE_OPENGL)
		return (BitmapScreen*)new GlBitmapScreen(source);
	#elif defined(USE_SOFT_BITMAP)
		return (BitmapScreen*)new SoftBitmapScreen(source);
	#elif defined(USE_PIXMAN_BITMAP)
		return (BitmapScreen*)new PixmanBitmapScreen(source);
	#else
		#error "No bitmap implementation selected"
	#endif
}

////////////////////////////////////////////////////////////
BitmapScreen* BitmapScreen::CreateBitmapScreen(bool delete_bitmap) {
	#if defined(USE_SDL_BITMAP)
		return (BitmapScreen*)new SdlBitmapScreen(delete_bitmap);
	#elif defined(USE_OPENGL)
		return (BitmapScreen*)new GlBitmapScreen(delete_bitmap);
	#elif defined(USE_SOFT_BITMAP)
		return (BitmapScreen*)new SoftBitmapScreen(delete_bitmap);
	#elif defined(USE_PIXMAN_BITMAP)
		return (BitmapScreen*)new PixmanBitmapScreen(delete_bitmap);
	#else
		#error "No bitmap implementation selected"
	#endif
}

////////////////////////////////////////////////////////////
BitmapScreen::BitmapScreen(Bitmap* bitmap) :
	bitmap(bitmap),
	delete_bitmap(bitmap != NULL) {

	ClearEffects();

	if (bitmap != NULL) {
		src_rect_effect = bitmap->GetRect();
		bitmap->AttachBitmapScreen(this);
	}
}

////////////////////////////////////////////////////////////
BitmapScreen::BitmapScreen(bool delete_bitmap) :
	bitmap(NULL),
	delete_bitmap(delete_bitmap) {

	ClearEffects();
}

////////////////////////////////////////////////////////////
BitmapScreen::~BitmapScreen() {
	if (delete_bitmap && bitmap != NULL) {
		delete bitmap;
	} else if (bitmap != NULL) {
		bitmap->DetachBitmapScreen(this);
	}
}

////////////////////////////////////////////////////////////
void BitmapScreen::SetDirty() {
	needs_refresh = true;
}

////////////////////////////////////////////////////////////
void BitmapScreen::SetBitmap(Bitmap* source) {
	if (delete_bitmap && bitmap != NULL) {
		delete bitmap;
	} else if (bitmap != NULL)
		bitmap->DetachBitmapScreen(this);

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
Bitmap* BitmapScreen::GetBitmap() {
	return bitmap;
}

////////////////////////////////////////////////////////////
void BitmapScreen::ClearEffects() {
	needs_refresh = true;

	opacity_top_effect = 255;
	opacity_bottom_effect = 128;
	bush_effect = 0;
	tone_effect = Tone();
	src_rect_effect = Rect(0, 0, 0, 0);
	flipx_effect = false;
	flipy_effect = false;
	zoom_x_effect = 1.0;
	zoom_y_effect = 1.0;
	angle_effect = 0.0;
}

void BitmapScreen::SetFlashEffect(const Color &color, int duration) {
	// TODO
}

void BitmapScreen::UpdateFlashEffect(int frame) {
	// TODO
}

void BitmapScreen::SetSrcRect(Rect src_rect) {
	if (src_rect_effect != src_rect) {
		src_rect_effect = src_rect;
		needs_refresh = true;
	}
}

void BitmapScreen::SetOpacityEffect(int opacity_top, int opacity_bottom) {
	if (opacity_top_effect != opacity_top) {
		opacity_top_effect = opacity_top;
		needs_refresh = true;
	}
	if (opacity_bottom_effect == -1)
		opacity_bottom_effect = (opacity_top_effect + 1) / 2;
	if (opacity_bottom_effect != opacity_bottom) {
		opacity_bottom_effect = opacity_bottom;
		needs_refresh = true;
	}
}

void BitmapScreen::SetBushDepthEffect(int bush_depth) {
	if (bush_effect != bush_depth) {
		bush_effect = bush_depth;
		needs_refresh = true;
	}
}

void BitmapScreen::SetToneEffect(Tone tone) {
	if (tone_effect != tone) {
		tone_effect = tone;
		needs_refresh = true;
	}
}

void BitmapScreen::SetFlipXEffect(bool flipx) {
	if (flipx_effect != flipx) {
		flipx_effect = flipx;
		needs_refresh = true;
	}
}

void BitmapScreen::SetFlipYEffect(bool flipy) {
	if (flipy_effect != flipy) {
		flipy_effect = flipy;
		needs_refresh = true;
	}
}

void BitmapScreen::SetZoomXEffect(double zoom_x) {
	if (zoom_x_effect != zoom_x) {
		zoom_x_effect = zoom_x;
		needs_refresh = true;
	}
}

void BitmapScreen::SetZoomYEffect(double zoom_y) {
	if (zoom_y_effect != zoom_y) {
		zoom_y_effect = zoom_y;
		needs_refresh = true;
	}
}

void BitmapScreen::SetAngleEffect(double angle) {
	if (angle_effect != angle) {
		angle_effect = angle;
		needs_refresh = true;
	}
}

void BitmapScreen::SetBlendType(int blend_type) {
	blend_type_effect = blend_type;
}

void BitmapScreen::SetBlendColor(Color blend_color) {
	blend_color_effect = blend_color;
}

Rect BitmapScreen::GetSrcRect() const {
	return src_rect_effect;
}

int BitmapScreen::GetOpacityEffect(int which) const {
	return which > 0 ? opacity_bottom_effect : opacity_top_effect;
}

int BitmapScreen::GetBushDepthEffect() const {
	return bush_effect;
}

Tone BitmapScreen::GetToneEffect() const {
	return tone_effect;
}

bool BitmapScreen::GetFlipXEffect() const {
	return flipx_effect;
}

bool BitmapScreen::GetFlipYEffect() const {
	return flipy_effect;
}

double BitmapScreen::GetZoomXEffect() const {
	return zoom_x_effect;
}

double BitmapScreen::GetZoomYEffect() const {
	return zoom_y_effect;
}

double BitmapScreen::GetAngleEffect() const {
	return angle_effect;
}

int BitmapScreen::GetBlendType() const {
	return blend_type_effect;
}

Color BitmapScreen::GetBlendColor() const {
	return blend_color_effect;
}
