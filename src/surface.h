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

#ifndef _EASYRPG_SURFACE_H_
#define _EASYRPG_SURFACE_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <string>
#include <list>
#include "color.h"
#include "font.h"
#include "rect.h"
#include "tone.h"
#include "bitmap.h"
#include "matrix.h"

#if defined(DINGOO)
#define wstring basic_string<wchar_t>
#endif


class BitmapUtils;

////////////////////////////////////////////////////////////
/// Base Surface class.
////////////////////////////////////////////////////////////
class Surface : public Bitmap {
public:
	////////////////////////////////////////////////////////
	/// Creates surface.
	/// @param width  : surface width
	/// @param height : surface height
	/// @param bpp : surface bpp
	/// @param transparent : allow transparency on surface
	////////////////////////////////////////////////////////
	static Surface* CreateSurface(int width, int height, bool transparent = true, int bpp = 0);

	////////////////////////////////////////////////////////
	/// Creates a copy of an existing bitmap.
	/// @param source : source bitmap
	/// @param src_rect : rect to copy from source bitmap
	/// @param transparent : allow transparency on bitmap
	////////////////////////////////////////////////////////
	static Surface* CreateSurface(Bitmap* source, Rect src_rect, bool transparent = true);

	////////////////////////////////////////////////////////
	/// Creates a surface wrapper around existing pixel data.
	/// @param pixels : pointer to pixel data
	/// @param width  : surface width
	/// @param height : surface height
	/// @param height : surface pitch
	/// @param format : pixel format
	////////////////////////////////////////////////////////
	static Surface* CreateSurfaceFrom(void *pixels, int width, int height, int pitch, const DynamicFormat& format);

	////////////////////////////////////////////////////////
	/// Destructor.
	////////////////////////////////////////////////////////
	virtual ~Surface();

	////////////////////////////////////////////////////////
	/// Blit source bitmap to this one.
	/// @param x : x position
	/// @param y : y position
	/// @param src : source bitmap
	/// @param src_rect : source bitmap rect
	/// @param opacity : opacity for blending with bitmap
	////////////////////////////////////////////////////////
	virtual void Blit(int x, int y, Bitmap* src, Rect src_rect, int opacity);

	////////////////////////////////////////////////////////
	/// Blit source bitmap in tiles to this one.
	/// @param src_rect : source bitmap rect
	/// @param src : source bitmap
	/// @param dst_rect : destination rect
	/// @param opacity : opacity for blending with bitmap
	////////////////////////////////////////////////////////
	virtual void TiledBlit(Rect src_rect, Bitmap* src, Rect dst_rect, int opacity);

	////////////////////////////////////////////////////////
	/// Blit source bitmap in tiles to this one.
	/// @param ox : tile start x offset
	/// @param ox : tile start y offset
	/// @param src_rect : source bitmap rect
	/// @param src : source bitmap
	/// @param dst_rect : destination rect
	/// @param opacity : opacity for blending with bitmap
	////////////////////////////////////////////////////////
	virtual void TiledBlit(int ox, int oy, Rect src_rect, Bitmap* src, Rect dst_rect, int opacity);

	////////////////////////////////////////////////////////
	/// Blit source bitmap stretched to this one.
	/// @param src : source bitmap
	/// @param src_rect : source bitmap rect
	/// @param opacity : opacity for blending with bitmap
	////////////////////////////////////////////////////////
	virtual void StretchBlit(Bitmap* src, Rect src_rect, int opacity);

	////////////////////////////////////////////////////////
	/// Blit source bitmap stretched to this one.
	/// @param dst_rect : destination rect
	/// @param src : source bitmap
	/// @param src_rect : source bitmap rect
	/// @param opacity : opacity for blending with bitmap
	////////////////////////////////////////////////////////
	virtual void StretchBlit(Rect dst_rect, Bitmap* src, Rect src_rect, int opacity);

	////////////////////////////////////////////////////////
	/// Blit source bitmap flipped
	/// @param x : x position
	/// @param y : y position
	/// @param src : source bitmap
	/// @param src_rect : source bitmap rect
	/// @param horizontal : flip horizontally
	/// @param vertical : flip vertically
	////////////////////////////////////////////////////////
	virtual void FlipBlit(int x, int y, Bitmap* src, Rect src_rect, bool horizontal, bool vertical);

