/*
    SPriG - SDL Primitive Generator
    by Jonathan Dearborn
    
    Based on SGE: SDL Graphics Extension r030809
    by Anders Lindström
*/


/*********************************************************************
 *  This library is free software; you can redistribute it and/or    *
 *  modify it under the terms of the GNU Library General Public      *
 *  License as published by the Free Software Foundation; either     *
 *  version 2 of the License, or (at your option) any later version. *
 *********************************************************************/


#ifndef _SPRIG_H__
#define _SPRIG_H__


#include "SDL.h"

#define SPG_VER 1  // Check this for MISSING functionality
#define SPG_VER_MINOR 0  // Check this for ADDED functionality
#define SPG_VER_BUGFIX 0

#define SPG_C_AND_CPP  // undef this if you want to force C under a C++ compiler
//#define SPG_USE_EXTENDED  // Build Sprig with this to add some extra functions
#define SPG_DEFINE_PI  // Allow defines of pi variations
#define SPG_MAX_ERRORS 40  // Max size of error message stack
#define SPG_USE_FAST_MATH // undef this to use math.h's sqrt()




/*
*  C compatibility
*  Thanks to Ohbayashi Ippei (ohai@kmc.gr.jp) for this clever hack!
*/
#ifdef SPG_C_AND_CPP
	#ifdef __cplusplus
		#define SPG_CPP           /* use extern "C" on base functions */
	#else
		#define SPG_C_ONLY       /* remove overloaded functions */
	#endif
#else
    #define SPG_C_ONLY
#endif

//PI_8, PI_4, PI_2, PI3_4, PI, PI5_4, PI3_2, PI7_4, PI2
#ifdef SPG_DEFINE_PI
    #ifndef PI_8
        #define PI_8   0.392699082f
    #endif
    #ifndef PI_4
        #define PI_4   0.785398163f
    #endif
    #ifndef PI_2
        #define PI_2   1.57079633f
    #endif
    #ifndef PI3_4
        #define PI3_4  2.35619449f
    #endif
    #ifndef PI
        #define PI     3.14159265f
    #endif
    #ifndef PI5_4
        #define PI5_4  3.92699082f
    #endif
    #ifndef PI3_2
        #define PI3_2  4.71238898f
    #endif
    #ifndef PI7_4
        #define PI7_4  5.49778714f
    #endif
    #ifndef PI2
        #define PI2    6.28318531f
    #endif
    
    #ifndef DEGPERRAD
        #define DEGPERRAD 57.2957795f
    #endif
    #ifndef RADPERDEG
        #define RADPERDEG 0.0174532925f
    #endif
#endif


/*
*  Bit flags
*/
#define SPG_FLAG0 0
#define SPG_FLAG1 0x01
#define SPG_FLAG2 0x02
#define SPG_FLAG3 0x04
#define SPG_FLAG4 0x08
#define SPG_FLAG5 0x10
#define SPG_FLAG6 0x20
#define SPG_FLAG7 0x40
#define SPG_FLAG8 0x80


/*
*  Define the right alpha values
*  (they were flipped in SDL 1.1.5+)
*  That means alpha is now a measure of opacity
*/
#ifndef SDL_ALPHA_OPAQUE
	#define SDL_ALPHA_OPAQUE 255
#endif
#ifndef SDL_ALPHA_TRANSPARENT
	#define SDL_ALPHA_TRANSPARENT 0
#endif


/*
*  Older versions of SDL don't have SDL_VERSIONNUM
*/
#ifndef SDL_VERSIONNUM
	#define SDL_VERSIONNUM(X, Y, Z) ((X)*1000 + (Y)*100 + (Z))
#endif


/*
*  Older versions of SDL don't have SDL_CreateRGBSurface
*/
#ifndef SDL_AllocSurface
	#define SDL_CreateRGBSurface  SDL_AllocSurface
#endif


/*
*  Macro to get clipping
*/
#if SDL_VERSIONNUM(SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_PATCHLEVEL) >= \
    SDL_VERSIONNUM(1, 1, 5)
	#define SPG_CLIP_XMIN(pnt) pnt->clip_rect.x
	#define SPG_CLIP_XMAX(pnt) pnt->clip_rect.x + pnt->clip_rect.w-1
	#define SPG_CLIP_YMIN(pnt) pnt->clip_rect.y
	#define SPG_CLIP_YMAX(pnt) pnt->clip_rect.y + pnt->clip_rect.h-1
