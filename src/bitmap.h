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

#ifndef _BITMAP_H_
#define _BITMAP_H_

// Headers
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

struct Transform;

/**
 * Opacity class.
 */
class Opacity {
public:
	int top;
	int bottom;
	int split;

	static const Opacity opaque;

	Opacity() :
		top(255), bottom(255), split(0) {}

	Opacity(int opacity) :
		top(opacity), bottom(opacity), split(0) {}

	Opacity(int top_opacity, int bottom_opacity, int opacity_split) :
		top(top_opacity), bottom(bottom_opacity), split(opacity_split) {}

	int Value() const {
		assert(!IsSplit());
		return top;
	}

	bool IsSplit() const {
		return split > 0 && top != bottom;
	}

	bool IsTransparent() const {
		return IsSplit() ? top <= 0 && bottom <= 0 : top <= 0;
	}

	bool IsOpaque() const {
		return IsSplit() ? top >= 255 && bottom >= 255 : top >= 255;
	}
};

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
	 * Loads a bitmap from an image file.
	 *
	 * @param filename image file to load.
	 * @param transparent allow transparency on bitmap.
	 * @param flags bitmap flags.
	 */
	static BitmapRef Create(const std::string& filename, bool transparent = true, uint32_t flags = 0);

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
	Bitmap(const std::string& filename, bool transparent, uint32_t flags);
	Bitmap(const uint8_t* data, unsigned bytes, bool transparent, uint32_t flags);
	Bitmap(Bitmap const& source, Rect const& src_rect, bool transparent);
	Bitmap(void *pixels, int width, int height, int pitch, const DynamicFormat& format);

	/**
	 * Destructor.
	 */
	~Bitmap();

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

	enum TileOpacity {
		// Image is full opaque and can be blitted fast
		Opaque,
		// Image has alpha and needs an alpha blit
		Partial,
		// Image is complately transparent
		Transparent
	};

	/**
	 * Provides opacity information about the image.
	 * This influences the selected operator when blitting.
	 *
	 * @return opacity information
	 */
	TileOpacity GetOpacity() const;

	/**
	 * Provides opacity information about a tile on a tilemap.
	 * This influences the selected operator when blitting a tile.
	 *
	 * @param row tile row
	 * @param col tile col
	 *
	 * @return opacity information
	 */
	TileOpacity GetTileOpacity(int row, int col) const;

	/**
	 * Writes PNG converted bitmap to output stream.
	 *
	 * @param os output stream that PNG will be output.
	 * @return true if success, otherwise false.
	 */
	bool WritePNG(std::ostream& os) const;

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

	void CheckPixels(uint32_t flags);

	/**
	 * Draws text to bitmap.
	 *
	 * @param x x coordinate where text rendering starts.
	 * @param y y coordinate where text rendering starts.
	 * @param color system color index.
	 * @param text text to draw.
	 * @param align text alignment.
	 */
	void TextDraw(int x, int y, int color, std::string const& text, Text::Alignment align = Text::AlignLeft);

	/**
	 * Draws text to bitmap.
	 *
	 * @param x x coordinate of bounding rectangle.
	 * @param y y coordinate of bounding rectangle.
	 * @param width width of bounding rectangle.
	 * @param height height of bounding rectangle.
	 * @param color system color index.
	 * @param text text to draw.
	 * @param align text alignment inside bounding rectangle.
	 */
	void TextDraw(int x, int y, int width, int height, int color, std::string const& text, Text::Alignment align = Text::AlignLeft);

	/**
	 * Draws text to bitmap.
	 *
	 * @param rect bounding rectangle.
	 * @param color system color index.
	 * @param text text to draw.
	 * @param align text alignment inside bounding rectangle.
	 */
	void TextDraw(Rect const& rect, int color, std::string const& text, Text::Alignment align = Text::AlignLeft);

	/**
	 * Draws text to bitmap.
	 *
	 * @param x x coordinate where text rendering starts.
	 * @param y y coordinate where text rendering starts.
	 * @param color text color.
	 * @param text text to draw.
	 */
	void TextDraw(int x, int y, Color color, std::string const& text);

	/**
	 * Draws text to bitmap.
	 *
	 * @param x x coordinate of bounding rectangle.
	 * @param y y coordinate of bounding rectangle.
	 * @param width width of bounding rectangle.
	 * @param height height of bounding rectangle.
	 * @param color text color.
	 * @param text text to draw.
	 * @param align text alignment inside bounding rectangle.
	 */
	void TextDraw(int x, int y, int width, int height, Color color, std::string const& text, Text::Alignment align = Text::AlignLeft);

	/**
	 * Draws text to bitmap.
	 *
	 * @param rect bounding rectangle.
	 * @param color text color.
	 * @param text text to draw.
	 * @param align text alignment inside bounding rectangle.
	 */
	void TextDraw(Rect const& rect, Color color, std::string const& text, Text::Alignment align = Text::AlignLeft);

	/**
	 * Gets text drawing font.
	 *
	 * @return text drawing font.
	 */
	FontRef const& GetFont() const;

	/**
	 * Sets text drawing font.
	 *
	 * @param font drawing font.
	 */
	void SetFont(FontRef const& font);

	/**
	 * Blits source bitmap to this one.
	 *
	 * @param x x position.
	 * @param y y position.
	 * @param src source bitmap.
	 * @param src_rect source bitmap rect.
	 * @param opacity opacity for blending with bitmap.
	 */
	void Blit(int x, int y, Bitmap const& src, Rect const& src_rect, Opacity const& opacity);

	/**
	 * Blits source bitmap to this one ignoring alpha (faster)
	 *
	 * @param x x position.
	 * @param y y position.
	 * @param src source bitmap.
	 * @param src_rect source bitmap rect.
	 * @param opacity opacity for blending with bitmap.
	 */
	void BlitFast(int x, int y, Bitmap const& src, Rect const& src_rect, Opacity const& opacity);

	/**
	 * Blits source bitmap in tiles to this one.
	 *
	 * @param src_rect source bitmap rect.
	 * @param src source bitmap.
	 * @param dst_rect destination rect.
	 * @param opacity opacity for blending with bitmap.
	 */
	void TiledBlit(Rect const& src_rect, Bitmap const& src, Rect const& dst_rect, Opacity const& opacity);

	/**
	 * Blits source bitmap in tiles to this one.
	 *
	 * @param ox tile start x offset.
	 * @param oy tile start y offset.
	 * @param src_rect source bitmap rect.
	 * @param src source bitmap.
	 * @param dst_rect destination rect.
	 * @param opacity opacity for blending with bitmap.
	 */
	void TiledBlit(int ox, int oy, Rect const& src_rect, Bitmap const& src, Rect const& dst_rect, Opacity const& opacity);

	/**
	 * Blits source bitmap stretched to this one.
	 *
	 * @param src source bitmap.
	 * @param src_rect source bitmap rect.
	 * @param opacity opacity for blending with bitmap.
	 */
	void StretchBlit(Bitmap const& src, Rect const& src_rect, Opacity const& opacity);

	/**
	 * Blits source bitmap stretched to this one.
	 *
	 * @param dst_rect destination rect.
	 * @param src source bitmap.
	 * @param src_rect source bitmap rect.
	 * @param opacity opacity for blending with bitmap.
	 */
	void StretchBlit(Rect const& dst_rect, Bitmap const& src, Rect const& src_rect, Opacity const& opacity);

	/**
	 * Blit source bitmap flipped.
	 *
	 * @param x x position.
	 * @param y y position.
	 * @param src source bitmap.
	 * @param src_rect source bitmap rect.
	 * @param horizontal flip horizontally.
	 * @param vertical flip vertically.
	 */
	void FlipBlit(int x, int y, Bitmap const& src, Rect const& src_rect, bool horizontal, bool vertical, Opacity const& opacity);

	/**
	 * Blits source bitmap scaled, rotated and translated.
	 *
	 * @param dst_rect destination rect.
	 * @param src source bitmap.
	 * @param src_rect source bitmap rect.
	 * @param inv transformation matrix from destination coordinates
	 *            to source coordinates.
	 * @param opacity opacity for blending with bitmap.
	 */
	void TransformBlit(Rect const& dst_rect, Bitmap const& src, Rect const& src_rect, const Transform& inv, Opacity const& opacity);

	/**
	 * Blits source bitmap with waver effect.
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
	 */
	void WaverBlit(int x, int y, double zoom_x, double zoom_y, Bitmap const& src, Rect const& src_rect, int depth, double phase, Opacity const& opacity);

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
	 */
	void BlendBlit(int x, int y, Bitmap const& src, Rect const& src_rect, const Color &color, Opacity const& opacity);

	/**
	 * Flips the bitmap pixels.
	 *
	 * @param dst_rect the rectangle to flip.
	 * @param horizontal flip horizontally (mirror).
	 * @param vertical flip vertically.
	 */
	void Flip(const Rect& dst_rect, bool horizontal, bool vertical);

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
	 * @param top_opacity opacity of top section.
	 * @param bottom_opacity opacity of bottom section.
	 * @param opacity_split boundary between sections,
	 *                      (zero is bottom edge).
	 * @param tone tone to apply.
	 * @param zoom_x x scale factor.
	 * @param zoom_y y scale factor.
	 * @param angle rotation angle.
	 * @param waver_depth wave magnitude.
	 * @param waver_phase wave phase.
	 */
	void EffectsBlit(int x, int y, int ox, int oy,
					 Bitmap const& src, Rect const& src_rect,
					 Opacity const& opacity,
					 double zoom_x, double zoom_y, double angle,
					 int waver_depth, double waver_phase);

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