	////////////////////////////////////////////////////////
	/// Blit source bitmap scaled, rotated and translated
	/// @param dst_rect : destination rect
	/// @param src : source bitmap
	/// @param src_rect : source bitmap rect
	/// @param inv : transformation matrix
	///  - from destination coordinates to source coordinates
	////////////////////////////////////////////////////////
	virtual void TransformBlit(Rect dst_rect, Bitmap* src, Rect src_rect, const Matrix& inv, int opacity);

	////////////////////////////////////////////////////////
	/// Blit source bitmap scaled, rotated and translated
	/// @param dst_rect : destination rectangle
	/// @param src : source bitmap
	/// @param src_rect : source bitmap rect
	/// @param angle : rotation angle (positive is clockwise)
	/// @param dst_w : scaled width
	/// @param dst_h : scaled height
	/// @param src_pos_x : source origin x
	/// @param src_pos_y : source origin y
	/// @param dst_pos_x : destination origin x
	/// @param dst_pos_y : destination origin y
	////////////////////////////////////////////////////////
	virtual void TransformBlit(Rect dst_rect,
							   Bitmap* src, Rect src_rect,
							   double angle,
							   double scale_x, double scale_y,
							   int src_pos_x, int src_pos_y,
							   int dst_pos_x, int dst_pos_y,
							   int opacity);

	////////////////////////////////////////////////////////
	/// Blit source bitmap transparency to this one.
	/// @param x : x position
	/// @param y : y position
	/// @param src : source bitmap
	/// @param src_rect : source bitmap rect
	////////////////////////////////////////////////////////
	virtual void MaskBlit(int x, int y, Bitmap* src, Rect src_rect);

	////////////////////////////////////////////////////////
	/// Blit source with waver effect.
	/// @param x : x position
	/// @param y : y position
	/// @param src : source bitmap
	/// @param src_rect : source bitmap rect
	/// @param depth : wave magnitude
	/// @param phase : wave phase
	////////////////////////////////////////////////////////
	virtual void WaverBlit(int x, int y, Bitmap* src, Rect src_rect, int depth, double phase, int opacity);

	////////////////////////////////////////////////////////
	/// Fill entire bitmap with color.
	/// @param color : color for filling
	////////////////////////////////////////////////////////
	virtual void Fill(const Color &color);

	////////////////////////////////////////////////////////
	/// Fill bitmap rect with color.
	/// @param dst_rect : destination rect
	/// @param color : color for filling
	////////////////////////////////////////////////////////
	virtual void FillRect(Rect dst_rect, const Color &color);

	////////////////////////////////////////////////////////
	/// Clears the bitmap with transparent pixels.
	////////////////////////////////////////////////////////
	virtual void Clear();

	////////////////////////////////////////////////////////
	/// Clears the bitmap rect with transparent pixels.
	/// @param dst_rect : destination rect
	////////////////////////////////////////////////////////
	virtual void ClearRect(Rect dst_rect);
	
	////////////////////////////////////////////////////////
	/// Rotate bitmap hue.
	/// @param x : x position
	/// @param y : y position
	/// @param src : source bitmap
	/// @param src_rect : source bitmap rect
	/// @param hue : hue change, degrees
	////////////////////////////////////////////////////////
	virtual void HueChangeBlit(int x, int y, Bitmap* src, Rect src_rect, double hue);

	////////////////////////////////////////////////////////
	/// Adjust bitmap HSL colors.
	/// @param x : x position
	/// @param y : y position
	/// @param src : source bitmap
	/// @param src_rect : source bitmap rect
	/// @param hue : hue change, degrees
	/// @param sat : saturation scale
	/// @param lum : luminance scale
	/// @param loff: luminance offset
	////////////////////////////////////////////////////////
	virtual void HSLBlit(int x, int y, Bitmap* src, Rect src_rect, double h, double s, double l, double lo);

	////////////////////////////////////////////////////////
	/// Adjust bitmap tone.
	/// @param x : x position
	/// @param y : y position
	/// @param src : source bitmap
	/// @param src_rect : source bitmap rect
	/// @param tone : tone to apply
	////////////////////////////////////////////////////////
	virtual void ToneBlit(int x, int y, Bitmap* src, Rect src_rect, const Tone &tone);

