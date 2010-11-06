#ifndef _SPG_INLINE_H__
#define _SPG_INLINE_H__


/* Colors */

static inline Uint8 SPG_GetRed(SDL_PixelFormat* format, Uint32 color)
{
    return (color & format->Rmask) >> format->Rshift;
}
static inline Uint8 SPG_GetGreen(SDL_PixelFormat* format, Uint32 color)
{
    return (color & format->Gmask) >> format->Gshift;
}
static inline Uint8 SPG_GetBlue(SDL_PixelFormat* format, Uint32 color)
{
    return (color & format->Bmask) >> format->Bshift;
}
static inline Uint8 SPG_GetAlpha(SDL_PixelFormat* format, Uint32 color)
{
    return (color & format->Amask) >> format->Ashift;
}


static inline Uint32 SPG_MixRed(SDL_PixelFormat* format, Uint32 color, Uint8 red)
{
    return (color & (~(format->Rmask))) | (red << format->Rshift);
}
static inline Uint32 SPG_MixGreen(SDL_PixelFormat* format, Uint32 color, Uint8 green)
{
    return (color & (~(format->Gmask))) | (green << format->Gshift);
}
static inline Uint32 SPG_MixBlue(SDL_PixelFormat* format, Uint32 color, Uint8 blue)
{
    return (color & (~(format->Bmask))) | (blue << format->Bshift);
}
static inline Uint32 SPG_MixAlpha(SDL_PixelFormat* format, Uint32 color, Uint8 alpha)
{
    return (color & (~(format->Amask))) | (alpha << format->Ashift);
}

static inline Uint32 SPG_ConvertColor(SDL_PixelFormat* srcfmt, Uint32 srccolor, SDL_PixelFormat* destfmt)
{
    if(srcfmt == NULL || destfmt == NULL)
        return 0;
    Uint8 r, g, b;
    SDL_GetRGB(srccolor, srcfmt, &r, &g, &b);
    return SDL_MapRGB(destfmt, r, g, b);
}

static inline SDL_Color SPG_GetColor(SDL_Surface* Surface, Uint32 Color)
{
	SDL_Color rgb;
	SDL_GetRGB(Color, Surface->format, &(rgb.r), &(rgb.g), &(rgb.b));
	rgb.unused = 0;
	return rgb;
}

static inline SDL_Color SPG_MakeColor(Uint8 R, Uint8 G, Uint8 B)
{
   SDL_Color color = {R, G, B, 0};
   return color;
}

static inline SDL_Color SPG_MakeColorAlpha(Uint8 R, Uint8 G, Uint8 B, Uint8 A)
{
   SDL_Color color = {R, G, B, A};
   return color;
}


/* Points */

static inline SPG_Point SPG_MakePoint(Sint16 x, Sint16 y)
{
    SPG_Point p = {x, y};
    return p;
}

static inline void SPG_RotatePoints(Uint16 n, SPG_Point* points, float angle)
{
    SPG_RotatePointsXY(n, points, 0, 0, angle);
}

static inline void SPG_ScalePoints(Uint16 n, SPG_Point* points, float xscale, float yscale)
{
    SPG_ScalePointsXY(n, points, 0, 0, xscale, yscale);
}

static inline void SPG_SkewPoints(Uint16 n, SPG_Point* points, float xskew, float yskew)
{
    SPG_SkewPointsXY(n, points, 0, 0, xskew, yskew);
}

static inline void SPG_TrigonTexPoints(SDL_Surface* dest, SPG_Point* dest_points, SDL_Surface* source, SPG_Point* source_points)
{
    SPG_TrigonTex(dest, (Sint16)(dest_points[0].x + 0.5f), (Sint16)(dest_points[0].y + 0.5f), (Sint16)(dest_points[1].x + 0.5f), (Sint16)(dest_points[1].y + 0.5f), (Sint16)(dest_points[2].x + 0.5f), (Sint16)(dest_points[2].y + 0.5f), 
                  source, (Sint16)(source_points[0].x + 0.5f), (Sint16)(source_points[0].y + 0.5f), (Sint16)(source_points[1].x + 0.5f), (Sint16)(source_points[1].y + 0.5f), (Sint16)(source_points[2].x + 0.5f), (Sint16)(source_points[2].y + 0.5f));
}

