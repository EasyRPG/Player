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
#ifdef USE_OPENGL_BITMAP

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <cmath>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iosfwd>
#include "gl_bitmap_screen.h"
#include "bitmap.h"

////////////////////////////////////////////////////////////
namespace GL {
	// Supported GL extensions
	bool npot_supported = false; // Non power of 2 textures supported

	// Get available GL extensions
	void InitGLExtensions();
}

#ifndef CHECK_GL_ERROR
#define CHECK_GL_ERROR()
#endif

////////////////////////////////////////////////////////////
void GL::InitGLExtensions() {
	static bool glext_inited = false;

	// Return if extensions were already checked
	if (glext_inited)
		return;

	// Get a string with all extensions
	const std::string ext_string = std::string(reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS)));
	CHECK_GL_ERROR();

	std::istringstream iss((ext_string));

	std::string token;

	// Iterate through all string tokens
	while (getline(iss, token, ' ')) {
		if (token == "GL_ARB_texture_non_power_of_two")
			npot_supported = true;
	}

	glext_inited = true;
}

////////////////////////////////////////////////////////////
GlBitmapScreen::GlBitmapScreen(Bitmap* bitmap, bool delete_bitmap) :
	BitmapScreen(bitmap, delete_bitmap) {
	glGenTextures(1, &tex);
	if (bitmap != NULL)
		UploadBitmap();
	bush_tex = 0;
	gray_tex = 0;
	gray_bitmap = NULL;
	needs_bush_refresh = true;
	needs_gray_refresh = true;
}

////////////////////////////////////////////////////////////
GlBitmapScreen::~GlBitmapScreen() {
	glDeleteTextures(1, &tex);
	if (bush_tex != 0)
		glDeleteTextures(1, &bush_tex);
	if (gray_tex != 0)
		glDeleteTextures(1, &gray_tex);
}

////////////////////////////////////////////////////////////
void GlBitmapScreen::UploadBitmap(
	GLint internal_format, int &width, int &height,
	GLenum format, GLenum type, const GLvoid *data) {

	// TODO: Maybe a better place for putting this? Some GlUi class? GlSdlUi? GlutUi?
	GL::InitGLExtensions();

	if (GL::npot_supported) {
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			internal_format,
			width, height, 0,
			format, type,
			data);
	} else {
		width = PowerOfTwo(width);
		height = PowerOfTwo(height);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			internal_format,
			width, height, 0,
			format, type,
			(void*) NULL);
		glTexSubImage2D(
			GL_TEXTURE_2D,
			0,
			0, 0,
			bitmap->GetWidth(), bitmap->GetHeight(),
			format, type,
			data);
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

////////////////////////////////////////////////////////////
void GlBitmapScreen::UploadBitmap() {
	glBindTexture(GL_TEXTURE_2D, tex);
	width = bitmap->GetWidth();
	height = bitmap->GetHeight();
	UploadBitmap(GL_RGBA, width, height, GL_BGRA, GL_UNSIGNED_BYTE, bitmap->pixels());
}

////////////////////////////////////////////////////////////
void GlBitmapScreen::MakeBushTex() {
	int h = bitmap->GetHeight();
	if (bush_effect <= 0 || bush_effect >= h)
		return;

	if (bush_tex != 0 && !needs_bush_refresh)
		return;

	if (bush_tex == 0)
		glGenTextures(1, &bush_tex);

	uint8* bush_pixels = new uint8[h];
	for (int i = 0; i < h; i++)
		bush_pixels[i] = i < h - bush_effect
			? opacity_top_effect
			: opacity_bottom_effect;

	glBindTexture(GL_TEXTURE_2D, bush_tex);
	int w = 1;
	UploadBitmap(GL_ALPHA, w, h, GL_ALPHA, GL_UNSIGNED_BYTE, bush_pixels);

	delete[] bush_pixels;

	needs_bush_refresh = false;
}

////////////////////////////////////////////////////////////
void GlBitmapScreen::MakeGrayTex() {
	if (tone_effect.gray == 0)
		return;

	if (gray_tex != 0 && !needs_gray_refresh)
		return;

	if (gray_tex == 0)
		glGenTextures(1, &gray_tex);

	if (gray_bitmap != NULL) {
		if (gray_bitmap->GetWidth() != bitmap->GetWidth() ||
			gray_bitmap->GetHeight() != bitmap->GetHeight()) {
			delete gray_bitmap;
			gray_bitmap = NULL;
		}
	}
	if (gray_bitmap == NULL)
		gray_bitmap = Surface::CreateSurface(bitmap->GetWidth(), bitmap->GetHeight(), true);

	gray_bitmap->Blit(0, 0, bitmap, bitmap->GetRect(), 255);
	gray_bitmap->ToneChange(Tone(0, 0, 0, 255));

	glBindTexture(GL_TEXTURE_2D, gray_tex);
	int w = bitmap->GetWidth();
	int h = bitmap->GetHeight();
	UploadBitmap(GL_RGBA, w, h, GL_BGRA, GL_UNSIGNED_BYTE, gray_bitmap->pixels());

	needs_gray_refresh = false;
}