	////////////////////////////////////////////////////////
	/// Change the opacity of a bitmap.
	/// @param x : x position
	/// @param y : y position
	/// @param src : source bitmap
	/// @param src_rect : source bitmap rect
	/// @param opacity : the maximum opacity
	////////////////////////////////////////////////////////
	virtual void OpacityBlit(int x, int y, Bitmap* src, Rect src_rect, int opacity);

	////////////////////////////////////////////////////////
	/// Flips the bitmap pixels.
	/// @param dst_rect : the rectangle to flip
	/// @param horizontal : flip horizontally (mirror)
	/// @param vertical : flip vertically
	////////////////////////////////////////////////////////
	virtual void Flip(const Rect& dst_rect, bool horizontal, bool vertical);

	////////////////////////////////////////////////////////
	/// Blit source bitmap scaled 2:1, with no transparency
	/// @param dst_rect : destination rectangle
	/// @param src : source bitmap
	/// @param src_rect : source bitmap rectangle
	////////////////////////////////////////////////////////
	virtual void Blit2x(Rect dst_rect, Bitmap* src, Rect src_rect);

	////////////////////////////////////////////////////////
	/// Calculate the bounding rectangle of a transformed rectangle
	/// @param m    : transformation matrix
	/// @param rect : source rectangle
	/// @return : the bounding rectangle
	////////////////////////////////////////////////////////
	static Rect TransformRectangle(const Matrix& m, const Rect& rect);

	/// Multiple Effects functions
	/// Note: these are in effects.cpp, not surface.cpp
	/// Note: all perform rendering using existing functions,
	///  so it is not necessary for back-ends to implement them

	////////////////////////////////////////////////////////
	/// Blit source bitmap with effects
	/// @param x : destination x position
	/// @param y : destination y position
	/// @param src : source bitmap
	/// @param src_rect : source bitmap rectangle
	/// @param top_opacity : opacity of top section
	/// @param bottom_opacity : opacity of bottom section
	/// @param opacity_split : boundary between sections,
	///  (zero is bottom edge)
	/// @param tone : tone to apply
	/// @param zoom_x : x scale factor
	/// @param zoom_y : y scale factor
	/// @param angle : rotation angle
	/// @param waver_depth : wave magnitude
	/// @param waver_phase : wave phase
	/// Note: rotation and waver are mutually exclusive
	////////////////////////////////////////////////////////
	virtual void EffectsBlit(int x, int y, Bitmap* src, Rect src_rect,
							 int top_opacity, int bottom_opacity, int opacity_split,
							 const Tone& tone,
							 double zoom_x, double zoom_y, double angle,
							 int waver_depth, double waver_phase);

	////////////////////////////////////////////////////////
	/// Blit source bitmap with transformation and opacity scaling
	/// @param fwd : forward (src->dst) transformation matrix
	/// @param src : source bitmap
	/// @param src_rect : source bitmap rectangle
	/// @param opacity : opacity
	////////////////////////////////////////////////////////
	virtual void EffectsBlit(const Matrix &fwd, Bitmap* src, Rect src_rect,
							 int opacity);

	////////////////////////////////////////////////////////
	/// Blit source bitmap with transformation and (split) opacity scaling
	/// @param fwd : forward (src->dst) transformation matrix
	/// @param src : source bitmap
	/// @param src_rect : source bitmap rectangle
	/// @param top_opacity : opacity of top section
	/// @param bottom_opacity : opacity of bottom section
	/// @param opacity_split : boundary between sections,
	///  (zero is bottom edge)
	////////////////////////////////////////////////////////
	virtual void EffectsBlit(const Matrix &fwd, Bitmap* src, Rect src_rect,
							 int top_opacity, int bottom_opacity, int opacity_split);

	////////////////////////////////////////////////////////
	/// Blit source bitmap with scaling, waver and (split) opacity scaling
	/// @param src : source bitmap
	/// @param src_rect : source bitmap rectangle
	/// @param top_opacity : opacity of top section
	/// @param bottom_opacity : opacity of bottom section
	/// @param opacity_split : boundary between sections,
	///  (zero is bottom edge)
	/// @param zoom_x : x scale factor
	/// @param zoom_y : y scale factor
	/// @param waver_depth : wave magnitude
	/// @param waver_phase : wave phase
	////////////////////////////////////////////////////////
	virtual void EffectsBlit(int x, int y, Bitmap* src, Rect src_rect,
							 int top_opacity, int bottom_opacity, int opacity_split,
							 double zoom_x, double zoom_y,
							 int waver_depth, double waver_phase);