protected:
	TileOpacity CheckOpacity(Rect const& rect);

	DynamicFormat format;

	std::vector<std::vector<TileOpacity>> tile_opacity;
	TileOpacity opacity = Partial;
	Color bg_color, sh_color;

	friend void Text::Draw(Bitmap& dest, int x, int y, int color, std::string const& text, Text::Alignment align);

#ifdef USE_SDL
	friend class SdlUi;
#endif

	/** Font for text drawing. */
	FontRef font;

	/** Bitmap data. */
	pixman_image_t *bitmap = nullptr;
	pixman_format_code_t pixman_format;

	void Init(int width, int height, void* data, int pitch = 0, bool destroy = true);
	void ConvertImage(int& width, int& height, void*& pixels, bool transparent);

	static pixman_image_t* GetSubimage(Bitmap const& src, const Rect& src_rect);
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

	typedef std::pair<int, pixman_format_code_t> format_pair;
	static std::map<int, pixman_format_code_t> formats_map;
	static bool formats_initialized;

	static void initialize_formats();
	static void add_pair(pixman_format_code_t pcode, const DynamicFormat& format);
	static pixman_format_code_t find_format(const DynamicFormat& format);

	pixman_op_t GetOperator(pixman_image_t* mask = nullptr) const;
	bool read_only = false;

private:
	/**
	 * Blits source bitmap with transformation and opacity scaling.
	 *
	 * @param fwd forward (src->dst) transformation matrix.
	 * @param src source bitmap.
	 * @param src_rect source bitmap rectangle.
	 * @param opacity opacity.
	 */
	void RotateZoomOpacityBlit(const Transform &fwd, Bitmap const& src, Rect const& src_rect,
							   Opacity const& opacity);

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
	 */
	void ZoomOpacityBlit(int x, int y, int ox, int oy,
						 Bitmap const& src, Rect const& src_rect,
						 double zoom_x, double zoom_y,
						 Opacity const& opacity);
};

#endif