static inline void SPG_QuadTexPoints(SDL_Surface* dest, SPG_Point* dest_points, SDL_Surface *source, SPG_Point* source_points)
{
    SPG_QuadTex(dest, (Sint16)(dest_points[0].x + 0.5f), (Sint16)(dest_points[0].y + 0.5f), (Sint16)(dest_points[1].x + 0.5f), (Sint16)(dest_points[1].y + 0.5f), (Sint16)(dest_points[2].x + 0.5f), (Sint16)(dest_points[2].y + 0.5f),(Sint16)(dest_points[3].x + 0.5f),(Sint16)(dest_points[3].y + 0.5f),
                source, (Sint16)(source_points[0].x + 0.5f), (Sint16)(source_points[0].y + 0.5f), (Sint16)(source_points[1].x + 0.5f), (Sint16)(source_points[1].y + 0.5f), (Sint16)(source_points[2].x + 0.5f), (Sint16)(source_points[2].y + 0.5f), (Sint16)(source_points[3].x + 0.5f),(Sint16)(source_points[3].y + 0.5f));
}


/* Scale */

static inline SDL_Surface* SPG_Scale(SDL_Surface *src, float xscale, float yscale)
{
    return SPG_Transform(src, 0x000000, 0, xscale, yscale, 0);
}

static inline SDL_Surface* SPG_ScaleAA(SDL_Surface *src, float xscale, float yscale)
{
    return SPG_Transform(src, 0x000000, 0, xscale, yscale, SPG_TAA);
}


/* C++ Rotate without bgColor */

#ifndef SPG_C_ONLY

static inline SDL_Surface* SPG_Rotate(SDL_Surface *src, float angle)
{
	return SPG_Rotate(src, angle, 0);
}

static inline SDL_Surface* SPG_RotateAA(SDL_Surface *src, float angle)
{
    return SPG_RotateAA(src, angle, 0);
}
#endif


/* Surface */

static inline void SPG_Free(SDL_Surface* surface)
{
    SDL_FreeSurface(surface);
}

static inline void SPG_SetColorkey(SDL_Surface* surface, Uint32 color)
{
    SDL_SetColorKey(surface, SDL_SRCCOLORKEY, color);
}

static inline SDL_Surface* SPG_DisplayFormat(SDL_Surface* surf)
{
    SDL_Surface* temp = SDL_DisplayFormat(surf);
    SDL_FreeSurface(surf);
    return temp;
}

static inline SDL_Surface* SPG_DisplayFormatAlpha(SDL_Surface* surf)
{
    SDL_Surface* temp = SDL_DisplayFormatAlpha(surf);
    SDL_FreeSurface(surf);
    return temp;
}

static inline SDL_Rect SPG_GetClip(SDL_Surface* surface)
{
	if (surface)
		return surface->clip_rect;
	else
	{
        SDL_Rect r;
        r.x = 0;
        r.y = 0;
        r.w = 0;
        r.h = 0;
        return r;
	}
}

static inline void SPG_RestoreClip(SDL_Surface* surface)
{
    if(surface)
    {
        surface->clip_rect.x = 0;
        surface->clip_rect.y = 0;
        surface->clip_rect.w = surface->w;
        surface->clip_rect.h = surface->h;
    }
}

// 5-5-5
static inline SDL_Surface* SPG_CreateSurface16(Uint32 flags, Uint16 width, Uint16 height)
{
    SDL_Surface* result = SDL_CreateRGBSurface(flags, width, height, 16, 31 << 10, 31 << 5, 31, 0);
    
    return result;
}

// 4-4-4-4
static inline SDL_Surface* SPG_CreateSurface16Alpha(Uint32 flags, Uint16 width, Uint16 height)
{
    #if SDL_BYTEORDER == SDL_BIG_ENDIAN
    SDL_Surface* result = SDL_CreateRGBSurface(flags, width, height, 16, 0xf000, 0x0f00, 0x00f0, 0x000f);
    #else
    SDL_Surface* result = SDL_CreateRGBSurface(flags, width, height, 16, 0x00f0, 0x000f, 0xf000, 0x0f00);
    #endif
    
    return result;
}

static inline SDL_Surface* SPG_CreateSurface24(Uint32 flags, Uint16 width, Uint16 height)
{
    #if SDL_BYTEORDER == SDL_BIG_ENDIAN
        SDL_Surface* result = SDL_CreateRGBSurface(flags,width,height,24, 0xFF0000, 0x00FF00, 0x0000FF, 0);
    #else
        SDL_Surface* result = SDL_CreateRGBSurface(flags,width,height,24, 0x0000FF, 0x00FF00, 0xFF0000, 0);
    #endif
    SDL_SetAlpha(result, 0, SDL_ALPHA_OPAQUE);
	return result;
}