////////////////////////////////////////////////////////////
void GlBitmapScreen::SetBitmap(Bitmap* source, bool delete_bitmap) {
	BitmapScreen::SetBitmap(source, delete_bitmap);
	if (bitmap != NULL)
		UploadBitmap();
	needs_bush_refresh = true;
	needs_gray_refresh = true;
}

////////////////////////////////////////////////////////////
void GlBitmapScreen::SetBushDepthEffect(int bush_depth) {
	BitmapScreen::SetBushDepthEffect(bush_depth);
	needs_bush_refresh = true;
}

void GlBitmapScreen::SetOpacityEffect(int opacity_top, int opacity_bottom) {
	BitmapScreen::SetOpacityEffect(opacity_top, opacity_bottom);
	needs_bush_refresh = true;
}

void GlBitmapScreen::SetDirty() {
	BitmapScreen::SetDirty();
	needs_gray_refresh = true;
}

////////////////////////////////////////////////////////////
void GlBitmapScreen::BlitScreen(int x, int y) {
	if (bitmap == NULL || (opacity_top_effect <= 0 &&
						   opacity_bottom_effect <= 0))
		return;

	BlitScreen(x, y, bitmap->GetRect());
}

////////////////////////////////////////////////////////////
void GlBitmapScreen::BlitScreen(int x, int y, Rect src_rect) {
	if (bitmap == NULL || (opacity_top_effect <= 0 &&
						   opacity_bottom_effect <= 0))
		return;

	std::vector<int> textures;
	int num_units;
	EffectsSetup(textures, num_units);

	float u0 = (float) src_rect.x / width;
	float v0 = (float) src_rect.y / height;
	float u1 = (float) (src_rect.x + src_rect.width ) / width;
	float v1 = (float) (src_rect.y + src_rect.height) / height;

	int nverts;

	if (waver_effect_depth > 0) {
		int nstrips = src_rect.height / 4;
		nverts = (nstrips + 1) * 2;
		GLfloat (*verts)[2][2] = new GLfloat[nstrips + 1][2][2];
		GLfloat (*texco)[2][2] = new GLfloat[nstrips + 1][2][2];
		for (int i = 0; i <= nstrips; i++) {
			double ky = 1.0 / (nstrips + 1);
			double y = src_rect.height * ky;
			double offset = waver_effect_depth * (1 + sin((waver_effect_phase + y * 20) * 3.14159 / 180));
			double kx = offset / src_rect.width;
			verts[i][0][0] = kx + 0;
			verts[i][0][1] = ky;
			verts[i][1][0] = kx + 1;
			verts[i][1][1] = ky;

			double v = v0 + (v1 - v0) * ky;
			texco[i][0][0] = u0;
			texco[i][0][1] = v;
			texco[i][1][0] = u1;
			texco[i][1][1] = v;
		}
		glVertexPointer(2, GL_FLOAT, 0, &verts[0][0][0]);
		std::vector<int>::const_iterator it;
		for (it = textures.begin(); it != textures.end(); it++) {
			glActiveTexture(*it);
			glTexCoordPointer(2, GL_FLOAT, 0, &texco[0][0][0]);
		}
		delete[] verts;
		delete[] texco;
	}
	else {
		nverts = 4;
		GLfloat verts[2][2][2] = {{{0, 0}, {0, 1}}, {{1, 0}, {1, 1}}};
		glVertexPointer(2, GL_FLOAT, 0, &verts[0][0][0]);
		GLfloat texco[2][2][2] = {{{u0, v0}, {u0, v1}}, {{u1, v0}, {u1, v1}}};
		std::vector<int>::const_iterator it;
		for (it = textures.begin(); it != textures.end(); it++) {
			glActiveTexture(*it);
			glTexCoordPointer(2, GL_FLOAT, 0, &texco[0][0][0]);
		}
	}
	glEnable(GL_VERTEX_ARRAY);
	glEnable(GL_TEXTURE_COORD_ARRAY);

	glLoadIdentity();
	glTranslated(x, y, 0);
	if (angle_effect != 0.0) {
		glTranslated(0.5, 0.5, 0);
		glRotated(angle_effect * 3.14159 / 180, 0, 0, 1);
		glTranslated(-0.5, -0.5, 0);
	}

	double zoomed_width  = bitmap->GetWidth()  * zoom_x_effect;
	double zoomed_height = bitmap->GetHeight() * zoom_y_effect;
	glScaled(zoomed_width, zoomed_height, 1);

	if (flipx_effect) {
		glTranslated(1, 0, 0);
		glScaled(-1, 0, 0);
	}
	if (flipy_effect) {
		glTranslated(0, 1, 0);
		glScaled(0, -1, 0);
	}

	glDrawArrays(GL_TRIANGLE_STRIP, 0, nverts);

	EffectsFinish(textures, num_units);
}

