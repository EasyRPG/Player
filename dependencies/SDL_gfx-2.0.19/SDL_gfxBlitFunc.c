/* 

 SDL_gfxBlitFunc: custom blitters (part of SDL_gfx library)

 LGPL (c) A. Schiffler
 
*/


#include "SDL_gfxBlitFunc.h"

/* Special blitter for correct destination Alpha during RGBA->RGBA blits */

void SDL_gfxBlitBlitterRGBA(SDL_gfxBlitInfo * info)
{
  int       width = info->d_width;
  int       height = info->d_height;
  Uint8    *src = info->s_pixels;
  int       srcskip = info->s_skip;
  Uint8    *dst = info->d_pixels;
  int       dstskip = info->d_skip;
  SDL_PixelFormat *srcfmt = info->src;
  SDL_PixelFormat *dstfmt = info->dst;
  int       srcbpp = srcfmt->BytesPerPixel;
  int       dstbpp = dstfmt->BytesPerPixel;

  while (height--) {
    GFX_DUFFS_LOOP4( {
		Uint32 pixel;
		unsigned sR;
		unsigned sG;
		unsigned sB;
		unsigned sA;
		unsigned dR;
		unsigned dG;
		unsigned dB;
		unsigned dA;
		unsigned sAA;
		GFX_DISEMBLE_RGBA(src, srcbpp, srcfmt, pixel, sR, sG, sB, sA);
		GFX_DISEMBLE_RGBA(dst, dstbpp, dstfmt, pixel, dR, dG, dB, dA);
		sAA=GFX_ALPHA_ADJUST[sA & 255]; 
		GFX_ALPHA_BLEND(sR, sG, sB, sAA, dR, dG, dB); 
		dA |= sAA;
		GFX_ASSEMBLE_RGBA(dst, dstbpp, dstfmt, dR, dG, dB, dA); 
		src += srcbpp; dst += dstbpp;
        }, width);
    src += srcskip;
    dst += dstskip;
  }
}

int SDL_gfxBlitRGBACall(SDL_Surface * src, SDL_Rect * srcrect, SDL_Surface * dst, SDL_Rect * dstrect)
{
  /*
   * Set up source and destination buffer pointers, then blit 
   */
  if (srcrect->w && srcrect->h) {
    SDL_gfxBlitInfo info;

    /*
     * Set up the blit information 
     */
    info.s_pixels = (Uint8 *) src->pixels + src->offset + (Uint16) srcrect->y * src->pitch + (Uint16) srcrect->x * src->format->BytesPerPixel;
    info.s_width = srcrect->w;
    info.s_height = srcrect->h;
    info.s_skip = src->pitch - info.s_width * src->format->BytesPerPixel;
    info.d_pixels = (Uint8 *) dst->pixels + dst->offset + (Uint16) dstrect->y * dst->pitch + (Uint16) dstrect->x * dst->format->BytesPerPixel;
    info.d_width = dstrect->w;
    info.d_height = dstrect->h;
    info.d_skip = dst->pitch - info.d_width * dst->format->BytesPerPixel;
    info.aux_data = NULL;
    info.src = src->format;
    info.table = NULL;
    info.dst = dst->format;

    /*
     * Run the actual software blitter 
     */
    SDL_gfxBlitBlitterRGBA(&info);
  }

  return (0);
}


int SDL_gfxBlitRGBA(SDL_Surface * src, SDL_Rect * srcrect, SDL_Surface * dst, SDL_Rect * dstrect)
{
  SDL_Rect  sr, dr;
  int       srcx, srcy, w, h;

  /*
   * Make sure the surfaces aren't locked 
   */
  if (!src || !dst) {
    SDL_SetError("SDL_UpperBlit: passed a NULL surface");
    return (-1);
  }
  if (src->locked || dst->locked) {
    SDL_SetError("Surfaces must not be locked during blit");
    return (-1);
  }

  /*
   * If the destination rectangle is NULL, use the entire dest surface 
   */
  if (dstrect == NULL) {
    dr.x = dr.y = 0;
    dr.w = dst->w;
    dr.h = dst->h;
  } else {
    dr = *dstrect;
  }

  /*
   * Clip the source rectangle to the source surface 
   */
  if (srcrect) {
    int       maxw, maxh;

    srcx = srcrect->x;
    w = srcrect->w;
    if (srcx < 0) {
      w += srcx;
      dr.x -= srcx;
      srcx = 0;
    }
    maxw = src->w - srcx;
    if (maxw < w)
      w = maxw;

    srcy = srcrect->y;
    h = srcrect->h;
    if (srcy < 0) {
      h += srcy;
      dr.y -= srcy;
      srcy = 0;
    }
    maxh = src->h - srcy;
    if (maxh < h)
      h = maxh;

  } else {
    srcx = srcy = 0;
    w = src->w;
    h = src->h;
  }

  /*
   * Clip the destination rectangle against the clip rectangle 
   */
  {
    SDL_Rect *clip = &dst->clip_rect;
    int       dx, dy;

    dx = clip->x - dr.x;
    if (dx > 0) {
      w -= dx;
      dr.x += dx;
      srcx += dx;
    }
    dx = dr.x + w - clip->x - clip->w;
    if (dx > 0)
      w -= dx;

    dy = clip->y - dr.y;
    if (dy > 0) {
      h -= dy;
      dr.y += dy;
      srcy += dy;
    }
    dy = dr.y + h - clip->y - clip->h;
    if (dy > 0)
      h -= dy;
  }

  if (w > 0 && h > 0) {
    sr.x = srcx;
    sr.y = srcy;
    sr.w = dr.w = w;
    sr.h = dr.h = h;
    return (SDL_gfxBlitRGBACall(src, &sr, dst, &dr));
  }
  return 0;
}

/* Helper function that sets the alpha channel in a 32bit surface */

int SDL_gfxSetAlpha(SDL_Surface *src, Uint8 a)
{
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	Uint16 alpha_offset = 0;
#else
	Uint16 alpha_offset = 3;
#endif
	Uint16 i, j;
	
	/* Check if we have a 32bit surface */
	if ( (src) && (src->format) && (src->format->BytesPerPixel==4) ) {
	 /* Lock and process */
  	 if ( SDL_LockSurface(src) == 0 ) {
	  Uint8 *pixels = (Uint8 *)src->pixels;
	  Uint16 row_skip = (src->pitch - (4*src->w));
	  pixels += alpha_offset;
	  for ( i=0; i<src->h; i++ ) {
		 for ( j=0; j<src->w; j++  ) {
		  *pixels = a; 
		  pixels += 4;
		 }
		 pixels += row_skip;
          }
          SDL_UnlockSurface(src);
	 }
	 return 1;
        } else {
         return 0;
        } 
}