static inline SDL_Surface* SPG_CreateSurface32(Uint32 flags, Uint16 width, Uint16 height)
{
    #if SDL_BYTEORDER == SDL_BIG_ENDIAN
        SDL_Surface* result = SDL_CreateRGBSurface(flags,width,height,32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
    #else
        SDL_Surface* result = SDL_CreateRGBSurface(flags,width,height,32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
    #endif
    SDL_SetAlpha(result, SDL_SRCALPHA, SDL_ALPHA_OPAQUE);
	return result;
}

#define SPG_CreateSurface SPG_CreateSurface32


static inline SDL_Surface* SPG_CreateSurfaceFrom(void* linearArray, Uint16 width, Uint16 height, SDL_PixelFormat* format)
{
    if(linearArray == NULL || format == NULL)
        return NULL;
    SDL_Surface* result = SDL_CreateRGBSurfaceFrom(linearArray, width, height, format->BitsPerPixel, width*format->BytesPerPixel, format->Rmask, format->Gmask, format->Bmask, format->Amask);
    if(format->Amask)
        SDL_SetAlpha(result, SDL_SRCALPHA, SDL_ALPHA_OPAQUE);
    return result;
}

static inline SDL_Surface* SPG_CopySurface(SDL_Surface* src)
{
	return SDL_ConvertSurface(src, src->format, SDL_SWSURFACE);
}

static inline void SPG_SetSurfaceAlpha(SDL_Surface* surface, Uint8 alpha)
{
    SDL_SetAlpha(surface, surface->flags & SDL_SRCALPHA, alpha);
}


/* Misc */

static inline int SPG_Clamp(int value, int min, int max)
{
    return ((value < min)? min : (value > max)? max : value);
}

static inline SDL_Rect SPG_MakeRect(Sint16 x, Sint16 y, Uint16 w, Uint16 h)
{
    SDL_Rect r;
    r.x = x;
    r.y = y;
    r.w = w;
    r.h = h;
    return r;
}

static inline SDL_Rect SPG_MakeRectRelative(Sint16 x, Sint16 y, Sint16 x2, Sint16 y2)
{
    SDL_Rect r;
    r.x = x;
    r.y = y;
    r.w = x2 - x;
    r.h = y2 - y;
    return r;
}


/* Drawing */

static inline void SPG_Fill(SDL_Surface* surface, Uint32 color)
{
	SDL_FillRect(surface, NULL, color);
}

static inline void SPG_FillAlpha(SDL_Surface* surface, Uint32 color, Uint8 alpha)
{
	SDL_FillRect(surface, NULL, SPG_MixAlpha(surface->format, color, alpha));
}


static inline void SPG_BlockWrite8(SDL_Surface* Surface, Uint8* block, Sint16 y)
{
	memcpy(	(Uint8 *)Surface->pixels + y*Surface->pitch, block, sizeof(Uint8)*Surface->w );
}
static inline void SPG_BlockWrite16(SDL_Surface* Surface, Uint16* block, Sint16 y)
{
	memcpy(	(Uint16 *)Surface->pixels + y*Surface->pitch/2, block, sizeof(Uint16)*Surface->w );
}
static inline void SPG_BlockWrite32(SDL_Surface* Surface, Uint32* block, Sint16 y)
{
	memcpy(	(Uint32 *)Surface->pixels + y*Surface->pitch/4, block, sizeof(Uint32)*Surface->w );
}

static inline void SPG_BlockRead8(SDL_Surface* Surface, Uint8* block, Sint16 y)
{
	memcpy(	block,(Uint8 *)Surface->pixels + y*Surface->pitch, sizeof(Uint8)*Surface->w );
}
static inline void SPG_BlockRead16(SDL_Surface* Surface, Uint16* block, Sint16 y)
{
	memcpy(	block,(Uint16 *)Surface->pixels + y*Surface->pitch/2, sizeof(Uint16)*Surface->w );
}
static inline void SPG_BlockRead32(SDL_Surface* Surface, Uint32* block, Sint16 y)
{
	memcpy(	block,(Uint32 *)Surface->pixels + y*Surface->pitch/4, sizeof(Uint32)*Surface->w );
}




static inline void SPG_Draw(SDL_Surface* source, SDL_Surface* dest, Sint16 x, Sint16 y)
{
    SDL_Rect rect;
    rect.x = x;
    rect.y = y;
    SDL_BlitSurface(source, NULL, dest, &rect);
}

static inline void SPG_DrawCenter(SDL_Surface* source, SDL_Surface* dest, Sint16 x, Sint16 y)
{
    SDL_Rect rect;
    rect.x = x - source->w/2;
    rect.y = y - source->h/2;
    SDL_BlitSurface(source, NULL, dest, &rect);
}

static inline void SPG_DrawBlit(SDL_Surface* source, SDL_Surface* dest, Sint16 x, Sint16 y)
{
    SDL_Rect rect;
    rect.x = x;
    rect.y = y;
    SPG_Blit(source, NULL, dest, &rect);
}







#endif
