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

#ifndef _GL_BITMAP_SCREEN_H_
#define _GL_BITMAP_SCREEN_H_

#include "system.h"
#ifdef USE_OPENGL_BITMAP

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <vector>
#include <GL/gl.h>
#include "surface.h"
#include "bitmap_screen.h"

////////////////////////////////////////////////////////////
/// GlBitmapScreen class.
////////////////////////////////////////////////////////////
class GlBitmapScreen : public BitmapScreen {
public:
	GlBitmapScreen(Bitmap* source, bool delete_bitmap);
	~GlBitmapScreen();

	void BlitScreen(int x, int y);
	void BlitScreen(int x, int y, Rect src_rect);
	void BlitScreenTiled(Rect src_rect, Rect dst_rect);

	void SetBitmap(Bitmap* source, bool delete_bitmap);
	void SetBushDepthEffect(int bush_depth);
	void SetOpacityEffect(int opacity_top, int opacity_bottom);
	void SetDirty();

	static void Setup(int width, int height);
	static int PowerOfTwo(int x);

protected:
	void UploadBitmap();
	void UploadBitmap(
		GLint internal_format, int &width, int &height,
		GLenum format, GLenum type, const GLvoid *data);
	void MakeBushTex();
	void MakeGrayTex();
	void EffectsSetup(std::vector<int>& textures, int& num_units);
	void EffectsFinish(const std::vector<int>& textures, const int& num_units);

	int width, height;
	GLuint tex;
	GLuint bush_tex;
	GLuint gray_tex;
	Surface* gray_bitmap;
	bool needs_bush_refresh;
	bool needs_gray_refresh;
};

#endif

#endif
