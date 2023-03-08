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

#ifndef EP_BITMAP_H
#define EP_BITMAP_H

// Headers
#include <cstdint>
#include <string>
#include <map>
#include <vector>
#include <cassert>
#include <pixman.h>

#include "system.h"
#include "color.h"
#include "rect.h"
#include "pixel_format.h"
#include "tone.h"
#include "text.h"
#include "pixman_image_ptr.h"
#include "opacity.h"
#include "filesystem_stream.h"
#include "string_view.h"

struct Transform;

/**
 * Base Bitmap class.
 */
class Bitmap {
public:
	/**
	 * Creates bitmap with empty surface.
	 *
	 * @param width surface width.
	 * @param height surface height.
	 * @param color color for filling.
	 */
	static BitmapRef Create(int width, int height, const Color& color);

	/**
	 * Loads a bitmap from a stream.
	 *
	 * @param stream stream to read image from.
	 * @param transparent allow transparency on bitmap.
	 * @param flags bitmap flags.
	 */
	static BitmapRef Create(Filesystem_Stream::InputStream stream, bool transparent = true, uint32_t flags = 0);

	/*
	 * Loads a bitmap from memory.
	 *
	 * @param data image data.
	 * @param bytes size of data.
	 * @param transparent allow transparency on bitmap.
	 * @param flags bitmap flags.
	 */
	static BitmapRef Create(const uint8_t* data, unsigned bytes, bool transparent = true, uint32_t flags = 0);

	/**
	 * Creates a bitmap from another.
	 *
	 * @param source source bitmap.
	 * @param src_rect rect to copy from source bitmap.
	 * @param transparent allow transparency on bitmap.
	 */
	static BitmapRef Create(Bitmap const& source, Rect const& src_rect, bool transparent = true);

	/**
	 * Creates a surface.
	 *
	 * @param width surface width.
	 * @param height surface height.
	 * @param bpp surface bpp.
	 * @param transparent allow transparency on surface.
	 */
	static BitmapRef Create(int width, int height, bool transparent = true, int bpp = 0);

	/**
	 * Creates a surface wrapper around existing pixel data.
	 *
	 * @param pixels pointer to pixel data.
	 * @param width surface width.
	 * @param height surface height.
	 * @param pitch surface pitch.
	 * @param format pixel format.
	 */
	static BitmapRef Create(void *pixels, int width, int height, int pitch, const DynamicFormat& format);

	Bitmap(int width, int height, bool transparent);
	Bitmap(Filesystem_Stream::InputStream stream, bool transparent, uint32_t flags);
	Bitmap(const uint8_t* data, unsigned bytes, bool transparent, uint32_t flags);
	Bitmap(Bitmap const& source, Rect const& src_rect, bool transparent);
	Bitmap(void *pixels, int width, int height, int pitch, const DynamicFormat& format);

	/**
	 * Gets the bitmap width.
	 *
	 * @return the bitmap width.
	 */
	int GetWidth() const;

	/**
	 * Gets the bitmap height.
	 *
	 * @return the bitmap height.
	 */
	int GetHeight() const;

	/**
	 * Gets bitmap bounds rect.
	 *
	 * @return bitmap bounds rect.
	 */
	Rect GetRect() const;

	/**
	 * Gets how many bytes the bitmap consumes.
	 *
	 * @return bitmap size in bytes
	 */
	size_t GetSize() const;

	/**
	 * Gets if bitmap allows transparency.
	 *
	 * @return if bitmap allows transparency.
	 */
	bool GetTransparent() const;

	enum Flags {
		// Special handling for system graphic.
		Flag_System = 1 << 1,
		// Special handling for chipset graphic.
		// Generates a tile opacity list.
		Flag_Chipset = 1 << 2,
		// Bitmap will not be written to. This allows blit optimisations because the
		// opacity information will not change.
		Flag_ReadOnly = 1 << 16
	};

	enum class BlendMode {
		Default, // SRC or OVER depending on the image
		Normal, // OP_OVER
		NormalWithoutAlpha, // OP_SRC
		XOR,
		Additive,
		Multiply,
		Overlay,
		Saturate,
		Darken,
		Lighten,
		ColorDodge,
		ColorBurn,
		Difference,
		Exclusion,
		SoftLight,
		HardLight
	};