#else
	#define SPG_CLIP_XMIN(pnt) pnt->clip_minx
	#define SPG_CLIP_XMAX(pnt) pnt->clip_maxx
	#define SPG_CLIP_YMIN(pnt) pnt->clip_miny
	#define SPG_CLIP_YMAX(pnt) pnt->clip_maxy
#endif


/*
*  We need to use alpha sometimes but older versions of SDL don't have
*  alpha support.
*/
#if SDL_VERSIONNUM(SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_PATCHLEVEL) >= \
    SDL_VERSIONNUM(1, 1, 5)
	#define SPG_MapRGBA SDL_MapRGBA
	#define SPG_GetRGBA SDL_GetRGBA
#else
	#define SPG_MapRGBA(fmt, r, g, b, a) SDL_MapRGB(fmt, r, g, b)
	#define SPG_GetRGBA(pixel, fmt, r, g, b, a) SDL_GetRGB(pixel, fmt, r, g, b)
#endif


/*
*  Some compilers use a special export keyword
*  Thanks to Seung Chan Lim (limsc@maya.com or slim@djslim.com) to pointing this out
*  (From SDL)
*/
#ifndef DECLSPEC
	#ifdef __BEOS__
		#if defined(__GNUC__)
			#define DECLSPEC __declspec(dllexport)
		#else
			#define DECLSPEC __declspec(export)
		#endif
	#else
		#ifdef WIN32
			#define DECLSPEC __declspec(dllexport)
		#else
			#define DECLSPEC
		#endif
	#endif
#endif

typedef struct SPG_Point
{
    float x;
    float y;
}SPG_Point;

/* A table of dirtyrects for one display page */
typedef struct SPG_DirtyTable
{
	Uint16		size;	/* Table size */
	SDL_Rect	*rects;	/* Table of rects */
	Uint16		count;	/* # of rects currently used */
	Uint16		best;	/* Merge testing starts here! */
} SPG_DirtyTable;


#define SPG_bool Uint8

// default = 0
#define SPG_DEST_ALPHA 0
#define SPG_SRC_ALPHA 1
#define SPG_COMBINE_ALPHA 2
#define SPG_COPY_NO_ALPHA 3
#define SPG_COPY_SRC_ALPHA 4
#define SPG_COPY_DEST_ALPHA 5
#define SPG_COPY_COMBINE_ALPHA 6
#define SPG_COPY_ALPHA_ONLY 7
#define SPG_COMBINE_ALPHA_ONLY 8
#define SPG_REPLACE_COLORKEY 9

// Alternate names:
#define SPG_SRC_MASK 4
#define SPG_DEST_MASK 5


/* Transformation flags */
#define SPG_NONE SPG_FLAG0
#define SPG_TAA SPG_FLAG1
#define SPG_TSAFE SPG_FLAG2
#define SPG_TTMAP SPG_FLAG3
#define SPG_TSLOW SPG_FLAG4
#define SPG_TCOLORKEY SPG_FLAG5
#define SPG_TBLEND SPG_FLAG6
#define SPG_TSURFACE_ALPHA SPG_FLAG7