////////////////////////////////////////////////////////////
void GlBitmapScreen::BlitScreenTiled(Rect src_rect, Rect dst_rect) {
	if (bitmap == NULL || (opacity_top_effect <= 0 &&
						   opacity_bottom_effect <= 0))
		return;

	std::vector<int> textures;
	int num_units;
	EffectsSetup(textures, num_units);

	float u0 = (float) src_rect.x / width;
	float v0 = (float) src_rect.y / height;
	float u1 = (float) (src_rect.x + src_rect.width ) / width;
	float v1 = (float) (src_rect.y + src_rect.height) / height;

	GLfloat texco[2][2][2] = {{{u0, v0}, {u0, v1}}, {{u1, v0}, {u1, v1}}};
	glTexCoordPointer(2, GL_FLOAT, 0, &texco[0][0]);
	glEnable(GL_TEXTURE_COORD_ARRAY);

	float x0 = dst_rect.x;
	float y0 = dst_rect.y;
	float x1 = dst_rect.x + dst_rect.width;
	float y1 = dst_rect.y + dst_rect.height;
	GLfloat verts[2][2][2] = {{{x0, y0}, {x0, y1}}, {{x1, y0}, {x1, y1}}};
	glVertexPointer(2, GL_FLOAT, 0, &verts[0][0][0]);
	glEnable(GL_VERTEX_ARRAY);

	glLoadIdentity();

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	EffectsFinish(textures, num_units);
}

////////////////////////////////////////////////////////////
static void combine(int which, int rgb_source, int rgb_operand,
					int alpha_source, int alpha_operand) {
	glTexEnvi(GL_TEXTURE_ENV, which + GL_SOURCE0_RGB, rgb_source);
	glTexEnvi(GL_TEXTURE_ENV, which + GL_OPERAND0_RGB, rgb_operand);
	glTexEnvi(GL_TEXTURE_ENV, which + GL_SOURCE0_ALPHA, alpha_source);
	glTexEnvi(GL_TEXTURE_ENV, which + GL_OPERAND0_ALPHA, alpha_operand);
}