	/**
	 * Provides opacity information about the image.
	 * This influences the selected operator when blitting.
	 *
	 * @return opacity information
	 */
	ImageOpacity GetImageOpacity() const;

	/**
	 * Provides opacity information about a tile on a tilemap.
	 * This influences the selected operator when blitting a tile.
	 *
	 * @param x tile x coordinate
	 * @param y tile y coordinate
	 *
	 * @return opacity information
	 */
	ImageOpacity GetTileOpacity(int x, int y) const;

	/**
	 * Writes PNG converted bitmap to output stream.
	 *
	 * @param os output stream that PNG will be output.
	 * @return true if success, otherwise false.
	 */
	bool WritePNG(Filesystem_Stream::OutputStream&) const;

	/**
	 * Gets the background color
	 * Bitmap must have been loaded with the Bitmap::System flag
	 *
	 * @return background color.
	 */
	Color GetBackgroundColor() const;

	/**
	 * Gets the shadow color
	 * Bitmap must have been loaded with the Bitmap::System flag
	 *
	 * @return shadow color.
	 */
	Color GetShadowColor() const;

	/**
	 * Gets the filename this bitmap was loaded from.
	 * This will be empty when the origin was not a file.
	 *
	 * @return filename
	 */
	StringView GetFilename() const;

	void CheckPixels(uint32_t flags);

	/**
	 * @param x x-coordinate
	 * @param y y-coordinate
	 * @return color at the pixel location
	 */
	Color GetColorAt(int x, int y) const;

	/**
	 * Draws text to bitmap using the Font::Default() font.
	 *
	 * @param x x coordinate where text rendering starts.
	 * @param y y coordinate where text rendering starts.
	 * @param color system color index.
	 * @param text text to draw.
	 * @param align text alignment.
	 * @return Where to draw the next glyph
	 */
	Point TextDraw(int x, int y, int color, StringView text, Text::Alignment align = Text::AlignLeft);

	/**
	 * Draws text to bitmap using the Font::Default() font.
	 *
	 * @param rect bounding rectangle.
	 * @param color system color index.
	 * @param text text to draw.
	 * @param align text alignment inside bounding rectangle.
	 * @return Where to draw the next glyph
	 */
	Point TextDraw(Rect const& rect, int color, StringView text, Text::Alignment align = Text::AlignLeft);

	/**
	 * Draws text to bitmap using the Font::Default() font.
	 *
	 * @param x x coordinate where text rendering starts.
	 * @param y y coordinate where text rendering starts.
	 * @param color text color.
	 * @param text text to draw.
	 * @return Where to draw the next glyph
	 */
	Point TextDraw(int x, int y, Color color, StringView text);

	/**
	 * Draws text to bitmap using the Font::Default() font.
	 *
	 * @param rect bounding rectangle.
	 * @param color text color.
	 * @param text text to draw.
	 * @param align text alignment inside bounding rectangle.
	 */
	Point TextDraw(Rect const& rect, Color color, StringView, Text::Alignment align = Text::AlignLeft);

	/**
	 * Blits source bitmap to this one.
	 *
	 * @param x x position.
	 * @param y y position.
	 * @param src source bitmap.
	 * @param src_rect source bitmap rect.
	 * @param opacity opacity for blending with bitmap.
	 * @param blend_mode Blend mode to use.
	 */
	void Blit(int x, int y, Bitmap const& src, Rect const& src_rect,
		Opacity const& opacity, BlendMode blend_mode = BlendMode::Default);

	/**
	 * Blits source bitmap to this one ignoring alpha (faster)
	 *
	 * @param x x position.
	 * @param y y position.
	 * @param src source bitmap.
	 * @param src_rect source bitmap rect.
	 * @param opacity opacity for blending with bitmap.
	 */
	void BlitFast(int x, int y, Bitmap const& src, Rect const& src_rect,
		Opacity const& opacity);