#ifdef SPG_CPP // BOTH C and C++
extern "C" {
#endif


// MISC
DECLSPEC SDL_Surface* SPG_InitSDL(Uint16 w, Uint16 h, Uint8 bitsperpixel, Uint32 systemFlags, Uint32 screenFlags);

DECLSPEC void SPG_EnableAutolock(SPG_bool enable);
DECLSPEC SPG_bool SPG_GetAutolock();

DECLSPEC void SPG_EnableRadians(SPG_bool enable);
DECLSPEC SPG_bool SPG_GetRadians();

DECLSPEC void SPG_Error(const char* err);
DECLSPEC void SPG_EnableErrors(SPG_bool enable);
DECLSPEC char* SPG_GetError();
DECLSPEC Uint16 SPG_NumErrors();

DECLSPEC void SPG_PushThickness(Uint16 state);
DECLSPEC Uint16 SPG_PopThickness();
DECLSPEC Uint16 SPG_GetThickness();
DECLSPEC void SPG_PushBlend(Uint8 state);
DECLSPEC Uint8 SPG_PopBlend();
DECLSPEC Uint8 SPG_GetBlend();
DECLSPEC void SPG_PushAA(SPG_bool state);
DECLSPEC SPG_bool SPG_PopAA();
DECLSPEC SPG_bool SPG_GetAA();
DECLSPEC void SPG_PushSurfaceAlpha(SPG_bool state);
DECLSPEC SPG_bool SPG_PopSurfaceAlpha();
DECLSPEC SPG_bool SPG_GetSurfaceAlpha();

DECLSPEC void SPG_RectOR(const SDL_Rect rect1, const SDL_Rect rect2, SDL_Rect* dst_rect);
DECLSPEC SPG_bool SPG_RectAND(const SDL_Rect A, const SDL_Rect B, SDL_Rect* intersection);

// DIRTY RECT
//  Important stuff
DECLSPEC void SPG_EnableDirty(SPG_bool enable);
DECLSPEC void SPG_DirtyInit(Uint16 maxsize);
DECLSPEC void SPG_DirtyAdd(SDL_Rect* rect);
DECLSPEC SPG_DirtyTable* SPG_DirtyUpdate(SDL_Surface* screen);
DECLSPEC void SPG_DirtySwap();
//  Other stuff
DECLSPEC SPG_bool SPG_DirtyEnabled();
DECLSPEC SPG_DirtyTable* SPG_DirtyMake(Uint16 maxsize);
DECLSPEC void SPG_DirtyAddTo(SPG_DirtyTable* table, SDL_Rect* rect);
DECLSPEC void SPG_DirtyFree(SPG_DirtyTable* table);
DECLSPEC SPG_DirtyTable* SPG_DirtyGet();
DECLSPEC void SPG_DirtyClear(SPG_DirtyTable* table);
DECLSPEC void SPG_DirtyLevel(Uint16 optimizationLevel);
DECLSPEC void SPG_DirtyClip(SDL_Surface* screen, SDL_Rect* rect);

// PALETTE
DECLSPEC SDL_Color* SPG_ColorPalette();
DECLSPEC SDL_Color* SPG_GrayPalette();
DECLSPEC Uint32 SPG_FindPaletteColor(SDL_Palette* palette, Uint8 r, Uint8 g, Uint8 b);
DECLSPEC SDL_Surface* SPG_PalettizeSurface(SDL_Surface* surface, SDL_Palette* palette);

DECLSPEC void SPG_FadedPalette32(SDL_PixelFormat* format, Uint32 color1, Uint32 color2, Uint32* colorArray, Uint16 startIndex, Uint16 stopIndex);
DECLSPEC void SPG_FadedPalette32Alpha(SDL_PixelFormat* format, Uint32 color1, Uint8 alpha1, Uint32 color2, Uint8 alpha2, Uint32* colorArray, Uint16 start, Uint16 stop);
DECLSPEC void SPG_RainbowPalette32(SDL_PixelFormat* format, Uint32 *colorArray, Uint8 intensity, Uint16 startIndex, Uint16 stopIndex);
DECLSPEC void SPG_GrayPalette32(SDL_PixelFormat* format, Uint32 *colorArray, Uint16 startIndex, Uint16 stopIndex);

// SURFACE

DECLSPEC SDL_Surface* SPG_CreateSurface8(Uint32 flags, Uint16 width, Uint16 height);
DECLSPEC Uint32 SPG_GetPixel(SDL_Surface *surface, Sint16 x, Sint16 y);
DECLSPEC void SPG_SetClip(SDL_Surface *surface, const SDL_Rect rect);

DECLSPEC SDL_Rect SPG_TransformX(SDL_Surface *src, SDL_Surface *dst, float angle, float xscale, float yscale, Uint16 pivotX, Uint16 pivotY, Uint16 destX, Uint16 destY, Uint8 flags);
DECLSPEC SDL_Surface* SPG_Transform(SDL_Surface *src, Uint32 bgColor, float angle, float xscale, float yscale, Uint8 flags);
DECLSPEC SDL_Surface* SPG_Rotate(SDL_Surface *src, float angle, Uint32 bgColor);
DECLSPEC SDL_Surface* SPG_RotateAA(SDL_Surface *src, float angle, Uint32 bgColor);

DECLSPEC SDL_Surface* SPG_ReplaceColor(SDL_Surface* src, SDL_Rect* srcrect, SDL_Surface* dest, SDL_Rect* destrect, Uint32 color);


// DRAWING

DECLSPEC int SPG_Blit(SDL_Surface *Src, SDL_Rect* srcRect, SDL_Surface *Dest, SDL_Rect* destRect);
DECLSPEC void SPG_SetBlit(void (*blitfn)(SDL_Surface*, SDL_Rect*, SDL_Surface*, SDL_Rect*));
DECLSPEC void (*SPG_GetBlit())(SDL_Surface*, SDL_Rect*, SDL_Surface*, SDL_Rect*);

DECLSPEC void SPG_FloodFill(SDL_Surface *dst, Sint16 x, Sint16 y, Uint32 color);


// PRIMITIVES

DECLSPEC void SPG_Pixel(SDL_Surface *surface, Sint16 x, Sint16 y, Uint32 color);
DECLSPEC void SPG_PixelBlend(SDL_Surface *surface, Sint16 x, Sint16 y, Uint32 color, Uint8 alpha);
DECLSPEC void SPG_PixelPattern(SDL_Surface *surface, SDL_Rect target, SPG_bool* pattern, Uint32* colors);
DECLSPEC void SPG_PixelPatternBlend(SDL_Surface *surface, SDL_Rect target, SPG_bool* pattern, Uint32* colors, Uint8* pixelAlpha);

DECLSPEC void SPG_LineH(SDL_Surface *surface, Sint16 x1, Sint16 y, Sint16 x2, Uint32 Color);
DECLSPEC void SPG_LineHBlend(SDL_Surface *surface, Sint16 x1, Sint16 y, Sint16 x2, Uint32 color, Uint8 alpha);

DECLSPEC void SPG_LineHFade(SDL_Surface *dest,Sint16 x1,Sint16 y,Sint16 x2,Uint32 color1, Uint32 color2);
DECLSPEC void SPG_LineHTex(SDL_Surface *dest,Sint16 x1,Sint16 y,Sint16 x2,SDL_Surface *source,Sint16 sx1,Sint16 sy1,Sint16 sx2,Sint16 sy2);

DECLSPEC void SPG_LineV(SDL_Surface *surface, Sint16 x, Sint16 y1, Sint16 y2, Uint32 Color);
DECLSPEC void SPG_LineVBlend(SDL_Surface *surface, Sint16 x, Sint16 y1, Sint16 y2, Uint32 color, Uint8 alpha);

DECLSPEC void SPG_LineFn(SDL_Surface *surface, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint32 Color, void Callback(SDL_Surface *Surf, Sint16 X, Sint16 Y, Uint32 Color));
DECLSPEC void SPG_Line(SDL_Surface *surface, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint32 Color);
DECLSPEC void SPG_LineBlend(SDL_Surface *surface, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint32 color, Uint8 alpha);

DECLSPEC void SPG_LineFadeFn(SDL_Surface *surface, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint32 color1, Uint32 color2, void Callback(SDL_Surface *Surf, Sint16 X, Sint16 Y, Uint32 Color));
DECLSPEC void SPG_LineFade(SDL_Surface *surface, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint32 color1, Uint32 color2);
DECLSPEC void SPG_LineFadeBlend(SDL_Surface *surface, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint32 color1, Uint8 alpha1, Uint32 color2, Uint8 alpha2);


DECLSPEC void SPG_Rect(SDL_Surface *surface, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint32 color);
DECLSPEC void SPG_RectBlend(SDL_Surface *surface, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint32 color, Uint8 alpha);

DECLSPEC void SPG_RectFilled(SDL_Surface *surface, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint32 color);
DECLSPEC void SPG_RectFilledBlend(SDL_Surface *surface, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint32 color, Uint8 alpha);


DECLSPEC void SPG_RectRound(SDL_Surface *surface, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, float r, Uint32 color);
DECLSPEC void SPG_RectRoundBlend(SDL_Surface *surface, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, float r, Uint32 color, Uint8 alpha);

DECLSPEC void SPG_RectRoundFilled(SDL_Surface *surface, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, float r, Uint32 color);
DECLSPEC void SPG_RectRoundFilledBlend(SDL_Surface *surface, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, float r, Uint32 color, Uint8 alpha);


DECLSPEC void SPG_EllipseFn(SDL_Surface *surface, Sint16 x, Sint16 y, float rx, float ry, Uint32 color, void Callback(SDL_Surface *Surf, Sint16 X, Sint16 Y, Uint32 Color));
DECLSPEC void SPG_Ellipse(SDL_Surface *surface, Sint16 x, Sint16 y, float rx, float ry, Uint32 color);
DECLSPEC void SPG_EllipseBlend(SDL_Surface *surface, Sint16 x, Sint16 y, float rx, float ry, Uint32 color, Uint8 alpha);

DECLSPEC void SPG_EllipseFilled(SDL_Surface *surface, Sint16 x, Sint16 y, float rx, float ry, Uint32 color);
DECLSPEC void SPG_EllipseFilledBlend(SDL_Surface *surface, Sint16 x, Sint16 y, float rx, float ry, Uint32 color, Uint8 alpha);


DECLSPEC void SPG_EllipseArb(SDL_Surface *Surface, Sint16 x, Sint16 y, float rx, float ry, float angle, Uint32 color);
DECLSPEC void SPG_EllipseBlendArb(SDL_Surface *Surface, Sint16 x, Sint16 y, float rx, float ry, float angle, Uint32 color, Uint8 alpha);

DECLSPEC void SPG_EllipseFilledArb(SDL_Surface *Surface, Sint16 x, Sint16 y, float rx, float ry, float angle, Uint32 color);
DECLSPEC void SPG_EllipseFilledBlendArb(SDL_Surface *Surface, Sint16 x, Sint16 y, float rx, float ry, float angle, Uint32 color, Uint8 alpha);


DECLSPEC void SPG_CircleFn(SDL_Surface *surface, Sint16 x, Sint16 y, float r, Uint32 color, void Callback(SDL_Surface *Surf, Sint16 X, Sint16 Y, Uint32 Color));
DECLSPEC void SPG_Circle(SDL_Surface *surface, Sint16 x, Sint16 y, float r, Uint32 color);
DECLSPEC void SPG_CircleBlend(SDL_Surface *surface, Sint16 x, Sint16 y, float r, Uint32 color, Uint8 alpha);

DECLSPEC void SPG_CircleFilled(SDL_Surface *surface, Sint16 x, Sint16 y, float r, Uint32 color);
DECLSPEC void SPG_CircleFilledBlend(SDL_Surface *surface, Sint16 x, Sint16 y, float r, Uint32 color, Uint8 alpha);


DECLSPEC void SPG_ArcFn(SDL_Surface* surface, Sint16 cx, Sint16 cy, float radius, float startAngle, float endAngle, Uint32 color, void Callback(SDL_Surface *Surf, Sint16 X, Sint16 Y, Uint32 Color));
DECLSPEC void SPG_Arc(SDL_Surface* surface, Sint16 x, Sint16 y, float radius, float startAngle, float endAngle, Uint32 color);
DECLSPEC void SPG_ArcBlend(SDL_Surface* surface, Sint16 x, Sint16 y, float radius, float startAngle, float endAngle, Uint32 color, Uint8 alpha);

DECLSPEC void SPG_ArcFilled(SDL_Surface* surface, Sint16 cx, Sint16 cy, float radius, float startAngle, float endAngle, Uint32 color);
DECLSPEC void SPG_ArcFilledBlend(SDL_Surface* surface, Sint16 cx, Sint16 cy, float radius, float startAngle, float endAngle, Uint32 color, Uint8 alpha);


DECLSPEC void SPG_Bezier(SDL_Surface *surface, Sint16 startX, Sint16 startY, Sint16 cx1, Sint16 cy1, Sint16 cx2, Sint16 cy2, Sint16 endX, Sint16 endY, Uint8 quality, Uint32 color);
DECLSPEC void SPG_BezierBlend(SDL_Surface *surface, Sint16 startX, Sint16 startY, Sint16 cx1, Sint16 cy1, Sint16 cx2, Sint16 cy2, Sint16 endX, Sint16 endY, Uint8 quality, Uint32 color, Uint8 alpha);


// POLYGONS

DECLSPEC void SPG_Trigon(SDL_Surface *surface,Sint16 x1,Sint16 y1,Sint16 x2,Sint16 y2,Sint16 x3,Sint16 y3,Uint32 color);
DECLSPEC void SPG_TrigonBlend(SDL_Surface *surface,Sint16 x1,Sint16 y1,Sint16 x2,Sint16 y2,Sint16 x3,Sint16 y3,Uint32 color, Uint8 alpha);

DECLSPEC void SPG_TrigonFilled(SDL_Surface *surface,Sint16 x1,Sint16 y1,Sint16 x2,Sint16 y2,Sint16 x3,Sint16 y3,Uint32 color);
DECLSPEC void SPG_TrigonFilledBlend(SDL_Surface *surface,Sint16 x1,Sint16 y1,Sint16 x2,Sint16 y2,Sint16 x3,Sint16 y3,Uint32 color, Uint8 alpha);

DECLSPEC void SPG_TrigonFade(SDL_Surface *surface,Sint16 x1,Sint16 y1,Sint16 x2,Sint16 y2,Sint16 x3,Sint16 y3,Uint32 color1,Uint32 color2,Uint32 color3);
DECLSPEC void SPG_TrigonTex(SDL_Surface *dest,Sint16 x1,Sint16 y1,Sint16 x2,Sint16 y2,Sint16 x3,Sint16 y3,SDL_Surface *source,Sint16 sx1,Sint16 sy1,Sint16 sx2,Sint16 sy2,Sint16 sx3,Sint16 sy3);


DECLSPEC void SPG_QuadTex(SDL_Surface* dest, Sint16 destULx, Sint16 destULy, Sint16 destDLx, Sint16 destDLy, Sint16 destDRx, Sint16 destDRy, Sint16 destURx, Sint16 destURy, SDL_Surface* source, Sint16 srcULx, Sint16 srcULy, Sint16 srcDLx, Sint16 srcDLy, Sint16 srcDRx, Sint16 srcDRy, Sint16 srcURx, Sint16 srcURy);

DECLSPEC void SPG_Polygon(SDL_Surface *dest, Uint16 n, SPG_Point* points, Uint32 color);
DECLSPEC void SPG_PolygonBlend(SDL_Surface *dest, Uint16 n, SPG_Point* points, Uint32 color, Uint8 alpha);

DECLSPEC void SPG_PolygonFilled(SDL_Surface *surface, Uint16 n, SPG_Point* points, Uint32 color);
DECLSPEC void SPG_PolygonFilledBlend(SDL_Surface *surface, Uint16 n, SPG_Point* points, Uint32 color, Uint8 alpha);

DECLSPEC void SPG_PolygonFade(SDL_Surface *surface, Uint16 n, SPG_Point* points, Uint32* colors);
DECLSPEC void SPG_PolygonFadeBlend(SDL_Surface *surface, Uint16 n, SPG_Point* points, Uint32* colors, Uint8 alpha);

DECLSPEC void SPG_CopyPoints(Uint16 n, SPG_Point* points, SPG_Point* buffer);
DECLSPEC void SPG_RotatePointsXY(Uint16 n, SPG_Point* points, float cx, float cy, float angle);
DECLSPEC void SPG_ScalePointsXY(Uint16 n, SPG_Point* points, float cx, float cy, float xscale, float yscale);
DECLSPEC void SPG_SkewPointsXY(Uint16 n, SPG_Point* points, float cx, float cy, float xskew, float yskew);
DECLSPEC void SPG_TranslatePoints(Uint16 n, SPG_Point* points, float x, float y);


#ifdef SPG_CPP
}  // extern "C"
#endif




// Include all convenience calls
#include "sprig_inline.h"

// Include extended calls
#ifdef SPG_USE_EXTENDED


    #ifdef SPG_CPP // BOTH C and C++
    extern "C" {
    #endif


        DECLSPEC void SPG_FloodFill8(SDL_Surface* dest, Sint16 x, Sint16 y, Uint32 newColor);


    #ifdef SPG_CPP // BOTH C and C++
    }  // extern "C"
    #endif


#endif




#endif /* _SPRIG_H__ */

