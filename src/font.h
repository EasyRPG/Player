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

#ifndef EP_FONT_H
#define EP_FONT_H

// Headers
#include "filesystem_stream.h"
#include "point.h"
#include "system.h"
#include "memory_management.h"
#include "rect.h"
#include "string_view.h"
#include <string>
#include <lcf/scope_guard.h>

class Color;
class Rect;

/**
 * Font class.
 *
 * All methods in this class are private API.
 * Only use it when low level access is required.
 * Use the API in Text instead.
 *
 * @see Text
 */
class Font {
 public:
	/** Contains bitmap and metrics of a glyph */
	struct GlyphRet {
		/** bitmap which the glyph pixels are located within */
		BitmapRef bitmap;
		/**
		 * How far to advance the x/y offset after drawing for the next glyph.
		 * y value is only relevant for vertical layouts.
		 */
		Point advance;
		/** x/y position in the buffer where the glyph is rendered at */
		Point offset;
		/** When enabled the glyph is colored and not masked with the system graphic */
		bool has_color = false;
	};

	/** Contains metrics of a glyph shaped by Harfbuzz */
	struct ShapeRet {
		/** Codepoint of this glyph after shaping */
		char32_t code;
		/**
		 * How far to advance the x/y offset after drawing for the next glyph.
		 * y value is only relevant for vertical layouts.
		 */
		Point advance;
		/** x/y position in the buffer where the glyph is rendered at */
		Point offset;
		/**
		 * When true the glyph was not found.
		 * In that case code contains the original codepoint usable for a fallback.
		 */
		bool not_found;
	};

	/** Contains style informations */
	struct Style {
		/** Size in pixel to render at. -1 will use the size specified during initialisation */
		int size = -1;
		/** Whether to render text in bold (currently unsupported) */
		bool bold = false;
		/** Whether to render text in italic (currently unsupported) */
		bool italic = false;
		/** Whether to render a drop shadow */
		bool draw_shadow = true;
		/** Whether to draw the system color using a gradient */
		bool draw_gradient = true;
		/** When draw_gradient is false specifies the pixel of the current system color to use */
		Point color_offset = {};
		/**
		 * Specifies how far the drawing cursor is advanced in x direction after rendering a glyph.
		 * This will yield incorrect results for anything that involves complex shaping.
		 **/
		int letter_spacing = 0;
	};

	virtual ~Font() = default;

	/**
	 * Determines the size of a bitmap required to render a single character.
	 * The dimensions of the Rect describe a bounding box to fit the text.
	 *
	 * @param glyph the glyph to measure.
	 * @see Text::GetSize
	 * @return Rect describing the rendered string boundary
	 */
	Rect GetSize(char32_t glyph) const;

	/**
	 * Renders the glyph onto bitmap at the given position with system graphic and color
	 *
	 * @param dest the bitmap to render to
	 * @param x X offset to render glyph
	 * @param y Y offset to render glyph
	 * @param sys system graphic to use
	 * @param color which color in the system graphic
	 * @param glyph which utf32 glyph to render
	 *
	 * @return Point containing how far to advance in x/y direction.
	 */
	Point Render(Bitmap& dest, int x, int y, const Bitmap& sys, int color, char32_t glyph) const;

	/**
	 * Renders the glyph onto bitmap at the given position with system graphic and color.
	 * For glyph positioning pre-calculated shaping data can be specified.
	 *
	 * @param dest the bitmap to render to
	 * @param x X offset to render glyph
	 * @param y Y offset to render glyph
	 * @param sys system graphic to use
	 * @param color which color in the system graphic
	 * @param shape shaping information for the glyph
	 *
	 * @return Rect containing the x offset, y offset, width, and height of the subrect that was blitted onto dest. Not including text shadow!
	 */
	Point Render(Bitmap& dest, int x, int y, const Bitmap& sys, int color, const ShapeRet& shape) const;

	/**
	 * Renders the glyph onto bitmap at the given position with system graphic and color
	 *
	 * @param dest the bitmap to render to
	 * @param x X offset to render glyph
	 * @param y Y offset to render glyph
	 * @param color which color in the system graphic
	 * @param glyph which utf32 glyph to render
	 *
	 * @return Point containing how far to advance in x/y direction.
	 */
	Point Render(Bitmap& dest, int x, int y, Color const& color, char32_t glyph) const;

	/**
	 * Determines if the used font supports shaping.
	 * This will only return true when the font uses FreeType and HarfBuzz is enabled.
	 *
	 * @return Whether shaping is supported.
	 */
	bool CanShape() const;

	/**
	 * Shapes the passed text and returns new codepoints and positioning information.
	 * This method will abort when shaping is not supported.
	 *
	 * @see CanShape()
	 * @param text Text to shape
	 * @return Shaping information. See Font::ShapeRet
	 */
	std::vector<ShapeRet> Shape(U32StringView text) const;

	/**
	 * Defines a fallback font that shall be used when a glyph is not found in the current font.
	 * Currently only used by FreeType Fonts.
	 *
	 * @param fallback_font Font to fallback to
	 */
	void SetFallbackFont(FontRef fallback_font);

	using StyleScopeGuard = lcf::ScopeGuard<std::function<void()>>;

	/**
	 * @return Whether a custom style is currently active
	 */
	bool IsStyleApplied() const;

	/**
	 * Returns the current font style used for rendering.
	 *
	 * @return current style
	 */
	Style GetCurrentStyle() const;

	/**
	 * Applies a new text style for rendering.
	 * The style is reverted to the original style afterwards through the returned scope guard.
	 *
	 * @param new_style new style to apply
	 * @return StyleScopeGuard When destroyed, reverts to the old style
	 */
	StyleScopeGuard ApplyStyle(Style new_style);

	/**
	 * Uses the FreeType library to load a font from the provided stream.
	 *
	 * @param is Stream to read from
	 * @param size Font size (height) in px
	 * @param bold Configure for bold rendering. This option is ignored.
	 * @param italic Configure for italic rendering. This option is ignored.
	 * @return font handle or nullptr on failure or if FreeType is unavailable
	 */
	static FontRef CreateFtFont(Filesystem_Stream::InputStream is, int size, bool bold, bool italic);
	static FontRef Default();
	static FontRef Default(bool use_mincho);
	static FontRef DefaultBitmapFont();
	static FontRef DefaultBitmapFont(bool use_mincho);
	static void SetDefault(FontRef new_default, bool use_mincho);
	static void ResetDefault();
	static void Dispose();

	static FontRef exfont;

	enum SystemColor {
		ColorShadow = -1,
		ColorDefault = 0,
		ColorDisabled = 3,
		ColorCritical = 4,
		ColorKnockout = 5,
		ColorHeal = 9
	};

	virtual Rect vGetSize(char32_t glyph) const = 0;
	virtual GlyphRet vRender(char32_t glyph) const = 0;
	virtual GlyphRet vRenderShaped(char32_t glyph) const { return vRender(glyph); };
	virtual bool vCanShape() const { return false; }
	virtual std::vector<ShapeRet> vShape(U32StringView) const { return {}; }
	virtual void vApplyStyle(const Style& style) { (void)style; };

 protected:
	Font(StringView name, int size, bool bold, bool italic);

	std::string name;
	bool style_applied = false;
	Style original_style;
	Style current_style;
	FontRef fallback_font;
};

#endif