	/**
	 * Blits source bitmap in tiles to this one.
	 *
	 * @param src_rect source bitmap rect.
	 * @param src source bitmap.
	 * @param dst_rect destination rect.
	 * @param opacity opacity for blending with bitmap.
	 * @param blend_mode Blend mode to use.
	 */
	void TiledBlit(Rect const& src_rect, Bitmap const& src, Rect const& dst_rect,
		Opacity const& opacity, BlendMode blend_mode = BlendMode::Default);

	/**
	 * Blits source bitmap in tiles to this one.
	 *
	 * @param ox tile start x offset.
	 * @param oy tile start y offset.
	 * @param src_rect source bitmap rect.
	 * @param src source bitmap.
	 * @param dst_rect destination rect.
	 * @param opacity opacity for blending with bitmap.
	 * @param blend_mode Blend mode to use.
	 */
	void TiledBlit(int ox, int oy, Rect const& src_rect, Bitmap const& src, Rect const& dst_rect,
		Opacity const& opacity, BlendMode blend_mode = BlendMode::Default);

	/**
	 * Blits source bitmap to this one, making clones across the edges if src crossed a boundary of this.
	 *
	 * @param x x position.
	 * @param y y position.
	 * @param src source bitmap.
	 * @param src_rect source bitmap rect.
	 * @param mirror_x Blit a clone in x direction
	 * @param mirror_Y BLit a clone in y direction
	 * @param opacity opacity for blending with bitmap.
	 */
	void EdgeMirrorBlit(int x, int y, Bitmap const& src, Rect const& src_rect, bool mirror_x, bool mirror_y, Opacity const& opacity);

	/**
	 * Blits source bitmap stretched to this one.
	 *
	 * @param src source bitmap.
	 * @param src_rect source bitmap rect.
	 * @param opacity opacity for blending with bitmap.
	 * @param blend_mode Blend mode to use.
	 */
	void StretchBlit(Bitmap const& src, Rect const& src_rect,
		Opacity const& opacity, BlendMode blend_mode = BlendMode::Default);

	/**
	 * Blits source bitmap stretched to this one.
	 *
	 * @param dst_rect destination rect.
	 * @param src source bitmap.
	 * @param src_rect source bitmap rect.
	 * @param opacity opacity for blending with bitmap.
	 * @param blend_mode Blend mode to use.
	 */
	void StretchBlit(Rect const& dst_rect, Bitmap const& src, Rect const& src_rect,
		Opacity const& opacity, BlendMode blend_mode = BlendMode::Default);

	/**
	 * Blit source bitmap flipped.
	 *
	 * @param x x position.
	 * @param y y position.
	 * @param src source bitmap.
	 * @param src_rect source bitmap rect.
	 * @param horizontal flip horizontally.
	 * @param vertical flip vertically.
	 * @param opacity opacity to apply.
	 * @param blend_mode Blend mode to use.
	 */
	void FlipBlit(int x, int y, Bitmap const& src, Rect const& src_rect, bool horizontal, bool vertical,
		Opacity const& opacity, BlendMode blend_mode = BlendMode::Default);

	/**
	 * Blits source bitmap with waver, zoom, and opacity effects.
	 *
	 * @param x x position.
	 * @param y y position.
	 * @param zoom_x x scale factor.
	 * @param zoom_y y scale factor.
	 * @param src source bitmap.
	 * @param src_rect source bitmap rect.
	 * @param depth wave magnitude.
	 * @param phase wave phase.
	 * @param opacity opacity.
	 * @param blend_mode Blend mode to use.
	 */
	void WaverBlit(int x, int y, double zoom_x, double zoom_y, Bitmap const& src, Rect const& src_rect, int depth, double phase,
		Opacity const& opacity, BlendMode blend_mode = BlendMode::Default);

	/**
	 * Blits source bitmap with rotation, zoom, and opacity effects.
	 *
	 * @param x x position.
	 * @param y y position.
	 * @param ox source origin x.
	 * @param oy source origin y.
	 * @param src source bitmap.
	 * @param src_rect source bitmap rect.
	 * @param angle rotation angle in radians.
	 * @param zoom_x x scale factor.
	 * @param zoom_y y scale factor.
	 * @param opacity opacity.
	 * @param blend_mode Blend mode to use.
	 */
	void RotateZoomOpacityBlit(int x, int y, int ox, int oy,
		Bitmap const& src, Rect const& src_rect,
		double angle, double zoom_x, double zoom_y,
		Opacity const& opacity, BlendMode blend_mode = BlendMode::Normal);