void GlBitmapScreen::EffectsSetup(std::vector<int>& textures, int& num_units) {
	num_units = 0;
	#define PREVIOUS (num_units > 0 ? GL_PREVIOUS : GL_TEXTURE)

	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tex);

	if (tone_effect.gray != 0) {
	#ifdef HARWDWARE_GRAY
		GLfloat scale[4] = {0.5, 0.5, 0.5, opacity_top_effect / 255.0};
		textures.push_back(num_units);
		glActiveTexture(GL_TEXTURE0 + num_units++);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, scale);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
		combine(0, GL_TEXTURE,  GL_SRC_COLOR, GL_TEXTURE,  GL_SRC_ALPHA);
		combine(1, GL_CONSTANT, GL_SRC_COLOR, GL_CONSTANT, GL_SRC_ALPHA);

		GLfloat offset[4] = {0.5, 0.5, 0.5, 0.0};
		glActiveTexture(GL_TEXTURE0 + num_units++);
		glEnable(GL_TEXTURE_2D);
		glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, offset);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_ADD);
		combine(0, PREVIOUS,    GL_SRC_COLOR, PREVIOUS,    GL_SRC_ALPHA);
		combine(1, GL_CONSTANT, GL_SRC_COLOR, GL_CONSTANT, GL_SRC_ALPHA);

		GLfloat third[4] = {0.66667, 0.66667, 0.66667, 0.0};
		glActiveTexture(GL_TEXTURE0 + num_units++);
		glEnable(GL_TEXTURE_2D);
		glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, third);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_DOT3_RGB);
		combine(0, PREVIOUS,    GL_SRC_COLOR, PREVIOUS,    GL_SRC_ALPHA);
		combine(1, GL_CONSTANT, GL_SRC_COLOR, GL_CONSTANT, GL_SRC_ALPHA);
	#else
		MakeGrayTex();
		textures.push_back(num_units);
		glActiveTexture(GL_TEXTURE0 + num_units++);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, gray_tex);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	#endif
		GLfloat gray[4] = {0, 0, 0, tone_effect.gray / 255.0};
		textures.push_back(num_units);
		glActiveTexture(GL_TEXTURE0 + num_units++);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, gray);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_INTERPOLATE);
		combine(0, PREVIOUS,    GL_SRC_COLOR, PREVIOUS,    GL_SRC_ALPHA);
		combine(1, GL_TEXTURE,  GL_SRC_COLOR, GL_TEXTURE,  GL_SRC_ALPHA);
		combine(2, GL_CONSTANT, GL_SRC_ALPHA, GL_CONSTANT, GL_SRC_ALPHA);
	}

	if (tone_effect.red > 0 ||
		tone_effect.green > 0 ||
		tone_effect.blue > 0) {
		GLfloat add_color[4] = {std::max(0.0, tone_effect.red   / 255.0),
								std::max(0.0, tone_effect.green / 255.0),
								std::max(0.0, tone_effect.blue  / 255.0),
								0};
		glActiveTexture(GL_TEXTURE0 + num_units++);
		glEnable(GL_TEXTURE_2D);
		glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, add_color);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_ADD);
		combine(0, PREVIOUS,    GL_SRC_COLOR, PREVIOUS,    GL_SRC_ALPHA);
		combine(1, GL_CONSTANT, GL_SRC_COLOR, GL_CONSTANT, GL_SRC_ALPHA);
	}

	if (tone_effect.red < 0 ||
		tone_effect.green < 0 ||
		tone_effect.blue < 0) {
		GLfloat sub_color[4] = {std::max(0.0, -tone_effect.red   / 255.0),
								std::max(0.0, -tone_effect.green / 255.0),
								std::max(0.0, -tone_effect.blue  / 255.0),
								0};
		glActiveTexture(GL_TEXTURE0 + num_units++);
		glEnable(GL_TEXTURE_2D);
		glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, sub_color);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_SUBTRACT);
		combine(0, PREVIOUS,    GL_SRC_COLOR, PREVIOUS,    GL_SRC_ALPHA);
		combine(1, GL_CONSTANT, GL_SRC_COLOR, GL_CONSTANT, GL_SRC_ALPHA);
	}

	int opacity = (bush_effect >= bitmap->GetHeight())
		? opacity_bottom_effect
		: opacity_top_effect;

	if (bush_effect > 0 && bush_effect < bitmap->GetHeight()
		&& opacity_top_effect != opacity_bottom_effect) {
		MakeBushTex();
		GLfloat white[4] = {1, 1, 1, 1};
		textures.push_back(num_units);
		glActiveTexture(GL_TEXTURE0 + num_units++);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, bush_tex);
		glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, white);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, PREVIOUS);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, PREVIOUS);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_CONSTANT);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA);
	}
	else if (opacity < 255) {
		GLfloat alpha[4] = {1, 1, 1, opacity / 255.0};
		glActiveTexture(GL_TEXTURE0 + num_units++);
		glEnable(GL_TEXTURE_2D);
		glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, alpha);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, PREVIOUS);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, PREVIOUS);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_CONSTANT);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_CONSTANT);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA);
	}
}

////////////////////////////////////////////////////////////
void GlBitmapScreen::EffectsFinish(const std::vector<int>& textures, const int& num_units) {
	std::vector<int>::const_iterator it;
	for (it = textures.begin(); it != textures.end(); it++) {
		glActiveTexture(*it);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	for (int i = 0; i < num_units; i++) {
		glActiveTexture(GL_TEXTURE0 + i);
		glDisable(GL_TEXTURE_2D);
	}
}

////////////////////////////////////////////////////////////
void GlBitmapScreen::Setup(int width, int height) {
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glOrtho(0, 320, 240, 0, -1, 1);
	glMatrixMode(GL_MODELVIEW);
}

////////////////////////////////////////////////////////////
int GlBitmapScreen::PowerOfTwo(int x) {
	// works for x <= 2^16, which is enough
	x--;
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x++;
	return x;
}
#endif