	////////////////////////////////////////////////////////
	/// Blit source bitmap with waver and (split) opacity scaling
	/// @param src : source bitmap
	/// @param src_rect : source bitmap rectangle
	/// @param top_opacity : opacity of top section
	/// @param bottom_opacity : opacity of bottom section
	/// @param opacity_split : boundary between sections,
	///  (zero is bottom edge)
	/// @param waver_depth : wave magnitude
	/// @param waver_phase : wave phase
	////////////////////////////////////////////////////////
	virtual void EffectsBlit(int x, int y, Bitmap* src, Rect src_rect,
							 int top_opacity, int bottom_opacity, int opacity_split,
							 int waver_depth, double waver_phase);

	////////////////////////////////////////////////////////
	/// Blit source bitmap with waver and opacity scaling
	/// @param src : source bitmap
	/// @param src_rect : source bitmap rectangle
	/// @param opacity : opacity
	/// @param waver_depth : wave magnitude
	/// @param waver_phase : wave phase
	////////////////////////////////////////////////////////
	virtual void EffectsBlit(int x, int y, Bitmap* src, Rect src_rect,
							 int opacity,
							 int waver_depth, double waver_phase);

	/// TextDraw alignment options
	enum TextAlignment {
		TextAlignLeft,
		TextAlignCenter,
		TextAlignRight
	};

	////////////////////////////////////////////////////////
	/// Draws text to bitmap.
	/// @param x : x coordinate where text rendering starts
	/// @param y : y coordinate where text rendering starts
	/// @param color : text color
	/// @param text : text to draw
	/// @param align : text alignment
	////////////////////////////////////////////////////////
	virtual void TextDraw(int x, int y, int color, std::string text, TextAlignment align = Surface::TextAlignLeft);
	virtual void TextDraw(int x, int y, int color, std::wstring text, TextAlignment align = Surface::TextAlignLeft);

	////////////////////////////////////////////////////////
	/// Draws text to bitmap.
	/// @param x : x coordinate of bounding rectangle
	/// @param y : y coordinate of bounding rectangle
	/// @param width : width of bounding rectangle
	/// @param height : height of bounding rectangle
	/// @param color : text color
	/// @param text : text to draw
	/// @param align : text alignment inside bounding rectangle
	////////////////////////////////////////////////////////
	virtual void TextDraw(int x, int y, int width, int height, int color, std::string text, TextAlignment align = Surface::TextAlignLeft);
	virtual void TextDraw(int x, int y, int width, int height, int color, std::wstring text, TextAlignment align = Surface::TextAlignLeft);

	////////////////////////////////////////////////////////
	/// Draws text to bitmap.
	/// @param rect : bounding rectangle
	/// @param color : text color
	/// @param text : text to draw
	/// @param align : text alignment inside bounding rectangle
	////////////////////////////////////////////////////////
	virtual void TextDraw(Rect rect, int color, std::string text, TextAlignment align = Surface::TextAlignLeft);
	virtual void TextDraw(Rect rect, int color, std::wstring text, TextAlignment align = Surface::TextAlignLeft);

	////////////////////////////////////////////////////////
	/// Get space needed to draw some text.
	/// This assumes that every char has a size of 6x12.
	/// @param text : text to draw
	////////////////////////////////////////////////////////
	static Rect GetTextSize(const std::string& text);
	/// Wide string version
	static Rect GetTextSize(const std::wstring& text);

	/// @return text drawing font
	virtual Font* GetFont() const;

	/// @param text drawing font
	virtual void SetFont(Font* font);

protected:
	friend class Text;
	friend class FTFont;
	friend class BitmapUtils;
	template <class T1, class T2> friend class BitmapUtilsT;

#ifdef USE_SDL
	friend class SdlBitmap;
	friend class SdlUi;
#endif

	Surface();

	/// Font for text drawing.
	Font* font;

	virtual BitmapUtils* Begin();
	virtual BitmapUtils* Begin(Bitmap* src);
	virtual void End();
	virtual void End(Bitmap* src);
	virtual void RefreshCallback();

	////////////////////////////////////////////////////////
	/// Get a pixel color.
	/// @param x : pixel x
	/// @param y : pixel y
	/// @param color : pixel color
	////////////////////////////////////////////////////////
	virtual void SetPixel(int x, int y, const Color &color);

	bool editing;
};

#endif