	/**
	 * Blits source bitmap with zoom and opacity scaling.
	 *
	 * @param x x position.
	 * @param y y position.
	 * @param ox source origin x.
	 * @param oy source origin y.
	 * @param src source bitmap.
	 * @param src_rect source bitmap rectangle.
	 * @param zoom_x x scale factor.
	 * @param zoom_y y scale factor.
	 * @param opacity opacity.
	 * @param blend_mode Blend mode to use.
	 */
	void ZoomOpacityBlit(int x, int y, int ox, int oy,
		Bitmap const& src, Rect const& src_rect,
		double zoom_x, double zoom_y,
		Opacity const& opacity, BlendMode blend_mode = BlendMode::Default);

	/**
	 * Fills entire bitmap with color.
	 *
	 * @param color color for filling.
	 */
	void Fill(const Color &color);

	/**
	 * Fills bitmap rect with color.
	 *
	 * @param dst_rect destination rect.
	 * @param color color for filling.
	 */
	void FillRect(Rect const& dst_rect, const Color &color);

	/**
	 * Clears the bitmap with transparent pixels.
	 */
	void Clear();

	/**
	 * Clears the bitmap rect with transparent pixels.
	 *
	 * @param dst_rect destination rect.
	 */
	void ClearRect(Rect const& dst_rect);

	/**
	 * Rotates bitmap hue.
	 *
	 * @param x x position.
	 * @param y y position.
	 * @param src source bitmap.
	 * @param src_rect source bitmap rect.
	 * @param hue hue change, degrees.
	 */
	void HueChangeBlit(int x, int y, Bitmap const& src, Rect const& src_rect, double hue);

	/**
	 * Adjusts bitmap tone.
	 *
	 * @param x x position.
	 * @param y y position.
	 * @param src source bitmap.
	 * @param src_rect source bitmap rect.
	 * @param tone tone to apply.
	 * @param opacity opacity to apply.
	 */
	void ToneBlit(int x, int y, Bitmap const& src, Rect const& src_rect, const Tone &tone, Opacity const& opacity);

	/**
	 * Blends bitmap with color.
	 *
	 * @param x x position.
	 * @param y y position.
	 * @param src source bitmap.
	 * @param src_rect source bitmap rect.
	 * @param color color to apply.
	 * @param opacity opacity to apply.
	 */
	void BlendBlit(int x, int y, Bitmap const& src, Rect const& src_rect, const Color &color, Opacity const& opacity);

	/**
	 * Flips the bitmap pixels.
	 *
	 * @param horizontal flip horizontally (mirror).
	 * @param vertical flip vertically.
	 */
	void Flip(bool horizontal, bool vertical);

	/**
	 * Blits source bitmap to this one through a mask bitmap.
	 *
	 * @param dst_rect destination rectangle.
	 * @param mask mask bitmap
	 * @param mx mask x position
	 * @param my mask y position
	 * @param src source bitmap.
	 * @param sx source x position
	 * @param sy source y position
	 */
	void MaskedBlit(Rect const& dst_rect, Bitmap const& mask, int mx, int my, Bitmap const& src, int sx, int sy);

	/**
	 * Blits constant color to this one through a mask bitmap.
	 *
	 * @param dst_rect destination rectangle.
	 * @param mask mask bitmap
	 * @param mx mask x position
	 * @param my mask y position
	 * @param color source color.
	 */
	void MaskedBlit(Rect const& dst_rect, Bitmap const& mask, int mx, int my, Color const& color);

	/**
	 * Blits source bitmap scaled 2:1, with no transparency.
	 *
	 * @param dst_rect destination rectangle.
	 * @param src source bitmap.
	 * @param src_rect source bitmap rectangle.
	 */
	void Blit2x(Rect const& dst_rect, Bitmap const& src, Rect const& src_rect);

	/**
	 * Calculates the bounding rectangle of a transformed rectangle.
	 *
	 * @param m transformation matrix.
	 * @param rect source rectangle.
	 * @return the bounding rectangle.
	 */
	static Rect TransformRectangle(const Transform& m, const Rect& rect);

	/**
	 * Blits source bitmap with effects.
	 * Note: rotation and waver are mutually exclusive.
	 *
	 * @param x destination x position.
	 * @param y destination y position.
	 * @param ox source origin x.
	 * @param oy source origin y.
	 * @param src source bitmap.
	 * @param src_rect source bitmap rectangle.
	 * @param opacity opacity to apply.
	 * @param zoom_x x scale factor.
	 * @param zoom_y y scale factor.
	 * @param angle rotation angle.
	 * @param waver_depth wave magnitude.
	 * @param waver_phase wave phase.
	 * @param blend_mode Blend mode to use.
	 */
	void EffectsBlit(int x, int y, int ox, int oy,
		Bitmap const& src, Rect const& src_rect,
		Opacity const& opacity,
		double zoom_x, double zoom_y, double angle,
		int waver_depth, double waver_phase,
		BlendMode blend_mode = BlendMode::Default);

	static DynamicFormat ChooseFormat(const DynamicFormat& format);
	static void SetFormat(const DynamicFormat& format);

	static DynamicFormat pixel_format;
	static DynamicFormat opaque_pixel_format;
	static DynamicFormat image_format;
	static DynamicFormat opaque_image_format;

	void* pixels();
	void const* pixels() const;
	int width() const;
	int height() const;
	int bpp() const;
	int pitch() const;

	ImageOpacity ComputeImageOpacity() const;
	ImageOpacity ComputeImageOpacity(Rect rect) const;

protected:
	DynamicFormat format;

	ImageOpacity image_opacity = ImageOpacity::Alpha_8Bit;
	TileOpacity tile_opacity;
	Color bg_color, sh_color;

	std::string filename;

	/** Bitmap data. */
	PixmanImagePtr bitmap;
	pixman_format_code_t pixman_format;

	void Init(int width, int height, void* data, int pitch = 0, bool destroy = true);
	void ConvertImage(int& width, int& height, void*& pixels, bool transparent);

	static PixmanImagePtr GetSubimage(Bitmap const& src, const Rect& src_rect);
	static inline void MultiplyAlpha(uint8_t &r, uint8_t &g, uint8_t &b, const uint8_t &a) {
		r = (uint8_t)((int)r * a / 0xFF);
		g = (uint8_t)((int)g * a / 0xFF);
		b = (uint8_t)((int)b * a / 0xFF);
	}
	static inline void DivideAlpha(uint8_t &r, uint8_t &g, uint8_t &b, const uint8_t &a) {
		if (a == 0)
			r = g = b = 0;
		else {
			r = (uint8_t)((int)r * 0xFF / a);
			g = (uint8_t)((int)g * 0xFF / a);
			b = (uint8_t)((int)b * 0xFF / a);
		}
	}

	static pixman_format_code_t find_format(const DynamicFormat& format);

	/*
	 * Determines the fastest operator for the operation.
	 * When a blend_mode is specified the blend mode is used.
	 *
	 * @param mask Image mask
	 * @param blend_mode When >= 0: Force this blend mode as operator
	 * @return blend mode
	 */
	pixman_op_t GetOperator(pixman_image_t* mask = nullptr, BlendMode blend_mode = BlendMode::Default) const;
	bool read_only = false;
};

inline ImageOpacity Bitmap::GetImageOpacity() const {
	return image_opacity;
}

inline ImageOpacity Bitmap::GetTileOpacity(int x, int y) const {
	return tile_opacity.Get(x, y);
}

inline Color Bitmap::GetBackgroundColor() const {
	return bg_color;
}

inline Color Bitmap::GetShadowColor() const {
	return sh_color;
}

inline int Bitmap::GetWidth() const {
	return width();
}

inline int Bitmap::GetHeight() const {
	return height();
}

inline Rect Bitmap::GetRect() const {
	return Rect(0, 0, width(), height());
}

inline bool Bitmap::GetTransparent() const {
	return format.alpha_type != PF::NoAlpha;
}

inline StringView Bitmap::GetFilename() const {
	return filename;
}

#endif
