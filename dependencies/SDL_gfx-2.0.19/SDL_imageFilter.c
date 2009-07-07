/*
 
 SDL_imageFilter - bytes-image "filter" routines 
 (uses inline x86 MMX optimizations if available)
 
 LGPL (c) A. Schiffler

 Note: Most MMX code is based on published routines 
 by Vladimir Kravtchenko at vk@cs.ubc.ca - credits to 
 him for his work.

*/

#include <stdio.h>
#include <stdlib.h>

#include "SDL_imageFilter.h"

#define swap_32(x) (((x) >> 24) | (((x) & 0x00ff0000) >> 8)  | (((x) & 0x0000ff00) << 8)  | ((x) << 24))

/* ------ Static variables ----- */

/* Toggle the use of the MMX routines - ON by default */

static int SDL_imageFilterUseMMX = 1;

/* MMX detection routine (with override flag) */

unsigned int cpuFlags()
{
    int flags = 0;

#ifdef USE_MMX
    asm volatile ("pusha		     \n\t" "mov    %1, %%eax     \n\t"	/* request feature flag */
		  "cpuid                \n\t"	/* get CPU ID flag */
		  "mov    %%edx, %0     \n\t"	/* move result to mmx_bit */
		  "popa		     \n\t":"=m" (flags)	/* %0 */
		  :"i"(0x00000001)	/* %1 */
	);
#endif

    return (flags);
}

int SDL_imageFilterMMXdetect(void)
{
    unsigned int mmx_bit;

    /* Check override flag */
    if (SDL_imageFilterUseMMX == 0) {
	return (0);
    }

    mmx_bit = cpuFlags();
    mmx_bit &= 0x00800000;
    mmx_bit = (mmx_bit && 0x00800000);

    return (mmx_bit);
}

void SDL_imageFilterMMXoff()
{
    SDL_imageFilterUseMMX = 0;
}

void SDL_imageFilterMMXon()
{
    SDL_imageFilterUseMMX = 1;
}

/* ------------------------------------------------------------------------------------ */

/*  SDL_imageFilterAdd: D = saturation255(S1 + S2) */
int SDL_imageFilterAddMMX(unsigned char *Src1, unsigned char *Src2, unsigned char *Dest, int length)
{
#ifdef USE_MMX
    asm volatile
     ("pusha		     \n\t" "mov          %2, %%eax \n\t"	/* load Src1 address into eax */
      "mov          %1, %%ebx \n\t"	/* load Src2 address into ebx */
      "mov          %0, %%edi \n\t"	/* load Dest address into edi */
       "mov          %3, %%ecx \n\t"	/* load loop counter (SIZE) into ecx */
      "shr          $3, %%ecx \n\t"	/* counter/8 (MMX loads 8 bytes at a time) */
       ".align 16              \n\t"	/* 16 byte allignment of the loop entry */
      "1: movq (%%eax), %%mm1 \n\t"    	/* load 8 bytes from Src1 into mm1 */
      "paddusb (%%ebx), %%mm1 \n\t"	/* mm1=Src1+Src2 (add 8 bytes with saturation) */
      "movq    %%mm1, (%%edi) \n\t"	/* store result in Dest */
       "add          $8, %%eax \n\t"	/* increase Src1, Src2 and Dest  */
      "add          $8, %%ebx \n\t"	/* register pointers by 8 */
      "add          $8, %%edi \n\t" "dec              %%ecx \n\t"	/* decrease loop counter */
      "jnz             1b     \n\t"     /* check loop termination, proceed if required */
       "emms                   \n\t"	/* exit MMX state */
      "popa                   \n\t":"=m" (Dest)	/* %0 */
      :"m"(Src2),		/* %1 */
      "m"(Src1),		/* %2 */
      "m"(length)		/* %3 */
	);
#endif
    return (0);
}

/*  SDL_imageFilterAdd: D = saturation255(S1 + S2) */
int SDL_imageFilterAdd(unsigned char *Src1, unsigned char *Src2, unsigned char *Dest, int length)
{
    unsigned int i, istart;
    unsigned char *cursrc1, *cursrc2, *curdst;
    int result;

    if ((SDL_imageFilterMMXdetect()) && (length > 7)) {

	/* Use MMX assembly routine */
	SDL_imageFilterAddMMX(Src1, Src2, Dest, length);

	/* Check for unaligned bytes */
	if ((length & 7) > 0) {
	    /* Setup to process unaligned bytes */
	    istart = length & 0xfffffff8;
	    cursrc1 = &Src1[istart];
	    cursrc2 = &Src2[istart];
	    curdst = &Dest[istart];
	} else {
	    /* No unaligned bytes - we are done */
	    return (0);
	}
    } else {
	/* Setup to process whole image */
	istart = 0;
	cursrc1 = Src1;
	cursrc2 = Src2;
	curdst = Dest;
    }

    /* C routine to process image */
    for (i = istart; i < length; i++) {
	result = (int) *cursrc1 + (int) *cursrc2;
	if (result > 255)
	    result = 255;
	*curdst = (unsigned char) result;
	/* Advance pointers */
	cursrc1++;
	cursrc2++;
	curdst++;
    }

    return (0);
}

/*  SDL_imageFilterMean: D = S1/2 + S2/2 */
int SDL_imageFilterMeanMMX(unsigned char *Src1, unsigned char *Src2, unsigned char *Dest, int length,
			   unsigned char *Mask)
{
#ifdef USE_MMX
    asm volatile
     ("pusha		     \n\t" "movl         %4, %%edx \n\t"	/* load Mask address into edx */
      "movq    (%%edx), %%mm0 \n\t"	/* load Mask into mm0 */
       "mov          %2, %%eax \n\t"	/* load Src1 address into eax */
      "mov          %1, %%ebx \n\t"	/* load Src2 address into ebx */
      "mov          %0, %%edi \n\t"	/* load Dest address into edi */
       "mov          %3, %%ecx \n\t"	/* load loop counter (SIZE) into ecx */
      "shr          $3, %%ecx \n\t"	/* counter/8 (MMX loads 8 bytes at a time) */
       ".align 16              \n\t"	/* 16 byte allignment of the loop entry */
      "1:                      \n\t"
      "movq    (%%eax), %%mm1 \n\t"	/* load 8 bytes from Src1 into mm1 */
      "movq    (%%ebx), %%mm2 \n\t"	/* load 8 bytes from Src2 into mm2 */
      /* --- Byte shift via Word shift --- */
       "psrlw        $1, %%mm1 \n\t"	/* shift 4 WORDS of mm1 1 bit to the right */
      "psrlw        $1, %%mm2 \n\t"	/* shift 4 WORDS of mm2 1 bit to the right */
/*      "pand      %%mm0, %%mm1 \n\t"    // apply Mask to 8 BYTES of mm1 */
      ".byte     0x0f, 0xdb, 0xc8 \n\t"
/*      "pand      %%mm0, %%mm2 \n\t"    // apply Mask to 8 BYTES of mm2 */
      ".byte     0x0f, 0xdb, 0xd0 \n\t" 
      "paddusb   %%mm2, %%mm1 \n\t"	/* mm1=mm1+mm2 (add 8 bytes with saturation) */
      "movq    %%mm1, (%%edi) \n\t"	/* store result in Dest */
       "add          $8, %%eax \n\t"	/* increase Src1, Src2 and Dest  */
      "add          $8, %%ebx \n\t"	/* register pointers by 8 */
      "add          $8, %%edi \n\t" 
      "dec              %%ecx \n\t"	/* decrease loop counter */
      "jnz                 1b \n\t"     /* check loop termination, proceed if required */
       "emms                   \n\t"	/* exit MMX state */
      "popa                   \n\t":"=m" (Dest)	/* %0 */
      :"m"(Src2),		/* %1 */
      "m"(Src1),		/* %2 */
      "m"(length),		/* %3 */
      "m"(Mask)			/* %4 */
	);
#endif
    return (0);
}

/*  SDL_imageFilterMean: D = S1/2 + S2/2 */
int SDL_imageFilterMean(unsigned char *Src1, unsigned char *Src2, unsigned char *Dest, int length)
{
    static unsigned char Mask[8] = { 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F };
    unsigned int i, istart;
    unsigned char *cursrc1, *cursrc2, *curdst;
    int result;

    if ((SDL_imageFilterMMXdetect()) && (length > 7)) {
	/* MMX routine */
	SDL_imageFilterMeanMMX(Src1, Src2, Dest, length, Mask);

	/* Check for unaligned bytes */
	if ((length & 7) > 0) {
	    /* Setup to process unaligned bytes */
	    istart = length & 0xfffffff8;
	    cursrc1 = &Src1[istart];
	    cursrc2 = &Src2[istart];
	    curdst = &Dest[istart];
	} else {
	    /* No unaligned bytes - we are done */
	    return (0);
	}
    } else {
	/* Setup to process whole image */
	istart = 0;
	cursrc1 = Src1;
	cursrc2 = Src2;
	curdst = Dest;
    }

    /* C routine to process image */
    for (i = istart; i < length; i++) {
	result = (int) *cursrc1 / 2 + (int) *cursrc2 / 2;
	*curdst = (unsigned char) result;
	/* Advance pointers */
	cursrc1++;
	cursrc2++;
	curdst++;
    }

    return (0);
}

/*  SDL_imageFilterSub: D = saturation0(S1 - S2) */
int SDL_imageFilterSubMMX(unsigned char *Src1, unsigned char *Src2, unsigned char *Dest, int length)
{
#ifdef USE_MMX
    asm volatile
     ("pusha		     \n\t" "mov %2, %%eax \n\t"	/* load Src1 address into eax */
      "mov %1, %%ebx \n\t"	/* load Src2 address into ebx */
      "mov %0, %%edi \n\t"	/* load Dest address into edi */
       "mov %3, %%ecx \n\t"	/* load loop counter (SIZE) into ecx */
      "shr $3, %%ecx \n\t"	/* counter/8 (MMX loads 8 bytes at a time) */
       ".align 16       \n\t"	/* 16 byte allignment of the loop entry */
      "1: movq (%%eax), %%mm1 \n\t"     /* load 8 bytes from Src1 into mm1 */
      "psubusb (%%ebx), %%mm1 \n\t"	/* mm1=Src1-Src2 (sub 8 bytes with saturation) */
      "movq    %%mm1, (%%edi) \n\t"	/* store result in Dest */
       "add $8, %%eax \n\t"	/* increase Src1, Src2 and Dest  */
      "add $8, %%ebx \n\t"	/* register pointers by 8 */
      "add $8, %%edi \n\t" "dec %%ecx     \n\t"	/* decrease loop counter */
      "jnz 1b         \n\t"     /* check loop termination, proceed if required */
       "emms          \n\t"	/* exit MMX state */
      "popa                   \n\t":"=m" (Dest)	/* %0 */
      :"m"(Src2),		/* %1 */
      "m"(Src1),		/* %2 */
      "m"(length)		/* %3 */
	);
#endif
    return (0);
}

int SDL_imageFilterSub(unsigned char *Src1, unsigned char *Src2, unsigned char *Dest, int length)
{
    unsigned int i, istart;
    unsigned char *cursrc1, *cursrc2, *curdst;
    int result;

    if ((SDL_imageFilterMMXdetect()) && (length > 7)) {
	/* MMX routine */
	SDL_imageFilterSubMMX(Src1, Src2, Dest, length);

	/* Check for unaligned bytes */
	if ((length & 7) > 0) {
	    /* Setup to process unaligned bytes */
	    istart = length & 0xfffffff8;
	    cursrc1 = &Src1[istart];
	    cursrc2 = &Src2[istart];
	    curdst = &Dest[istart];
	} else {
	    /* No unaligned bytes - we are done */
	    return (0);
	}
    } else {
	/* Setup to process whole image */
	istart = 0;
	cursrc1 = Src1;
	cursrc2 = Src2;
	curdst = Dest;
    }

    /* C routine to process image */
    for (i = istart; i < length; i++) {
	result = (int) *cursrc1 - (int) *cursrc2;
	if (result < 0)
	    result = 0;
	*curdst = (unsigned char) result;
	/* Advance pointers */
	cursrc1++;
	cursrc2++;
	curdst++;
    }

    return (0);
}

/*  SDL_imageFilterAbsDiff: D = | S1 - S2 | */
int SDL_imageFilterAbsDiffMMX(unsigned char *Src1, unsigned char *Src2, unsigned char *Dest, int length)
{
#ifdef USE_MMX
    asm volatile
     ("pusha		     \n\t" "mov %2, %%eax \n\t"	/* load Src1 address into eax */
      "mov %1, %%ebx \n\t"	/* load Src2 address into ebx */
      "mov %0, %%edi \n\t"	/* load Dest address into edi */
       "mov %3, %%ecx \n\t"	/* load loop counter (SIZE) into ecx */
      "shr $3, %%ecx \n\t"	/* counter/8 (MMX loads 8 bytes at a time) */
       ".align 16       \n\t"	/* 16 byte allignment of the loop entry */
      "1: movq (%%eax), %%mm1 \n\t"     /* load 8 bytes from Src1 into mm1 */
      "movq    (%%ebx), %%mm2 \n\t"	/* load 8 bytes from Src2 into mm2 */
      "psubusb (%%ebx), %%mm1 \n\t"	/* mm1=Src1-Src2 (sub 8 bytes with saturation) */
      "psubusb (%%eax), %%mm2 \n\t"	/* mm2=Src2-Src1 (sub 8 bytes with saturation) */
      "por       %%mm2, %%mm1 \n\t"	/* combine both mm2 and mm1 results */
      "movq    %%mm1, (%%edi) \n\t"	/* store result in Dest */
       "add $8, %%eax \n\t"	/* increase Src1, Src2 and Dest  */
      "add $8, %%ebx \n\t"	/* register pointers by 8 */
      "add $8, %%edi \n\t" "dec %%ecx     \n\t"	/* decrease loop counter */
      "jnz 1b        \n\t"      /* check loop termination, proceed if required */
       "emms          \n\t"	/* exit MMX state */
      "popa                   \n\t":"=m" (Dest)	/* %0 */
      :"m"(Src2),		/* %1 */
      "m"(Src1),		/* %2 */
      "m"(length)		/* %3 */
	);
#endif
    return (0);
}

/*  SDL_imageFilterAbsDiff: D = | S1 - S2 | */
int SDL_imageFilterAbsDiff(unsigned char *Src1, unsigned char *Src2, unsigned char *Dest, int length)
{
    unsigned int i, istart;
    unsigned char *cursrc1, *cursrc2, *curdst;
    int result;

    if ((SDL_imageFilterMMXdetect()) && (length > 7)) {
	/* MMX routine */
	SDL_imageFilterAbsDiffMMX(Src1, Src2, Dest, length);

	/* Check for unaligned bytes */
	if ((length & 7) > 0) {
	    /* Setup to process unaligned bytes */
	    istart = length & 0xfffffff8;
	    cursrc1 = &Src1[istart];
	    cursrc2 = &Src2[istart];
	    curdst = &Dest[istart];
	} else {
	    /* No unaligned bytes - we are done */
	    return (0);
	}
    } else {
	/* Setup to process whole image */
	istart = 0;
	cursrc1 = Src1;
	cursrc2 = Src2;
	curdst = Dest;
    }

    /* C routine to process image */
    for (i = istart; i < length; i++) {
	result = abs((int) *cursrc1 - (int) *cursrc2);
	*curdst = (unsigned char) result;
	/* Advance pointers */
	cursrc1++;
	cursrc2++;
	curdst++;
    }

    return (0);
}

/*  SDL_imageFilterMult: D = saturation255(S1 * S2) */
int SDL_imageFilterMultMMX(unsigned char *Src1, unsigned char *Src2, unsigned char *Dest, int length)
{
#ifdef USE_MMX
    asm volatile
     ("pusha		     \n\t" "mov %2, %%eax \n\t"	/* load Src1 address into eax */
      "mov %1, %%ebx \n\t"	/* load Src2 address into ebx */
      "mov %0, %%edi \n\t"	/* load Dest address into edi */
       "mov %3, %%ecx \n\t"	/* load loop counter (SIZE) into ecx */
      "shr $3, %%ecx \n\t"	/* counter/8 (MMX loads 8 bytes at a time) */
       "pxor      %%mm0, %%mm0 \n\t"	/* zero mm0 register */
       ".align 16       \n\t"	/* 16 byte allignment of the loop entry */
      "1: movq (%%eax), %%mm1 \n\t"     /* load 8 bytes from Src1 into mm1 */
      "movq    (%%ebx), %%mm3 \n\t"	/* load 8 bytes from Src2 into mm3 */
      "movq      %%mm1, %%mm2 \n\t"	/* copy mm1 into mm2 */
      "movq      %%mm3, %%mm4 \n\t"	/* copy mm3 into mm4  */
       "punpcklbw %%mm0, %%mm1 \n\t"	/* unpack low  bytes of Src1 into words */
      "punpckhbw %%mm0, %%mm2 \n\t"	/* unpack high bytes of Src1 into words */
      "punpcklbw %%mm0, %%mm3 \n\t"	/* unpack low  bytes of Src2 into words */
      "punpckhbw %%mm0, %%mm4 \n\t"	/* unpack high bytes of Src2 into words */
       "pmullw    %%mm3, %%mm1 \n\t"	/* mul low  bytes of Src1 and Src2  */
      "pmullw    %%mm4, %%mm2 \n\t"	/* mul high bytes of Src1 and Src2 */
      /* Take abs value of the results (signed words) */
       "movq      %%mm1, %%mm5 \n\t"	/* copy mm1 into mm5 */
      "movq      %%mm2, %%mm6 \n\t"	/* copy mm2 into mm6 */
       "psraw       $15, %%mm5 \n\t"	/* fill mm5 words with word sign bit */
      "psraw       $15, %%mm6 \n\t"	/* fill mm6 words with word sign bit */
       "pxor      %%mm5, %%mm1 \n\t"	/* take 1's compliment of only neg. words */
      "pxor      %%mm6, %%mm2 \n\t"	/* take 1's compliment of only neg. words */
       "psubsw    %%mm5, %%mm1 \n\t"	/* add 1 to only neg. words, W-(-1) or W-0 */
      "psubsw    %%mm6, %%mm2 \n\t"	/* add 1 to only neg. words, W-(-1) or W-0 */
       "packuswb  %%mm2, %%mm1 \n\t"	/* pack words back into bytes with saturation */
       "movq    %%mm1, (%%edi) \n\t"	/* store result in Dest */
       "add $8, %%eax \n\t"	/* increase Src1, Src2 and Dest  */
      "add $8, %%ebx \n\t"	/* register pointers by 8 */
      "add $8, %%edi \n\t" "dec %%ecx     \n\t"	/* decrease loop counter */
      "jnz 1b        \n\t"      /* check loop termination, proceed if required */
       "emms          \n\t"	/* exit MMX state */
      "popa \n\t":"=m" (Dest)	/* %0 */
      :"m"(Src2),		/* %1 */
      "m"(Src1),		/* %2 */
      "m"(length)		/* %3 */
	);
#endif
    return (0);
}

/*  SDL_imageFilterMult: D = saturation255(S1 * S2) */
int SDL_imageFilterMult(unsigned char *Src1, unsigned char *Src2, unsigned char *Dest, int length)
{
    unsigned int i, istart;
    unsigned char *cursrc1, *cursrc2, *curdst;
    int result;

    if ((SDL_imageFilterMMXdetect()) && (length > 7)) {
	/* MMX routine */
	SDL_imageFilterMultMMX(Src1, Src2, Dest, length);

	/* Check for unaligned bytes */
	if ((length & 7) > 0) {
	    /* Setup to process unaligned bytes */
	    istart = length & 0xfffffff8;
	    cursrc1 = &Src1[istart];
	    cursrc2 = &Src2[istart];
	    curdst = &Dest[istart];
	} else {
	    /* No unaligned bytes - we are done */
	    return (0);
	}
    } else {
	/* Setup to process whole image */
	istart = 0;
	cursrc1 = Src1;
	cursrc2 = Src2;
	curdst = Dest;
    }

    /* C routine to process image */
    for (i = istart; i < length; i++) {

	/* NOTE: this is probably wrong - dunno what the MMX code does */

	result = (int) *cursrc1 * (int) *cursrc2;
	if (result > 255)
	    result = 255;
	*curdst = (unsigned char) result;
	/* Advance pointers */
	cursrc1++;
	cursrc2++;
	curdst++;
    }

    return (0);
}

/*  SDL_imageFilterMultNor: D = S1 * S2  (non-MMX) */
int SDL_imageFilterMultNorASM(unsigned char *Src1, unsigned char *Src2, unsigned char *Dest, int length)
{
#ifdef USE_MMX
    asm volatile
     ("pusha		     \n\t" "mov %2, %%edx \n\t"	/* load Src1 address into edx */
      "mov %1, %%esi \n\t"	/* load Src2 address into esi */
      "mov %0, %%edi \n\t"	/* load Dest address into edi */
       "mov %3, %%ecx \n\t"	/* load loop counter (SIZE) into ecx */
       ".align 16       \n\t"	/* 16 byte allignment of the loop entry */
      "1:mov  (%%edx), %%al \n\t"      /* load a byte from Src1 */
      "mulb (%%esi)       \n\t"	/* mul with a byte from Src2 */
       "mov %%al, (%%edi)  \n\t"       /* move a byte result to Dest */
       "inc %%edx \n\t"		/* increment Src1, Src2, Dest */
      "inc %%esi \n\t"		/* pointer registers by one */
      "inc %%edi \n\t" "dec %%ecx      \n\t"	/* decrease loop counter */
      "jnz 1b         \n\t"     /* check loop termination, proceed if required */
       "popa                   \n\t":"=m" (Dest)	/* %0 */
      :"m"(Src2),		/* %1 */
      "m"(Src1),		/* %2 */
      "m"(length)		/* %3 */
	);
#endif
    return (0);
}

/*  SDL_imageFilterMultNor: D = S1 * S2 */
int SDL_imageFilterMultNor(unsigned char *Src1, unsigned char *Src2, unsigned char *Dest, int length)
{
    unsigned int i, istart;
    unsigned char *cursrc1, *cursrc2, *curdst;
    int result;

    if (SDL_imageFilterMMXdetect()) {
    if (length > 0) {
	/* ASM routine */
	SDL_imageFilterMultNorASM(Src1, Src2, Dest, length);

	/* Check for unaligned bytes */
	if ((length & 7) > 0) {
	    /* Setup to process unaligned bytes */
	    istart = length & 0xfffffff8;
	    cursrc1 = &Src1[istart];
	    cursrc2 = &Src2[istart];
	    curdst = &Dest[istart];
	} else {
	    /* No unaligned bytes - we are done */
	    return (0);
	}
    } else {
	/* No bytes - we are done */
	return (0);
    }
    } else {
	/* Setup to process whole image */
	istart = 0;
	cursrc1 = Src1;
	cursrc2 = Src2;
	curdst = Dest;
    }

    /* C routine to process image */
    for (i = istart; i < length; i++) {
	result = (int) *cursrc1 * (int) *cursrc2;
	*curdst = (unsigned char) result;
	/* Advance pointers */
	cursrc1++;
	cursrc2++;
	curdst++;
    }

    return (0);
}

/*  SDL_imageFilterMultDivby2: D = saturation255(S1/2 * S2) */
int SDL_imageFilterMultDivby2MMX(unsigned char *Src1, unsigned char *Src2, unsigned char *Dest, int length)
{
#ifdef USE_MMX
    asm volatile
     ("pusha \n\t" "mov %2, %%eax \n\t"	/* load Src1 address into eax */
      "mov %1, %%ebx \n\t"	/* load Src2 address into ebx */
      "mov %0, %%edi \n\t"	/* load Dest address into edi */
       "mov %3, %%ecx \n\t"	/* load loop counter (SIZE) into ecx */
      "shr $3, %%ecx \n\t"	/* counter/8 (MMX loads 8 bytes at a time) */
       "pxor      %%mm0, %%mm0 \n\t"	/* zero mm0 register */
       ".align 16       \n\t"	/* 16 byte allignment of the loop entry */
      "1: movq (%%eax), %%mm1 \n\t"	/* load 8 bytes from Src1 into mm1 */
      "movq    (%%ebx), %%mm3 \n\t"	/* load 8 bytes from Src2 into mm3 */
      "movq      %%mm1, %%mm2 \n\t"	/* copy mm1 into mm2 */
      "movq      %%mm3, %%mm4 \n\t"	/* copy mm3 into mm4  */
       "punpcklbw %%mm0, %%mm1 \n\t"	/* unpack low  bytes of Src1 into words */
      "punpckhbw %%mm0, %%mm2 \n\t"	/* unpack high bytes of Src1 into words */
      "punpcklbw %%mm0, %%mm3 \n\t"	/* unpack low  bytes of Src2 into words */
      "punpckhbw %%mm0, %%mm4 \n\t"	/* unpack high bytes of Src2 into words */
       "psrlw        $1, %%mm1 \n\t"	/* divide mm1 words by 2, Src1 low bytes */
      "psrlw        $1, %%mm2 \n\t"	/* divide mm2 words by 2, Src1 high bytes */
       "pmullw    %%mm3, %%mm1 \n\t"	/* mul low  bytes of Src1 and Src2  */
      "pmullw    %%mm4, %%mm2 \n\t"	/* mul high bytes of Src1 and Src2 */
       "packuswb  %%mm2, %%mm1 \n\t"	/* pack words back into bytes with saturation */
       "movq    %%mm1, (%%edi) \n\t"	/* store result in Dest */
       "add $8, %%eax \n\t"	/* increase Src1, Src2 and Dest  */
      "add $8, %%ebx \n\t"	/* register pointers by 8 */
      "add $8, %%edi \n\t" "dec %%ecx     \n\t"	/* decrease loop counter */
      "jnz 1b        \n\t"	/* check loop termination, proceed if required */
       "emms          \n\t"	/* exit MMX state */
      "popa \n\t":"=m" (Dest)	/* %0 */
      :"m"(Src2),		/* %1 */
      "m"(Src1),		/* %2 */
      "m"(length)		/* %3 */
	);
#endif
    return (0);
}

/*  SDL_imageFilterMultDivby2: D = saturation255(S1/2 * S2) */
int SDL_imageFilterMultDivby2(unsigned char *Src1, unsigned char *Src2, unsigned char *Dest, int length)
{
    unsigned int i, istart;
    unsigned char *cursrc1, *cursrc2, *curdst;
    int result;

    if ((SDL_imageFilterMMXdetect()) && (length > 7)) {
	/* MMX routine */
	SDL_imageFilterMultDivby2MMX(Src1, Src2, Dest, length);

	/* Check for unaligned bytes */
	if ((length & 7) > 0) {
	    /* Setup to process unaligned bytes */
	    istart = length & 0xfffffff8;
	    cursrc1 = &Src1[istart];
	    cursrc2 = &Src2[istart];
	    curdst = &Dest[istart];
	} else {
	    /* No unaligned bytes - we are done */
	    return (0);
	}
    } else {
	/* Setup to process whole image */
	istart = 0;
	cursrc1 = Src1;
	cursrc2 = Src2;
	curdst = Dest;
    }

    /* C routine to process image */
    for (i = istart; i < length; i++) {
	result = ((int) *cursrc1 / 2) * (int) *cursrc2;
	if (result > 255)
	    result = 255;
	*curdst = (unsigned char) result;
	/* Advance pointers */
	cursrc1++;
	cursrc2++;
	curdst++;
    }

    return (0);
}

/*  SDL_imageFilterMultDivby4: D = saturation255(S1/2 * S2/2) */
int SDL_imageFilterMultDivby4MMX(unsigned char *Src1, unsigned char *Src2, unsigned char *Dest, int length)
{
#ifdef USE_MMX
    asm volatile
     ("pusha		     \n\t" "mov %2, %%eax \n\t"	/* load Src1 address into eax */
      "mov %1, %%ebx \n\t"	/* load Src2 address into ebx */
      "mov %0, %%edi \n\t"	/* load Dest address into edi */
       "mov %3, %%ecx \n\t"	/* load loop counter (SIZE) into ecx */
      "shr $3, %%ecx \n\t"	/* counter/8 (MMX loads 8 bytes at a time) */
       "pxor      %%mm0, %%mm0 \n\t"	/* zero mm0 register */
       ".align 16       \n\t"	/* 16 byte allignment of the loop entry */
      "1: movq (%%eax), %%mm1 \n\t"	/* load 8 bytes from Src1 into mm1 */
      "movq    (%%ebx), %%mm3 \n\t"	/* load 8 bytes from Src2 into mm3 */
      "movq      %%mm1, %%mm2 \n\t"	/* copy mm1 into mm2 */
      "movq      %%mm3, %%mm4 \n\t"	/* copy mm3 into mm4  */
       "punpcklbw %%mm0, %%mm1 \n\t"	/* unpack low  bytes of Src1 into words */
      "punpckhbw %%mm0, %%mm2 \n\t"	/* unpack high bytes of Src1 into words */
      "punpcklbw %%mm0, %%mm3 \n\t"	/* unpack low  bytes of Src2 into words */
      "punpckhbw %%mm0, %%mm4 \n\t"	/* unpack high bytes of Src2 into words */
       "psrlw        $1, %%mm1 \n\t"	/* divide mm1 words by 2, Src1 low bytes */
      "psrlw        $1, %%mm2 \n\t"	/* divide mm2 words by 2, Src1 high bytes */
      "psrlw        $1, %%mm3 \n\t"	/* divide mm3 words by 2, Src2 low bytes */
      "psrlw        $1, %%mm4 \n\t"	/* divide mm4 words by 2, Src2 high bytes */
       "pmullw    %%mm3, %%mm1 \n\t"	/* mul low  bytes of Src1 and Src2  */
      "pmullw    %%mm4, %%mm2 \n\t"	/* mul high bytes of Src1 and Src2 */
       "packuswb  %%mm2, %%mm1 \n\t"	/* pack words back into bytes with saturation */
       "movq    %%mm1, (%%edi) \n\t"	/* store result in Dest */
       "add $8, %%eax \n\t"	/* increase Src1, Src2 and Dest  */
      "add $8, %%ebx \n\t"	/* register pointers by 8 */
      "add $8, %%edi \n\t" "dec %%ecx     \n\t"	/* decrease loop counter */
      "jnz 1b        \n\t"	/* check loop termination, proceed if required */
       "emms          \n\t"	/* exit MMX state */
      "popa                   \n\t":"=m" (Dest)	/* %0 */
      :"m"(Src2),		/* %1 */
      "m"(Src1),		/* %2 */
      "m"(length)		/* %3 */
	);
#endif
    return (0);
}

/*  SDL_imageFilterMultDivby4: D = saturation255(S1/2 * S2/2) */
int SDL_imageFilterMultDivby4(unsigned char *Src1, unsigned char *Src2, unsigned char *Dest, int length)
{
    unsigned int i, istart;
    unsigned char *cursrc1, *cursrc2, *curdst;
    int result;

    if ((SDL_imageFilterMMXdetect()) && (length > 7)) {
	/* MMX routine */
	SDL_imageFilterMultDivby4MMX(Src1, Src2, Dest, length);

	/* Check for unaligned bytes */
	if ((length & 7) > 0) {
	    /* Setup to process unaligned bytes */
	    istart = length & 0xfffffff8;
	    cursrc1 = &Src1[istart];
	    cursrc2 = &Src2[istart];
	    curdst = &Dest[istart];
	} else {
	    /* No unaligned bytes - we are done */
	    return (0);
	}
    } else {
	/* Setup to process whole image */
	istart = 0;
	cursrc1 = Src1;
	cursrc2 = Src2;
	curdst = Dest;
    }

    /* C routine to process image */
    for (i = istart; i < length; i++) {
	result = ((int) *cursrc1 / 2) * ((int) *cursrc2 / 2);
	if (result > 255)
	    result = 255;
	*curdst = (unsigned char) result;
	/* Advance pointers */
	cursrc1++;
	cursrc2++;
	curdst++;
    }

    return (0);
}

/*  SDL_imageFilterBitAnd: D = S1 & S2 */
int SDL_imageFilterBitAndMMX(unsigned char *Src1, unsigned char *Src2, unsigned char *Dest, int length)
{
#ifdef USE_MMX
    asm volatile
     ("pusha		     \n\t" "mov %2, %%eax \n\t"	/* load Src1 address into eax */
      "mov %1, %%ebx \n\t"	/* load Src2 address into ebx */
      "mov %0, %%edi \n\t"	/* load Dest address into edi */
       "mov %3, %%ecx \n\t"	/* load loop counter (SIZE) into ecx */
      "shr $3, %%ecx \n\t"	/* counter/8 (MMX loads 8 bytes at a time) */
       ".align 16       \n\t"	/* 16 byte allignment of the loop entry */
      "1: movq (%%eax), %%mm1 \n\t"	/* load 8 bytes from Src1 into mm1 */
      "pand    (%%ebx), %%mm1 \n\t"	/* mm1=Src1&Src2 */
      "movq    %%mm1, (%%edi) \n\t"	/* store result in Dest */
       "add $8, %%eax \n\t"	/* increase Src1, Src2 and Dest  */
      "add $8, %%ebx \n\t"	/* register pointers by 8 */
      "add $8, %%edi \n\t" "dec %%ecx     \n\t"	/* decrease loop counter */
      "jnz 1b        \n\t"	/* check loop termination, proceed if required */
       "emms          \n\t"	/* exit MMX state */
      "popa                   \n\t":"=m" (Dest)	/* %0 */
      :"m"(Src2),		/* %1 */
      "m"(Src1),		/* %2 */
      "m"(length)		/* %3 */
	);
#endif
    return (0);
}

/*  SDL_imageFilterBitAnd: D = S1 & S2 */
int SDL_imageFilterBitAnd(unsigned char *Src1, unsigned char *Src2, unsigned char *Dest, int length)
{
    unsigned int i, istart;
    unsigned char *cursrc1, *cursrc2, *curdst;

    if ((SDL_imageFilterMMXdetect()>0) && (length>7)) {
/*  if (length > 7) { */
	/* Call MMX routine */

	SDL_imageFilterBitAndMMX(Src1, Src2, Dest, length);

	/* Check for unaligned bytes */
	if ((length & 7) > 0) {

	    /* Setup to process unaligned bytes */
	    istart = length & 0xfffffff8;
	    cursrc1 = &Src1[istart];
	    cursrc2 = &Src2[istart];
	    curdst = &Dest[istart];
	} else {
	    /* No unaligned bytes - we are done */
	    return (0);
	}
    } else {
	/* Setup to process whole image */
	istart = 0;
	cursrc1 = Src1;
	cursrc2 = Src2;
	curdst = Dest;
    }

    /* C routine to process image */
    for (i = istart; i < length; i++) {
	*curdst = (*cursrc1) & (*cursrc2);
	/* Advance pointers */
	cursrc1++;
	cursrc2++;
	curdst++;
    }

    return (0);
}

/*  SDL_imageFilterBitOr: D = S1 | S2 */
int SDL_imageFilterBitOrMMX(unsigned char *Src1, unsigned char *Src2, unsigned char *Dest, int length)
{
#ifdef USE_MMX
    asm volatile
     ("pusha		     \n\t" "mov %2, %%eax \n\t"	/* load Src1 address into eax */
      "mov %1, %%ebx \n\t"	/* load Src2 address into ebx */
      "mov %0, %%edi \n\t"	/* load Dest address into edi */
       "mov %3, %%ecx \n\t"	/* load loop counter (SIZE) into ecx */
      "shr $3, %%ecx \n\t"	/* counter/8 (MMX loads 8 bytes at a time) */
       ".align 16       \n\t"	/* 16 byte allignment of the loop entry */
      "1: movq (%%eax), %%mm1 \n\t"	/* load 8 bytes from Src1 into mm1 */
      "por     (%%ebx), %%mm1 \n\t"	/* mm1=Src1|Src2 */
      "movq    %%mm1, (%%edi) \n\t"	/* store result in Dest */
       "add $8, %%eax \n\t"	/* increase Src1, Src2 and Dest  */
      "add $8, %%ebx \n\t"	/* register pointers by 8 */
      "add $8, %%edi \n\t" "dec %%ecx     \n\t"	/* decrease loop counter */
      "jnz 1b        \n\t"	/* check loop termination, proceed if required */
       "emms          \n\t"	/* exit MMX state */
      "popa                   \n\t":"=m" (Dest)	/* %0 */
      :"m"(Src2),		/* %1 */
      "m"(Src1),		/* %2 */
      "m"(length)		/* %3 */
	);
#endif
    return (0);
}

/*  SDL_imageFilterBitOr: D = S1 | S2 */
int SDL_imageFilterBitOr(unsigned char *Src1, unsigned char *Src2, unsigned char *Dest, int length)
{
    unsigned int i, istart;
    unsigned char *cursrc1, *cursrc2, *curdst;

    if ((SDL_imageFilterMMXdetect()) && (length > 7)) {

	/* MMX routine */
	SDL_imageFilterBitOrMMX(Src1, Src2, Dest, length);

	/* Check for unaligned bytes */
	if ((length & 7) > 0) {
	    /* Setup to process unaligned bytes */
	    istart = length & 0xfffffff8;
	    cursrc1 = &Src1[istart];
	    cursrc2 = &Src2[istart];
	    curdst = &Dest[istart];
	} else {
	    /* No unaligned bytes - we are done */
	    return (0);
	}
    } else {
	/* Setup to process whole image */
	istart = 0;
	cursrc1 = Src1;
	cursrc2 = Src2;
	curdst = Dest;
    }

    /* C routine to process image */
    for (i = istart; i < length; i++) {
	*curdst = *cursrc1 | *cursrc2;
	/* Advance pointers */
	cursrc1++;
	cursrc2++;
	curdst++;
    }
    return (0);
}

/*  SDL_imageFilterDiv: D = S1 / S2  (non-MMX) */
int SDL_imageFilterDivASM(unsigned char *Src1, unsigned char *Src2, unsigned char *Dest, int length)
{
#ifdef USE_MMX
    asm volatile
     ("pusha \n\t" "mov %2, %%edx \n\t"	/* load Src1 address into edx */
      "mov %1, %%esi \n\t"	/* load Src2 address into esi */
      "mov %0, %%edi \n\t"	/* load Dest address into edi */
       "mov %3, %%ecx \n\t"	/* load loop counter (SIZE) into ecx */
       ".align 16     \n\t"	/* 16 byte allignment of the loop entry */
      "1: mov (%%esi), %%bl  \n\t"	/* load a byte from Src2 */
      "cmp       $0, %%bl  \n\t"	/* check if it zero */
      "jnz 2f              \n\t" "movb  $255, (%%edi) \n\t"	/* division by zero = 255 !!! */
      "jmp 3f              \n\t" "2:                  \n\t" "xor   %%ah, %%ah    \n\t"	/* prepare AX, zero AH register */
      "mov   (%%edx), %%al \n\t"	/* load a byte from Src1 into AL */
      "div   %%bl          \n\t"	/* divide AL by BL */
      "mov   %%al, (%%edi) \n\t"	/* move a byte result to Dest */
      "3: inc %%edx        \n\t"	/* increment Src1, Src2, Dest */
      "inc %%esi \n\t"		/* pointer registers by one */
      "inc %%edi \n\t" "dec %%ecx    \n\t"	/* decrease loop counter */
      "jnz 1b       \n\t"	/* check loop termination, proceed if required */
       "popa \n\t":"=m" (Dest)	/* %0 */
      :"m"(Src2),		/* %1 */
      "m"(Src1),		/* %2 */
      "m"(length)		/* %3 */
	);
#endif
    return (0);
}

/*  SDL_imageFilterDiv: D = S1 / S2  (non-MMX!) */
int SDL_imageFilterDiv(unsigned char *Src1, unsigned char *Src2, unsigned char *Dest, int length)
{
    unsigned int i, istart;
    unsigned char *cursrc1, *cursrc2, *curdst;
    int result;

    if (SDL_imageFilterMMXdetect()) {
    if (length > 0) {
	/* Call ASM routine */
	SDL_imageFilterDivASM(Src1, Src2, Dest, length);

	/* Never unaligned bytes - we are done */
	return (0);
    } else {
	return (-1);
    }
    } else {
	/* Setup to process whole image */
	istart = 0;
	cursrc1 = Src1;
	cursrc2 = Src2;
	curdst = Dest;
    }

    /* C routine to process image */
    for (i = istart; i < length; i++) {
	result = (int) *cursrc1 / (int) *cursrc2;
	*curdst = (unsigned char) result;
	/* Advance pointers */
	cursrc1++;
	cursrc2++;
	curdst++;
    }

    return (0);
}

/* ------------------------------------------------------------------------------------ */

/*  SDL_imageFilterBitNegation: D = !S */
int SDL_imageFilterBitNegationMMX(unsigned char *Src1, unsigned char *Dest, int length)
{
#ifdef USE_MMX
    asm volatile
     ("pusha		     \n\t" "pcmpeqb   %%mm1, %%mm1 \n\t"	/* generate all 1's in mm1 */
       "mov %1, %%eax \n\t"	/* load Src1 address into eax */
      "mov %0, %%edi \n\t"	/* load Dest address into edi */
       "mov %2, %%ecx \n\t"	/* load loop counter (SIZE) into ecx */
      "shr $3, %%ecx \n\t"	/* counter/8 (MMX loads 8 bytes at a time) */
       ".align 16       \n\t"	/* 16 byte allignment of the loop entry */
      "1: movq (%%eax), %%mm0 \n\t"	/* load 8 bytes from Src1 into mm1 */
      "pxor      %%mm1, %%mm0 \n\t"	/* negate mm0 by xoring with mm1 */
      "movq    %%mm0, (%%edi) \n\t"	/* store result in Dest */
       "add $8, %%eax \n\t"	/* increase Src1, Src2 and Dest  */
      "add $8, %%edi \n\t" "dec %%ecx     \n\t"	/* decrease loop counter */
      "jnz 1b        \n\t"	/* check loop termination, proceed if required */
       "emms          \n\t"	/* exit MMX state */
      "popa                   \n\t":"=m" (Dest)	/* %0 */
      :"m"(Src1),		/* %1 */
      "m"(length)		/* %2 */
	);
#endif
    return (0);
}

/*  SDL_imageFilterBitNegation: D = !S */
int SDL_imageFilterBitNegation(unsigned char *Src1, unsigned char *Dest, int length)
{
    unsigned int i, istart;
    unsigned char *cursrc1, *curdst;

    if ((SDL_imageFilterMMXdetect()) && (length > 7)) {
	/* MMX routine */
	SDL_imageFilterBitNegationMMX(Src1, Dest, length);

	/* Check for unaligned bytes */
	if ((length & 7) > 0) {
	    /* Setup to process unaligned bytes */
	    istart = length & 0xfffffff8;
	    cursrc1 = &Src1[istart];
	    curdst = &Dest[istart];
	} else {
	    /* No unaligned bytes - we are done */
	    return (0);
	}
    } else {
	/* Setup to process whole image */
	istart = 0;
	cursrc1 = Src1;
	curdst = Dest;
    }

    /* C routine to process image */
    for (i = istart; i < length; i++) {
	*curdst = ~(*cursrc1);
	/* Advance pointers */
	cursrc1++;
	curdst++;
    }

    return (0);
}

/*  SDL_imageFilterAddByteMMX: D = saturation255(S + C) */
int SDL_imageFilterAddByteMMX(unsigned char *Src1, unsigned char *Dest, int length, unsigned char C)
{
#ifdef USE_MMX
    asm volatile
     ("pusha		     \n\t"
      /* ** Duplicate C in 8 bytes of MM1 ** */
      "mov           %3, %%al \n\t"	/* load C into AL */
      "mov         %%al, %%ah \n\t"	/* copy AL into AH */
      "mov         %%ax, %%bx \n\t"	/* copy AX into BX */
      "shl         $16, %%eax \n\t"	/* shift 2 bytes of EAX left */
      "mov         %%bx, %%ax \n\t"	/* copy BX into AX */
      "movd      %%eax, %%mm1 \n\t"	/* copy EAX into MM1 */
      "movd      %%eax, %%mm2 \n\t"	/* copy EAX into MM2 */
      "punpckldq %%mm2, %%mm1 \n\t"	/* fill higher bytes of MM1 with C */
       "mov          %1, %%eax \n\t"	/* load Src1 address into eax */
      "mov          %0, %%edi \n\t"	/* load Dest address into edi */
      "mov          %2, %%ecx \n\t"	/* load loop counter (SIZE) into ecx */
      "shr          $3, %%ecx \n\t"	/* counter/8 (MMX loads 8 bytes at a time) */
       ".align 16              \n\t"	/* 16 byte allignment of the loop entry */
      "1:                     \n\t" 
      "movq    (%%eax), %%mm0 \n\t"	/* load 8 bytes from Src1 into MM0 */
      "paddusb   %%mm1, %%mm0 \n\t"	/* MM0=SrcDest+C (add 8 bytes with saturation) */
      "movq    %%mm0, (%%edi) \n\t"	/* store result in Dest */
       "add          $8, %%eax \n\t"	/* increase Dest register pointer by 8 */
      "add          $8, %%edi \n\t"	/* increase Dest register pointer by 8 */
      "dec              %%ecx \n\t"	/* decrease loop counter */
      "jnz                 1b \n\t"	/* check loop termination, proceed if required */
       "emms                   \n\t"	/* exit MMX state */
      "popa                   \n\t":"=m" (Dest)	/* %0 */
      :"m"(Src1),		/* %1 */
      "m"(length),		/* %2 */
      "m"(C)			/* %3 */
	);
#endif
    return (0);
}

/*  SDL_imageFilterAddByte: D = saturation255(S + C) */
int SDL_imageFilterAddByte(unsigned char *Src1, unsigned char *Dest, int length, unsigned char C)
{
    unsigned int i, istart;
    int iC;
    unsigned char *cursrc1, *curdest;
    int result;

    if ((SDL_imageFilterMMXdetect()) && (length > 7)) {

	/* MMX routine */
	SDL_imageFilterAddByteMMX(Src1, Dest, length, C);

	/* Check for unaligned bytes */
	if ((length & 7) > 0) {
	    /* Setup to process unaligned bytes */
	    istart = length & 0xfffffff8;
	    cursrc1 = &Src1[istart];
	    curdest = &Dest[istart];
	} else {
	    /* No unaligned bytes - we are done */
	    return (0);
	}
    } else {
	/* Setup to process whole image */
	istart = 0;
	cursrc1 = Src1;
	curdest = Dest;
    }

    /* C routine to process image */
    iC = (int) C;
    for (i = istart; i < length; i++) {
	result = (int) *cursrc1 + iC;
	if (result > 255)
	    result = 255;
	*curdest = (unsigned char) result;
	/* Advance pointers */
	cursrc1++;
	curdest++;
    }
    return (0);
}

/*  SDL_imageFilterAddUintMMX: D = saturation255((S + (uint)C), Cs=swap_32(C) */
int SDL_imageFilterAddUintMMX(unsigned char *Src1, unsigned char *Dest, int length, unsigned int C, unsigned int Cs)
{
#ifdef USE_MMX
    asm volatile
     ("pusha		     \n\t"
      /* ** Duplicate (int)C in 8 bytes of MM1 ** */
      "mov          %3, %%eax \n\t"	/* load C into EAX */
      "movd      %%eax, %%mm1 \n\t"	/* copy EAX into MM1 */
      "mov          %4, %%eax \n\t"	/* load Cs into EAX */
      "movd      %%eax, %%mm2 \n\t"	/* copy EAX into MM2 */
      "punpckldq %%mm2, %%mm1 \n\t"	/* fill higher bytes of MM1 with C */
      "mov          %1, %%eax \n\t"	/* load Src1 address into eax */
      "mov          %0, %%edi \n\t"	/* load Dest address into edi */
      "mov          %2, %%ecx \n\t"	/* load loop counter (SIZE) into ecx */
      "shr          $3, %%ecx \n\t"	/* counter/8 (MMX loads 8 bytes at a time) */
       ".align 16              \n\t"	/* 16 byte allignment of the loop entry */
      "1:                     \n\t" 
      "movq    (%%eax), %%mm0 \n\t"	/* load 8 bytes from SrcDest into MM0 */
      "paddusb   %%mm1, %%mm0 \n\t"	/* MM0=SrcDest+C (add 8 bytes with saturation) */
      "movq    %%mm0, (%%edi) \n\t"	/* store result in SrcDest */
      "add          $8, %%eax \n\t"	/* increase Src1 register pointer by 8 */
      "add          $8, %%edi \n\t"	/* increase Dest register pointer by 8 */
      "dec              %%ecx \n\t"	/* decrease loop counter */
      "jnz                 1b \n\t"	/* check loop termination, proceed if required */
       "emms                   \n\t"	/* exit MMX state */
      "popa                   \n\t":"=m" (Dest)	/* %0 */
      :"m"(Src1),		/* %1 */
      "m"(length),		/* %2 */
      "m"(C),			/* %3 */
      "m"(Cs)			/* %4 */
	);
#endif
    return (0);
}

/*  SDL_imageFilterAddUint: D = saturation0(S - (Uint)C) */
int SDL_imageFilterAddUint(unsigned char *Src1, unsigned char *Dest, int length, unsigned int C)
{
    unsigned int i, j, istart, D;
    int iC[4];
    unsigned char *cursrc1;
    unsigned char *curdest;
    int result;

    if ((SDL_imageFilterMMXdetect()) && (length > 7)) {

	/* MMX routine */
	D=swap_32(C);
	SDL_imageFilterAddUintMMX(Src1, Dest, length, C, D);

	/* Check for unaligned bytes */
	if ((length & 7) > 0) {
	    /* Setup to process unaligned bytes */
	    istart = length & 0xfffffff8;
	    cursrc1 = &Src1[istart];
	    curdest = &Dest[istart];
	} else {
	    /* No unaligned bytes - we are done */
	    return (0);
	}
    } else {
	/* Setup to process whole image */
	istart = 0;
	cursrc1 = Src1;
	curdest = Dest;
    }

    /* C routine to process bytes */
    iC[3] = (int) ((C >> 24) & 0xff);
    iC[2] = (int) ((C >> 16) & 0xff);
    iC[1] = (int) ((C >>  8) & 0xff);
    iC[0] = (int) ((C >>  0) & 0xff);
    for (i = istart; i < length; i += 4) {
     for (j = 0; j < 4; j++) {
      if ((i+j)<length) {
	result = (int) *cursrc1 + iC[j];
	if (result > 255) result = 255;
	*curdest = (unsigned char) result;
	/* Advance pointers */
	cursrc1++;
	curdest++;
      }
     }
    }
    return (0);
}


/*  SDL_imageFilterAddByteToHalfMMX: D = saturation255(S/2 + C) */
int SDL_imageFilterAddByteToHalfMMX(unsigned char *Src1, unsigned char *Dest, int length, unsigned char C,
				    unsigned char *Mask)
{
#ifdef USE_MMX
    asm volatile
     ("pusha		     \n\t"
      /* ** Duplicate C in 8 bytes of MM1 ** */
      "mov           %3, %%al \n\t"	/* load C into AL */
      "mov         %%al, %%ah \n\t"	/* copy AL into AH */
      "mov         %%ax, %%bx \n\t"	/* copy AX into BX */
      "shl         $16, %%eax \n\t"	/* shift 2 bytes of EAX left */
      "mov         %%bx, %%ax \n\t"	/* copy BX into AX */
      "movd      %%eax, %%mm1 \n\t"	/* copy EAX into MM1 */
      "movd      %%eax, %%mm2 \n\t"	/* copy EAX into MM2 */
      "punpckldq %%mm2, %%mm1 \n\t"	/* fill higher bytes of MM1 with C */
       "movl         %4, %%edx \n\t"	/* load Mask address into edx */
      "movq    (%%edx), %%mm0 \n\t"	/* load Mask into mm0 */
       "mov          %1, %%eax \n\t"	/* load Src1 address into eax */
      "mov          %0, %%edi \n\t"	/* load Dest address into edi */
      "mov          %2, %%ecx \n\t"	/* load loop counter (SIZE) into ecx */
      "shr          $3, %%ecx \n\t"	/* counter/8 (MMX loads 8 bytes at a time) */
       ".align 16              \n\t"	/* 16 byte allignment of the loop entry */
      "1:                     \n\t" 
      "movq    (%%eax), %%mm2 \n\t"	/* load 8 bytes from Src1 into MM2 */
      "psrlw        $1, %%mm2 \n\t"	/* shift 4 WORDS of MM2 1 bit to the right */
      /*    "pand      %%mm0, %%mm2 \n\t"    // apply Mask to 8 BYTES of MM2 */
      ".byte     0x0f, 0xdb, 0xd0 \n\t" 
      "paddusb   %%mm1, %%mm2 \n\t"	/* MM2=SrcDest+C (add 8 bytes with saturation) */
      "movq    %%mm2, (%%edi) \n\t"	/* store result in Dest */
       "add          $8, %%eax \n\t"	/* increase Src1 register pointer by 8 */
      "add          $8, %%edi \n\t"	/* increase Dest register pointer by 8 */
      "dec              %%ecx \n\t"	/* decrease loop counter */
      "jnz                  1b \n\t"	/* check loop termination, proceed if required */
       "emms                   \n\t"	/* exit MMX state */
      "popa                   \n\t":"=m" (Dest)	/* %0 */
      :"m"(Src1),		/* %1 */
      "m"(length),		/* %2 */
      "m"(C),			/* %3 */
      "m"(Mask)			/* %4 */
	);
#endif
    return (0);
}

/*  SDL_imageFilterAddByteToHalf: D = saturation255(S/2 + C) */
int SDL_imageFilterAddByteToHalf(unsigned char *Src1, unsigned char *Dest, int length, unsigned char C)
{
    static unsigned char Mask[8] = { 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F };
    unsigned int i, istart;
    int iC;
    unsigned char *cursrc1;
    unsigned char *curdest;
    int result;

    if ((SDL_imageFilterMMXdetect()) && (length > 7)) {

	/* MMX routine */
	SDL_imageFilterAddByteToHalfMMX(Src1, Dest, length, C, Mask);

	/* Check for unaligned bytes */
	if ((length & 7) > 0) {
	    /* Setup to process unaligned bytes */
	    istart = length & 0xfffffff8;
	    cursrc1 = &Src1[istart];
	    curdest = &Dest[istart];
	} else {
	    /* No unaligned bytes - we are done */
	    return (0);
	}
    } else {
	/* Setup to process whole image */
	istart = 0;
	cursrc1 = Src1;
	curdest = Dest;
    }

    /* C routine to process image */
    iC = (int) C;
    for (i = istart; i < length; i++) {
	result = (int) (*cursrc1 / 2) + iC;
	if (result > 255)
	    result = 255;
	*curdest = (unsigned char) result;
	/* Advance pointers */
	cursrc1++;
	curdest++;
    }

    return (0);
}

/*  SDL_imageFilterSubByteMMX: D = saturation0(S - C) */
int SDL_imageFilterSubByteMMX(unsigned char *Src1, unsigned char *Dest, int length, unsigned char C)
{
#ifdef USE_MMX
    asm volatile
     ("pusha		     \n\t"
      /* ** Duplicate C in 8 bytes of MM1 ** */
      "mov           %3, %%al \n\t"	/* load C into AL */
      "mov         %%al, %%ah \n\t"	/* copy AL into AH */
      "mov         %%ax, %%bx \n\t"	/* copy AX into BX */
      "shl         $16, %%eax \n\t"	/* shift 2 bytes of EAX left */
      "mov         %%bx, %%ax \n\t"	/* copy BX into AX */
      "movd      %%eax, %%mm1 \n\t"	/* copy EAX into MM1 */
      "movd      %%eax, %%mm2 \n\t"	/* copy EAX into MM2 */
      "punpckldq %%mm2, %%mm1 \n\t"	/* fill higher bytes of MM1 with C */
      "mov          %1, %%eax \n\t"	/* load Src1 address into eax */
      "mov          %0, %%edi \n\t"	/* load Dest address into edi */
      "mov          %2, %%ecx \n\t"	/* load loop counter (SIZE) into ecx */
      "shr          $3, %%ecx \n\t"	/* counter/8 (MMX loads 8 bytes at a time) */
       ".align 16              \n\t"	/* 16 byte allignment of the loop entry */
      "1: movq (%%eax), %%mm0 \n\t"	/* load 8 bytes from SrcDest into MM0 */
      "psubusb   %%mm1, %%mm0 \n\t"	/* MM0=SrcDest-C (sub 8 bytes with saturation) */
      "movq    %%mm0, (%%edi) \n\t"	/* store result in SrcDest */
       "add          $8, %%eax \n\t"	/* increase Src1 register pointer by 8 */
      "add          $8, %%edi \n\t"	/* increase Dest register pointer by 8 */
      "dec              %%ecx \n\t"	/* decrease loop counter */
      "jnz                 1b \n\t"	/* check loop termination, proceed if required */
       "emms                   \n\t"	/* exit MMX state */
      "popa                   \n\t":"=m" (Dest)	/* %0 */
      :"m"(Src1),		/* %1 */
      "m"(length),		/* %2 */
      "m"(C)			/* %3 */
	);
#endif
    return (0);
}

/*  SDL_imageFilterSubByte: D = saturation0(S - C) */
int SDL_imageFilterSubByte(unsigned char *Src1, unsigned char *Dest, int length, unsigned char C)
{
    unsigned int i, istart;
    int iC;
    unsigned char *cursrc1;
    unsigned char *curdest;
    int result;

    if ((SDL_imageFilterMMXdetect()) && (length > 7)) {

	/* MMX routine */
	SDL_imageFilterSubByteMMX(Src1, Dest, length, C);

	/* Check for unaligned bytes */
	if ((length & 7) > 0) {
	    /* Setup to process unaligned bytes */
	    istart = length & 0xfffffff8;
	    cursrc1 = &Src1[istart];
	    curdest = &Dest[istart];
	} else {
	    /* No unaligned bytes - we are done */
	    return (0);
	}
    } else {
	/* Setup to process whole image */
	istart = 0;
	cursrc1 = Src1;
	curdest = Dest;
    }

    /* C routine to process image */
    iC = (int) C;
    for (i = istart; i < length; i++) {
	result = (int) *cursrc1 - iC;
	if (result < 0)
	    result = 0;
	*curdest = (unsigned char) result;
	/* Advance pointers */
	cursrc1++;
	curdest++;
    }
    return (0);
}

/*  SDL_imageFilterSubUintMMX: D = saturation0(S - (uint)C), Cs=swap_32(C) */
int SDL_imageFilterSubUintMMX(unsigned char *Src1, unsigned char *Dest, int length, unsigned int C, unsigned int Cs)
{
#ifdef USE_MMX
    asm volatile
     ("pusha		     \n\t"
      /* ** Duplicate (int)C in 8 bytes of MM1 ** */
      "mov          %3, %%eax \n\t"	/* load C into EAX */
      "movd      %%eax, %%mm1 \n\t"	/* copy EAX into MM1 */
      "mov          %4, %%eax \n\t"	/* load Cs into EAX */
      "movd      %%eax, %%mm2 \n\t"	/* copy EAX into MM2 */
      "punpckldq %%mm2, %%mm1 \n\t"	/* fill higher bytes of MM1 with C */
      "mov          %1, %%eax \n\t"	/* load Src1 address into eax */
      "mov          %0, %%edi \n\t"	/* load Dest address into edi */
      "mov          %2, %%ecx \n\t"	/* load loop counter (SIZE) into ecx */
      "shr          $3, %%ecx \n\t"	/* counter/8 (MMX loads 8 bytes at a time) */
       ".align 16              \n\t"	/* 16 byte allignment of the loop entry */
      "1: movq (%%eax), %%mm0 \n\t"	/* load 8 bytes from SrcDest into MM0 */
      "psubusb   %%mm1, %%mm0 \n\t"	/* MM0=SrcDest-C (sub 8 bytes with saturation) */
      "movq    %%mm0, (%%edi) \n\t"	/* store result in SrcDest */
       "add          $8, %%eax \n\t"	/* increase Src1 register pointer by 8 */
      "add          $8, %%edi \n\t"	/* increase Dest register pointer by 8 */
      "dec              %%ecx \n\t"	/* decrease loop counter */
      "jnz                  1b \n\t"	/* check loop termination, proceed if required */
       "emms                   \n\t"	/* exit MMX state */
      "popa                   \n\t":"=m" (Dest)	/* %0 */
      :"m"(Src1),		/* %1 */
      "m"(length),		/* %2 */
      "m"(C),			/* %3 */
      "m"(Cs)			/* %4 */
	);
#endif
    return (0);
}

/*  SDL_imageFilterSubUint: D = saturation0(S - (Uint)C) */
int SDL_imageFilterSubUint(unsigned char *Src1, unsigned char *Dest, int length, unsigned int C)
{
    unsigned int i, j, istart, D;
    int iC[4];
    unsigned char *cursrc1;
    unsigned char *curdest;
    int result;

    if ((SDL_imageFilterMMXdetect()) && (length > 7)) {

	/* MMX routine */
	D=swap_32(C);
	SDL_imageFilterSubUintMMX(Src1, Dest, length, C, D);

	/* Check for unaligned bytes */
	if ((length & 7) > 0) {
	    /* Setup to process unaligned bytes */
	    istart = length & 0xfffffff8;
	    cursrc1 = &Src1[istart];
	    curdest = &Dest[istart];
	} else {
	    /* No unaligned bytes - we are done */
	    return (0);
	}
    } else {
	/* Setup to process whole image */
	istart = 0;
	cursrc1 = Src1;
	curdest = Dest;
    }

    /* C routine to process image */
    iC[3] = (int) ((C >> 24) & 0xff);
    iC[2] = (int) ((C >> 16) & 0xff);
    iC[1] = (int) ((C >>  8) & 0xff);
    iC[0] = (int) ((C >>  0) & 0xff);
    for (i = istart; i < length; i += 4) {
     for (j = 0; j < 4; j++) {
      if ((i+j)<length) {
	result = (int) *cursrc1 - iC[j];
	if (result < 0) result = 0;
	*curdest = (unsigned char) result;
	/* Advance pointers */
	cursrc1++;
	curdest++;
      }
     }
    }
    return (0);
}


/*  SDL_imageFilterShiftRightMMX: D = saturation0(S >> N) */
int SDL_imageFilterShiftRightMMX(unsigned char *Src1, unsigned char *Dest, int length, unsigned char N,
				 unsigned char *Mask)
{
#ifdef USE_MMX
    asm volatile
     ("pusha		     \n\t" "movl         %4, %%edx \n\t"	/* load Mask address into edx */
      "movq    (%%edx), %%mm0 \n\t"	/* load Mask into mm0 */
       "xor       %%ecx, %%ecx \n\t"	/* zero ECX */
      "mov           %3, %%cl \n\t"	/* load loop counter (N) into CL */
      "movd      %%ecx, %%mm3 \n\t"	/* copy (N) into MM3  */
       "pcmpeqb   %%mm1, %%mm1 \n\t"	/* generate all 1's in mm1 */
       "1:                     \n\t"	/* ** Prepare proper bit-Mask in MM1 ** */
       "psrlw        $1, %%mm1 \n\t"	/* shift 4 WORDS of MM1 1 bit to the right */
      /*    "pand      %%mm0, %%mm1 \n\t"    // apply Mask to 8 BYTES of MM1 */
      ".byte     0x0f, 0xdb, 0xc8 \n\t" 
      "dec               %%cl \n\t"	/* decrease loop counter */
      "jnz                 1b \n\t"	/* check loop termination, proceed if required */
      /* ** Shift all bytes of the image ** */
       "mov          %1, %%eax \n\t"	/* load Src1 address into eax */
      "mov          %0, %%edi \n\t"	/* load Dest address into edi */
      "mov          %2, %%ecx \n\t"	/* load loop counter (SIZE) into ecx */
      "shr          $3, %%ecx \n\t"	/* counter/8 (MMX loads 8 bytes at a time) */
       ".align 16              \n\t"	/* 16 byte allignment of the loop entry */
      "2:                     \n\t" 
      "movq    (%%eax), %%mm0 \n\t"	/* load 8 bytes from SrcDest into MM0 */
      "psrlw     %%mm3, %%mm0 \n\t"	/* shift 4 WORDS of MM0 (N) bits to the right */
      /*    "pand      %%mm1, %%mm0 \n\t"    // apply proper bit-Mask to 8 BYTES of MM0 */
      ".byte     0x0f, 0xdb, 0xc1 \n\t" 
      "movq    %%mm0, (%%edi) \n\t"	/* store result in SrcDest */
      "add          $8, %%eax \n\t"	/* increase Src1 register pointer by 8 */
      "add          $8, %%edi \n\t"	/* increase Dest register pointer by 8 */
      "dec              %%ecx \n\t"	/* decrease loop counter */
      "jnz                 2b \n\t"	/* check loop termination, proceed if required */
       "emms                   \n\t"	/* exit MMX state */
      "popa                   \n\t":"=m" (Dest)	/* %0 */
      :"m"(Src1),		/* %1 */
      "m"(length),		/* %2 */
      "m"(N),			/* %3 */
      "m"(Mask)			/* %4 */
	);
#endif
    return (0);
}

/*  SDL_imageFilterShiftRight: D = saturation0(S >> N) */
int SDL_imageFilterShiftRight(unsigned char *Src1, unsigned char *Dest, int length, unsigned char N)
{
    static unsigned char Mask[8] = { 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F };
    unsigned int i, istart;
    unsigned char *cursrc1;
    unsigned char *curdest;

    /* Check shift */
    if ((N > 8) || (N < 1)) {
	return (-1);
    }

    if ((SDL_imageFilterMMXdetect()) && (length > 7)) {

	/* MMX routine */
	SDL_imageFilterShiftRightMMX(Src1, Dest, length, N, Mask);

	/* Check for unaligned bytes */
	if ((length & 7) > 0) {
	    /* Setup to process unaligned bytes */
	    istart = length & 0xfffffff8;
	    cursrc1 = &Src1[istart];
	    curdest = &Dest[istart];
	} else {
	    /* No unaligned bytes - we are done */
	    return (0);
	}
    } else {
	/* Setup to process whole image */
	istart = 0;
	cursrc1 = Src1;
	curdest = Dest;
    }

    /* C routine to process image */
    for (i = istart; i < length; i++) {
	*curdest = (unsigned char) *cursrc1 >> N;
	/* Advance pointers */
	cursrc1++;
	curdest++;
    }

    return (0);
}

/*  SDL_imageFilterShiftRightUintMMX: D = Saturation0(S >> N) */
int SDL_imageFilterShiftRightUintMMX(unsigned char *Src1, unsigned char *Dest, int length, unsigned char N)
{
#ifdef USE_MMX
    asm volatile
     ("pusha		     \n\t"
      "mov          %1, %%eax \n\t"	/* load Src1 address into eax */
      "mov          %0, %%edi \n\t"	/* load Dest address into edi */
      "mov          %2, %%ecx \n\t"	/* load loop counter (SIZE) into ecx */
      "shr          $3, %%ecx \n\t"	/* counter/8 (MMX loads 8 bytes at a time) */
       ".align 16              \n\t"	/* 16 byte allignment of the loop entry */
      "1: movq (%%eax), %%mm0 \n\t"	/* load 8 bytes from SrcDest into MM0 */
      "psrld   %3, %%mm0 \n\t"
      "movq    %%mm0, (%%edi) \n\t"	/* store result in SrcDest */
      "add          $8, %%eax \n\t"	/* increase Src1 register pointer by 8 */
      "add          $8, %%edi \n\t"	/* increase Dest register pointer by 8 */
      "dec              %%ecx \n\t"	/* decrease loop counter */
      "jnz                 1b \n\t"	/* check loop termination, proceed if required */
      "emms                   \n\t"	/* exit MMX state */
      "popa                   \n\t":"=m" (Dest)	/* %0 */
      :"m"(Src1),		/* %1 */
      "m"(length),		/* %2 */
      "m"(N)			/* %3 */
	);
#endif
    return (0);
}

/*  SDL_imageFilterShiftRightUint: D = Saturation0((uint)S >> N) */
int SDL_imageFilterShiftRightUint(unsigned char *Src1, unsigned char *Dest, int length, unsigned char N)
{
    unsigned int i, istart;
    unsigned char *cursrc1, *curdest;
    unsigned int *icursrc1, *icurdest;
    int result;

    /* min. 1 bit and max. 32 bit shift is allowed */
    if ((N > 32) || (N < 1)) return (-1);

    if ((SDL_imageFilterMMXdetect()) && (length > 7)) {

	SDL_imageFilterShiftRightUintMMX(Src1, Dest, length, N);

	/* Check for unaligned bytes */
	if ((length & 7) > 0) {
	    /* Setup to process unaligned bytes */
	    istart = length & 0xfffffff8;
	    cursrc1 = &Src1[istart];
	    curdest = &Dest[istart];
	} else {
	    /* No unaligned bytes - we are done */
	    return (0);
	}
    } else {
	/* Setup to process whole image */
	istart = 0;
	cursrc1 = Src1;
	curdest = Dest;
    }

    /* C routine to process image */
    icursrc1=(unsigned int *)cursrc1;
    icurdest=(unsigned int *)curdest;
    for (i = istart; i < length; i += 4) {
     if ((i+4)<length) {
	result = ((unsigned int)*icursrc1 >> N);
	*icurdest = (unsigned int)result;
     }
     /* Advance pointers */
     icursrc1++;
     icurdest++;
    }

    return (0);
}

/*  SDL_imageFilterMultByByteMMX: D = saturation255(S * C) */
int SDL_imageFilterMultByByteMMX(unsigned char *Src1, unsigned char *Dest, int length, unsigned char C)
{
#ifdef USE_MMX
    asm volatile
     ("pusha		     \n\t"
      /* ** Duplicate C in 4 words of MM1 ** */
      "mov           %3, %%al \n\t"	/* load C into AL */
      "xor         %%ah, %%ah \n\t"	/* zero AH */
      "mov         %%ax, %%bx \n\t"	/* copy AX into BX */
      "shl         $16, %%eax \n\t"	/* shift 2 bytes of EAX left */
      "mov         %%bx, %%ax \n\t"	/* copy BX into AX */
      "movd      %%eax, %%mm1 \n\t"	/* copy EAX into MM1 */
      "movd      %%eax, %%mm2 \n\t"	/* copy EAX into MM2 */
      "punpckldq %%mm2, %%mm1 \n\t"	/* fill higher words of MM1 with C */
       "pxor      %%mm0, %%mm0 \n\t"	/* zero MM0 register */
       "mov          %1, %%eax \n\t"	/* load Src1 address into eax */
      "mov          %0, %%edi \n\t"	/* load Dest address into edi */
      "mov          %2, %%ecx \n\t"	/* load loop counter (SIZE) into ecx */
      "shr          $3, %%ecx \n\t"	/* counter/8 (MMX loads 8 bytes at a time) */
       "cmp         $128, %%al \n\t"	/* if (C <= 128) execute more efficient code */
      "jg                  2f \n\t" ".align 16              \n\t"	/* 16 byte allignment of the loop entry */
      "1: movq (%%eax), %%mm3 \n\t"	/* load 8 bytes from Src1 into MM3 */
      "movq      %%mm3, %%mm4 \n\t"	/* copy MM3 into MM4  */
      "punpcklbw %%mm0, %%mm3 \n\t"	/* unpack low  bytes of SrcDest into words */
      "punpckhbw %%mm0, %%mm4 \n\t"	/* unpack high bytes of SrcDest into words */
      "pmullw    %%mm1, %%mm3 \n\t"	/* mul low  bytes of SrcDest and MM1 */
      "pmullw    %%mm1, %%mm4 \n\t"	/* mul high bytes of SrcDest and MM1 */
      "packuswb  %%mm4, %%mm3 \n\t"	/* pack words back into bytes with saturation */
      "movq    %%mm3, (%%edi) \n\t"	/* store result in Dest */
       "add          $8, %%eax \n\t"	/* increase Src1 register pointer by 8 */
      "add          $8, %%edi \n\t"	/* increase Dest register pointer by 8 */
      "dec              %%ecx \n\t"	/* decrease loop counter */
      "jnz                 1b \n\t"	/* check loop termination, proceed if required */
      "jmp                 3f \n\t" ".align 16              \n\t"	/* 16 byte allignment of the loop entry */
      "2: movq (%%eax), %%mm3 \n\t"	/* load 8 bytes from Src1 into MM3 */
      "movq      %%mm3, %%mm4 \n\t"	/* copy MM3 into MM4  */
      "punpcklbw %%mm0, %%mm3 \n\t"	/* unpack low  bytes of SrcDest into words */
      "punpckhbw %%mm0, %%mm4 \n\t"	/* unpack high bytes of SrcDest into words */
      "pmullw    %%mm1, %%mm3 \n\t"	/* mul low  bytes of SrcDest and MM1 */
      "pmullw    %%mm1, %%mm4 \n\t"	/* mul high bytes of SrcDest and MM1 */
      /* ** Take abs value of the results (signed words) ** */
       "movq      %%mm3, %%mm5 \n\t"	/* copy mm3 into mm5 */
      "movq      %%mm4, %%mm6 \n\t"	/* copy mm4 into mm6 */
      "psraw       $15, %%mm5 \n\t"	/* fill mm5 words with word sign bit */
      "psraw       $15, %%mm6 \n\t"	/* fill mm6 words with word sign bit */
      "pxor      %%mm5, %%mm3 \n\t"	/* take 1's compliment of only neg. words */
      "pxor      %%mm6, %%mm4 \n\t"	/* take 1's compliment of only neg. words */
      "psubsw    %%mm5, %%mm3 \n\t"	/* add 1 to only neg. words, W-(-1) or W-0 */
      "psubsw    %%mm6, %%mm4 \n\t"	/* add 1 to only neg. words, W-(-1) or W-0 */
      "packuswb  %%mm4, %%mm3 \n\t"	/* pack words back into bytes with saturation */
      "movq    %%mm3, (%%edi) \n\t"	/* store result in Dest */
       "add          $8, %%eax \n\t"	/* increase Src1 register pointer by 8 */
      "add          $8, %%edi \n\t"	/* increase Dest register pointer by 8 */
      "dec              %%ecx \n\t"	/* decrease loop counter */
      "jnz                 2b \n\t"	/* check loop termination, proceed if required */
       "3: emms               \n\t"	/* exit MMX state */
      "popa                   \n\t":"=m" (Dest)	/* %0 */
      :"m"(Src1),		/* %1 */
      "m"(length),		/* %2 */
      "m"(C)			/* %3 */
	);
#endif
    return (0);
}

/*  SDL_imageFilterMultByByte: D = saturation255(S * C) */
int SDL_imageFilterMultByByte(unsigned char *Src1, unsigned char *Dest, int length, unsigned char C)
{
    unsigned int i, istart;
    int iC;
    unsigned char *cursrc1;
    unsigned char *curdest;
    int result;

    if ((SDL_imageFilterMMXdetect()) && (length > 7)) {

	SDL_imageFilterMultByByteMMX(Src1, Dest, length, C);

	/* Check for unaligned bytes */
	if ((length & 7) > 0) {
	    /* Setup to process unaligned bytes */
	    istart = length & 0xfffffff8;
	    cursrc1 = &Src1[istart];
	    curdest = &Dest[istart];
	} else {
	    /* No unaligned bytes - we are done */
	    return (0);
	}
    } else {
	/* Setup to process whole image */
	istart = 0;
	cursrc1 = Src1;
	curdest = Dest;
    }

    /* C routine to process image */
    iC = (int) C;
    for (i = istart; i < length; i++) {
	result = (int) *cursrc1 * iC;
	if (result > 255)
	    result = 255;
	*curdest = (unsigned char) result;
	/* Advance pointers */
	cursrc1++;
	curdest++;
    }

    return (0);
}

/*  SDL_imageFilterShiftRightAndMultByByteMMX: D = saturation255((S >> N) * C) */
int SDL_imageFilterShiftRightAndMultByByteMMX(unsigned char *Src1, unsigned char *Dest, int length, unsigned char N,
					      unsigned char C)
{
#ifdef USE_MMX
    asm volatile
     ("pusha		     \n\t"
      /* ** Duplicate C in 4 words of MM1 ** */
      "mov           %4, %%al \n\t"	/* load C into AL */
      "xor         %%ah, %%ah \n\t"	/* zero AH */
      "mov         %%ax, %%bx \n\t"	/* copy AX into BX */
      "shl         $16, %%eax \n\t"	/* shift 2 bytes of EAX left */
      "mov         %%bx, %%ax \n\t"	/* copy BX into AX */
      "movd      %%eax, %%mm1 \n\t"	/* copy EAX into MM1 */
      "movd      %%eax, %%mm2 \n\t"	/* copy EAX into MM2 */
      "punpckldq %%mm2, %%mm1 \n\t"	/* fill higher words of MM1 with C */
       "xor       %%ecx, %%ecx \n\t"	/* zero ECX */
      "mov           %3, %%cl \n\t"	/* load N into CL */
      "movd      %%ecx, %%mm7 \n\t"	/* copy N into MM7 */
       "pxor      %%mm0, %%mm0 \n\t"	/* zero MM0 register */
       "mov          %1, %%eax \n\t"	/* load Src1 address into eax */
      "mov          %0, %%edi \n\t"	/* load Dest address into edi */
      "mov          %2, %%ecx \n\t"	/* load loop counter (SIZE) into ecx */
      "shr          $3, %%ecx \n\t"	/* counter/8 (MMX loads 8 bytes at a time) */
       ".align 16             \n\t"	/* 16 byte allignment of the loop entry */
      "1: movq (%%eax), %%mm3 \n\t"	/* load 8 bytes from Src1 into MM3 */
      "movq      %%mm3, %%mm4 \n\t"	/* copy MM3 into MM4  */
      "punpcklbw %%mm0, %%mm3 \n\t"	/* unpack low  bytes of SrcDest into words */
      "punpckhbw %%mm0, %%mm4 \n\t"	/* unpack high bytes of SrcDest into words */
      "psrlw     %%mm7, %%mm3 \n\t"	/* shift 4 WORDS of MM3 (N) bits to the right */
      "psrlw     %%mm7, %%mm4 \n\t"	/* shift 4 WORDS of MM4 (N) bits to the right */
      "pmullw    %%mm1, %%mm3 \n\t"	/* mul low  bytes of SrcDest by MM1 */
      "pmullw    %%mm1, %%mm4 \n\t"	/* mul high bytes of SrcDest by MM1 */
      "packuswb  %%mm4, %%mm3 \n\t"	/* pack words back into bytes with saturation */
      "movq    %%mm3, (%%edi) \n\t"	/* store result in Dest */
       "add          $8, %%eax \n\t"	/* increase Src1 register pointer by 8 */
      "add          $8, %%edi \n\t"	/* increase Dest register pointer by 8 */
      "dec              %%ecx \n\t"	/* decrease loop counter */
      "jnz                 1b \n\t"	/* check loop termination, proceed if required */
       "emms                   \n\t"	/* exit MMX state */
      "popa                   \n\t":"=m" (Dest)	/* %0 */
      :"m"(Src1),		/* %1 */
      "m"(length),		/* %2 */
      "m"(N),			/* %3 */
      "m"(C)			/* %4 */
	);
#endif
    return (0);
}

/*  SDL_imageFilterShiftRightAndMultByByte: D = saturation255((S >> N) * C) */
int SDL_imageFilterShiftRightAndMultByByte(unsigned char *Src1, unsigned char *Dest, int length, unsigned char N,
					   unsigned char C)
{
    unsigned int i, istart;
    int iC;
    unsigned char *cursrc1;
    unsigned char *curdest;
    int result;

    /* Check shift */
    if ((N > 8) || (N < 1)) {
	return (-1);
    }

    if ((SDL_imageFilterMMXdetect()) && (length > 7)) {

	SDL_imageFilterShiftRightAndMultByByteMMX(Src1, Dest, length, N, C);

	/* Check for unaligned bytes */
	if ((length & 7) > 0) {
	    /* Setup to process unaligned bytes */
	    istart = length & 0xfffffff8;
	    cursrc1 = &Src1[istart];
	    curdest = &Dest[istart];
	} else {
	    /* No unaligned bytes - we are done */
	    return (0);
	}
    } else {
	/* Setup to process whole image */
	istart = 0;
	cursrc1 = Src1;
	curdest = Dest;
    }

    /* C routine to process image */
    iC = (int) C;
    for (i = istart; i < length; i++) {
	result = (int) (*cursrc1 >> N) * iC;
	if (result > 255)
	    result = 255;
	*curdest = (unsigned char) result;
	/* Advance pointers */
	cursrc1++;
	curdest++;
    }

    return (0);
}

/*  SDL_imageFilterShiftLeftByteMMX: D = (S << N) */
int SDL_imageFilterShiftLeftByteMMX(unsigned char *Src1, unsigned char *Dest, int length, unsigned char N,
				    unsigned char *Mask)
{
#ifdef USE_MMX
    asm volatile
     ("pusha		     \n\t" "movl         %4, %%edx \n\t"	/* load Mask address into edx */
      "movq    (%%edx), %%mm0 \n\t"	/* load Mask into mm0 */
       "xor       %%ecx, %%ecx \n\t"	/* zero ECX */
      "mov           %3, %%cl \n\t"	/* load loop counter (N) into CL */
      "movd      %%ecx, %%mm3 \n\t"	/* copy (N) into MM3  */
       "pcmpeqb   %%mm1, %%mm1 \n\t"	/* generate all 1's in mm1 */
       "1:                     \n\t"	/* ** Prepare proper bit-Mask in MM1 ** */
       "psllw        $1, %%mm1 \n\t"	/* shift 4 WORDS of MM1 1 bit to the left */
      /*    "pand      %%mm0, %%mm1 \n\t"    // apply Mask to 8 BYTES of MM1 */
      ".byte     0x0f, 0xdb, 0xc8 \n\t" "dec %%cl               \n\t"	/* decrease loop counter */
      "jnz                 1b \n\t"	/* check loop termination, proceed if required */
      /* ** Shift all bytes of the image ** */
       "mov          %1, %%eax \n\t"	/* load Src1 address into eax */
      "mov          %0, %%edi \n\t"	/* load SrcDest address into edi */
      "mov          %2, %%ecx \n\t"	/* load loop counter (SIZE) into ecx */
      "shr          $3, %%ecx \n\t"	/* counter/8 (MMX loads 8 bytes at a time) */
       ".align 16              \n\t"	/* 16 byte allignment of the loop entry */
      "2: movq (%%eax), %%mm0 \n\t"	/* load 8 bytes from Src1 into MM0 */
      "psllw     %%mm3, %%mm0 \n\t"	/* shift 4 WORDS of MM0 (N) bits to the left */
      /*    "pand      %%mm1, %%mm0 \n\t"    // apply proper bit-Mask to 8 BYTES of MM0 */
      ".byte     0x0f, 0xdb, 0xc1 \n\t" "movq    %%mm0, (%%edi) \n\t"	/* store result in Dest */
       "add          $8, %%eax \n\t"	/* increase Src1 register pointer by 8 */
      "add          $8, %%edi \n\t"	/* increase Dest register pointer by 8 */
      "dec              %%ecx \n\t"	/* decrease loop counter */
      "jnz                 2b \n\t"	/* check loop termination, proceed if required */
       "emms                   \n\t"	/* exit MMX state */
      "popa                   \n\t":"=m" (Dest)	/* %0 */
      :"m"(Src1),		/* %1 */
      "m"(length),		/* %2 */
      "m"(N),			/* %3 */
      "m"(Mask)			/* %4 */
	);
#endif
    return (0);
}

/*  SDL_imageFilterShiftLeftByte: D = (S << N) */
int SDL_imageFilterShiftLeftByte(unsigned char *Src1, unsigned char *Dest, int length, unsigned char N)
{
    static unsigned char Mask[8] = { 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE };
    unsigned int i, istart;
    unsigned char *cursrc1, *curdest;
    int result;

    if ((N > 8) || (N < 1))
	return (-1);		/* image size must be at least 8 bytes  */
    /* and min. 1 bit and max. 8 bit shift is allowed */

    if ((SDL_imageFilterMMXdetect()) && (length > 7)) {

	SDL_imageFilterShiftLeftByteMMX(Src1, Dest, length, N, Mask);

	/* Check for unaligned bytes */
	if ((length & 7) > 0) {
	    /* Setup to process unaligned bytes */
	    istart = length & 0xfffffff8;
	    cursrc1 = &Src1[istart];
	    curdest = &Dest[istart];
	} else {
	    /* No unaligned bytes - we are done */
	    return (0);
	}
    } else {
	/* Setup to process whole image */
	istart = 0;
	cursrc1 = Src1;
	curdest = Dest;
    }

    /* C routine to process image */
    for (i = istart; i < length; i++) {
	result = ((int) *cursrc1 << N) & 0xff;
	*curdest = (unsigned char) result;
	/* Advance pointers */
	cursrc1++;
	curdest++;
    }

    return (0);
}

/*  SDL_imageFilterShiftLeftUintMMX: D = (S << N) */
int SDL_imageFilterShiftLeftUintMMX(unsigned char *Src1, unsigned char *Dest, int length, unsigned char N)
{
#ifdef USE_MMX
    asm volatile
     ("pusha		     \n\t"
      "mov          %1, %%eax \n\t"	/* load Src1 address into eax */
      "mov          %0, %%edi \n\t"	/* load Dest address into edi */
      "mov          %2, %%ecx \n\t"	/* load loop counter (SIZE) into ecx */
      "shr          $3, %%ecx \n\t"	/* counter/8 (MMX loads 8 bytes at a time) */
       ".align 16              \n\t"	/* 16 byte allignment of the loop entry */
      "1: movq (%%eax), %%mm0 \n\t"	/* load 8 bytes from SrcDest into MM0 */
      "pslld   %3, %%mm0 \n\t"	/* MM0=SrcDest+C (add 8 bytes with saturation) */
      "movq    %%mm0, (%%edi) \n\t"	/* store result in SrcDest */
      "add          $8, %%eax \n\t"	/* increase Src1 register pointer by 8 */
      "add          $8, %%edi \n\t"	/* increase Dest register pointer by 8 */
      "dec              %%ecx \n\t"	/* decrease loop counter */
      "jnz                 1b \n\t"	/* check loop termination, proceed if required */
      "emms                   \n\t"	/* exit MMX state */
      "popa                   \n\t":"=m" (Dest)	/* %0 */
      :"m"(Src1),		/* %1 */
      "m"(length),		/* %2 */
      "m"(N)			/* %3 */
	);
#endif
    return (0);
}

/*  SDL_imageFilterShiftLeftUint: D = ((uint)S << N) */
int SDL_imageFilterShiftLeftUint(unsigned char *Src1, unsigned char *Dest, int length, unsigned char N)
{
    unsigned int i, istart;
    unsigned char *cursrc1, *curdest;
    unsigned int *icursrc1, *icurdest;
    int result;

    /* min. 1 bit and max. 32 bit shift is allowed */
    if ((N > 32) || (N < 1)) return (-1);

    if ((SDL_imageFilterMMXdetect()) && (length > 7)) {

	SDL_imageFilterShiftLeftUintMMX(Src1, Dest, length, N);

	/* Check for unaligned bytes */
	if ((length & 7) > 0) {
	    /* Setup to process unaligned bytes */
	    istart = length & 0xfffffff8;
	    cursrc1 = &Src1[istart];
	    curdest = &Dest[istart];
	} else {
	    /* No unaligned bytes - we are done */
	    return (0);
	}
    } else {
	/* Setup to process whole image */
	istart = 0;
	cursrc1 = Src1;
	curdest = Dest;
    }

    /* C routine to process image */
    icursrc1=(unsigned int *)cursrc1;
    icurdest=(unsigned int *)curdest;
    for (i = istart; i < length; i += 4) {
     if ((i+4)<length) {
	result = ((unsigned int)*icursrc1 << N);
	*icurdest = (unsigned int)result;
     }
     /* Advance pointers */
     icursrc1++;
     icurdest++;
    }

    return (0);
}

/*  SDL_imageFilterShiftLeftMMX: D = saturation255(S << N) */
int SDL_imageFilterShiftLeftMMX(unsigned char *Src1, unsigned char *Dest, int length, unsigned char N)
{
#ifdef USE_MMX
    asm volatile
     ("pusha		     \n\t" "xor       %%eax, %%eax \n\t"	/* zero EAX */
      "mov           %3, %%al \n\t"	/* load N into AL */
      "movd      %%eax, %%mm7 \n\t"	/* copy N into MM7 */
       "pxor      %%mm0, %%mm0 \n\t"	/* zero MM0 register */
       "mov         %1, %%eax  \n\t"	/* load Src1 address into eax */
      "mov         %0, %%edi  \n\t"	/* load Dest address into edi */
      "mov         %2, %%ecx  \n\t"	/* load loop counter (SIZE) into ecx */
      "shr         $3, %%ecx  \n\t"	/* counter/8 (MMX loads 8 bytes at a time) */
       "cmp           $7, %%al \n\t"	/* if (N <= 7) execute more efficient code */
      "jg                  2f \n\t" ".align 16              \n\t"	/* 16 byte allignment of the loop entry */
      "1: movq (%%eax), %%mm3 \n\t"	/* load 8 bytes from Src1 into MM3 */
      "movq      %%mm3, %%mm4 \n\t"	/* copy MM3 into MM4  */
      "punpcklbw %%mm0, %%mm3 \n\t"	/* unpack low  bytes of SrcDest into words */
      "punpckhbw %%mm0, %%mm4 \n\t"	/* unpack high bytes of SrcDest into words */
      "psllw     %%mm7, %%mm3 \n\t"	/* shift 4 WORDS of MM3 (N) bits to the right */
      "psllw     %%mm7, %%mm4 \n\t"	/* shift 4 WORDS of MM4 (N) bits to the right */
      "packuswb  %%mm4, %%mm3 \n\t"	/* pack words back into bytes with saturation */
      "movq    %%mm3, (%%edi) \n\t"	/* store result in Dest */
       "add          $8, %%eax \n\t"	/* increase Src1 register pointer by 8 */
      "add          $8, %%edi \n\t"	/* increase Dest register pointer by 8 */
      "dec              %%ecx \n\t"	/* decrease loop counter */
      "jnz                 1b \n\t"	/* check loop termination, proceed if required */
      "jmp                 3f \n\t" ".align 16              \n\t"	/* 16 byte allignment of the loop entry */
      "2: movq (%%eax), %%mm3 \n\t"	/* load 8 bytes from Src1 into MM3 */
      "movq      %%mm3, %%mm4 \n\t"	/* copy MM3 into MM4  */
      "punpcklbw %%mm0, %%mm3 \n\t"	/* unpack low  bytes of SrcDest into words */
      "punpckhbw %%mm0, %%mm4 \n\t"	/* unpack high bytes of SrcDest into words */
      "psllw     %%mm7, %%mm3 \n\t"	/* shift 4 WORDS of MM3 (N) bits to the right */
      "psllw     %%mm7, %%mm4 \n\t"	/* shift 4 WORDS of MM4 (N) bits to the right */
      /* ** Take abs value of the signed words ** */
       "movq      %%mm3, %%mm5 \n\t"	/* copy mm3 into mm5 */
      "movq      %%mm4, %%mm6 \n\t"	/* copy mm4 into mm6 */
      "psraw       $15, %%mm5 \n\t"	/* fill mm5 words with word sign bit */
      "psraw       $15, %%mm6 \n\t"	/* fill mm6 words with word sign bit */
      "pxor      %%mm5, %%mm3 \n\t"	/* take 1's compliment of only neg. words */
      "pxor      %%mm6, %%mm4 \n\t"	/* take 1's compliment of only neg. words */
      "psubsw    %%mm5, %%mm3 \n\t"	/* add 1 to only neg. words, W-(-1) or W-0 */
      "psubsw    %%mm6, %%mm4 \n\t"	/* add 1 to only neg. words, W-(-1) or W-0 */
       "packuswb  %%mm4, %%mm3 \n\t"	/* pack words back into bytes with saturation */
      "movq    %%mm3, (%%edi) \n\t"	/* store result in Dest */
       "add          $8, %%eax \n\t"	/* increase Src1 register pointer by 8 */
      "add          $8, %%edi \n\t"	/* increase Dest register pointer by 8 */
      "dec              %%ecx \n\t"	/* decrease loop counter */
      "jnz                 2b \n\t"	/* check loop termination, proceed if required */
       "3: emms                \n\t"	/* exit MMX state */
      "popa                   \n\t":"=m" (Dest)	/* %0 */
      :"m"(Src1),		/* %1 */
      "m"(length),		/* %2 */
      "m"(N)			/* %3 */
	);
#endif
    return (0);
}

/*  SDL_imageFilterShiftLeft: D = saturation255(S << N) */
int SDL_imageFilterShiftLeft(unsigned char *Src1, unsigned char *Dest, int length, unsigned char N)
{
    unsigned int i, istart;
    unsigned char *cursrc1, *curdest;
    int result;

    if ((N > 8) || (N < 1))
	return (-1);		/* image size must be at least 8 bytes  */
    /* and min. 1 bit and max. 8 bit shift is allowed */

    if ((SDL_imageFilterMMXdetect()) && (length > 7)) {

	SDL_imageFilterShiftLeftMMX(Src1, Dest, length, N);

	/* Check for unaligned bytes */
	if ((length & 7) > 0) {
	    /* Setup to process unaligned bytes */
	    istart = length & 0xfffffff8;
	    cursrc1 = &Src1[istart];
	    curdest = &Dest[istart];
	} else {
	    /* No unaligned bytes - we are done */
	    return (0);
	}
    } else {
	/* Setup to process whole image */
	istart = 0;
	cursrc1 = Src1;
	curdest = Dest;
    }

    /* C routine to process image */
    for (i = istart; i < length; i++) {
	result = (int) *cursrc1 << N;
	if (result > 255)
	    result = 255;
	*curdest = (unsigned char) result;
	/* Advance pointers */
	cursrc1++;
	curdest++;
    }

    return (0);
}

/*  SDL_imageFilterBinarizeUsingThresholdMMX: D = (S >= T) ? 255:0 */
int SDL_imageFilterBinarizeUsingThresholdMMX(unsigned char *Src1, unsigned char *Dest, int length, unsigned char T)
{
#ifdef USE_MMX
    asm volatile
     ("pusha		     \n\t"
      /* ** Duplicate T in 8 bytes of MM3 ** */
      "pcmpeqb   %%mm1, %%mm1 \n\t"	/* generate all 1's in mm1 */
      "pcmpeqb   %%mm2, %%mm2 \n\t"	/* generate all 1's in mm2 */
       "mov           %3, %%al \n\t"	/* load T into AL */
      "mov         %%al, %%ah \n\t"	/* copy AL into AH */
      "mov         %%ax, %%bx \n\t"	/* copy AX into BX */
      "shl         $16, %%eax \n\t"	/* shift 2 bytes of EAX left */
      "mov         %%bx, %%ax \n\t"	/* copy BX into AX */
      "movd      %%eax, %%mm3 \n\t"	/* copy EAX into MM3 */
      "movd      %%eax, %%mm4 \n\t"	/* copy EAX into MM4 */
      "punpckldq %%mm4, %%mm3 \n\t"	/* fill higher bytes of MM3 with T */
       "psubusb   %%mm3, %%mm2 \n\t"	/* store 0xFF - T in MM2 */
       "mov          %1, %%eax \n\t"	/* load Src1 address into eax */
      "mov          %0, %%edi \n\t"	/* load Dest address into edi */
      "mov          %2, %%ecx \n\t"	/* load loop counter (SIZE) into ecx */
      "shr          $3, %%ecx \n\t"	/* counter/8 (MMX loads 8 bytes at a time) */
       ".align 16              \n\t"	/* 16 byte alignment of the loop entry */
      "1:                     \n\t" 
      "movq    (%%eax), %%mm0 \n\t"	/* load 8 bytes from SrcDest into MM0 */
      "paddusb   %%mm2, %%mm0 \n\t"	/* MM0=SrcDest+(0xFF-T) (add 8 bytes with saturation) */
      "pcmpeqb   %%mm1, %%mm0 \n\t"	/* binarize 255:0, comparing to 255 */
      "movq    %%mm0, (%%edi) \n\t"	/* store result in SrcDest */
       "add          $8, %%eax \n\t"	/* increase Src1 register pointer by 8 */
      "add          $8, %%edi \n\t"	/* increase Dest register pointer by 8 */
      "dec              %%ecx \n\t"	/* decrease loop counter */
      "jnz                 1b \n\t"	/* check loop termination, proceed if required */
       "emms                   \n\t"	/* exit MMX state */
      "popa                   \n\t":"=m" (Dest)	/* %0 */
      :"m"(Src1),		/* %1 */
      "m"(length),		/* %2 */
      "m"(T)			/* %3 */
	);
#endif
    return (0);
}

/*  SDL_imageFilterBinarizeUsingThreshold: D = (S >= T) ? 255:0 */
int SDL_imageFilterBinarizeUsingThreshold(unsigned char *Src1, unsigned char *Dest, int length, unsigned char T)
{
    unsigned int i, istart;
    unsigned char *cursrc1;
    unsigned char *curdest;

    if ((SDL_imageFilterMMXdetect()) && (length > 7)) {

	SDL_imageFilterBinarizeUsingThresholdMMX(Src1, Dest, length, T);

	/* Check for unaligned bytes */
	if ((length & 7) > 0) {
	    /* Setup to process unaligned bytes */
	    istart = length & 0xfffffff8;
	    cursrc1 = &Src1[istart];
	    curdest = &Dest[istart];
	} else {
	    /* No unaligned bytes - we are done */
	    return (0);
	}
    } else {
	/* Setup to process whole image */
	istart = 0;
	cursrc1 = Src1;
	curdest = Dest;
    }

    /* C routine to process image */
    for (i = istart; i < length; i++) {
	*curdest = ((unsigned char) *cursrc1 >= T) ? 255 : 0;
	/* Advance pointers */
	cursrc1++;
	curdest++;
    }

    return (0);
}

/*  SDL_imageFilterClipToRangeMMX: D = (S >= Tmin) & (S <= Tmax) S:Tmin | Tmax */
int SDL_imageFilterClipToRangeMMX(unsigned char *Src1, unsigned char *Dest, int length, unsigned char Tmin,
				  unsigned char Tmax)
{
#ifdef USE_MMX
    asm volatile
     ("pusha		     \n\t" "pcmpeqb   %%mm1, %%mm1 \n\t"	/* generate all 1's in mm1 */
      /* ** Duplicate Tmax in 8 bytes of MM3 ** */
       "mov           %4, %%al \n\t"	/* load Tmax into AL */
      "mov         %%al, %%ah \n\t"	/* copy AL into AH */
      "mov         %%ax, %%bx \n\t"	/* copy AX into BX */
      "shl         $16, %%eax \n\t"	/* shift 2 bytes of EAX left */
      "mov         %%bx, %%ax \n\t"	/* copy BX into AX */
      "movd      %%eax, %%mm3 \n\t"	/* copy EAX into MM3 */
      "movd      %%eax, %%mm4 \n\t"	/* copy EAX into MM4 */
      "punpckldq %%mm4, %%mm3 \n\t"	/* fill higher bytes of MM3 with Tmax */
       "psubusb   %%mm3, %%mm1 \n\t"	/* store 0xFF - Tmax in MM1 */
      /* ** Duplicate Tmin in 8 bytes of MM5 ** */
       "mov           %3, %%al \n\t"	/* load Tmin into AL */
      "mov         %%al, %%ah \n\t"	/* copy AL into AH */
      "mov         %%ax, %%bx \n\t"	/* copy AX into BX */
      "shl         $16, %%eax \n\t"	/* shift 2 bytes of EAX left */
      "mov         %%bx, %%ax \n\t"	/* copy BX into AX */
      "movd      %%eax, %%mm5 \n\t"	/* copy EAX into MM5 */
      "movd      %%eax, %%mm4 \n\t"	/* copy EAX into MM4 */
      "punpckldq %%mm4, %%mm5 \n\t"	/* fill higher bytes of MM5 with Tmin */
       "movq      %%mm5, %%mm7 \n\t"	/* copy MM5 into MM7 */
      "paddusb   %%mm1, %%mm7 \n\t"	/* store 0xFF - Tmax + Tmin in MM7 */
       "mov          %1, %%eax \n\t"	/* load Src1 address into eax */
      "mov          %0, %%edi \n\t"	/* load Dest address into edi */
      "mov          %2, %%ecx \n\t"	/* load loop counter (SIZE) into ecx */
      "shr          $3, %%ecx \n\t"	/* counter/8 (MMX loads 8 bytes at a time) */
       ".align 16              \n\t"	/* 16 byte allignment of the loop entry */
      "1:                     \n\t" 
      "movq    (%%eax), %%mm0 \n\t"	/* load 8 bytes from Src1 into MM0 */
      "paddusb   %%mm1, %%mm0 \n\t"	/* MM0=SrcDest+(0xFF-Tmax) */
      "psubusb   %%mm7, %%mm0 \n\t"	/* MM0=MM0-(0xFF-Tmax+Tmin) */
      "paddusb   %%mm5, %%mm0 \n\t"	/* MM0=MM0+Tmin */
      "movq    %%mm0, (%%edi) \n\t"	/* store result in Dest */
       "add          $8, %%eax \n\t"	/* increase Src1 register pointer by 8 */
      "add          $8, %%edi \n\t"	/* increase Dest register pointer by 8 */
      "dec              %%ecx \n\t"	/* decrease loop counter */
      "jnz                 1b \n\t"	/* check loop termination, proceed if required */
       "emms                   \n\t"	/* exit MMX state */
      "popa                   \n\t":"=m" (Dest)	/* %0 */
      :"m"(Src1),		/* %1 */
      "m"(length),		/* %2 */
      "m"(Tmin),		/* %3 */
      "m"(Tmax)			/* %4 */
	);
#endif
    return (0);
}

/*  SDL_imageFilterClipToRange: D = (S >= Tmin) & (S <= Tmax) S:Tmin | Tmax */
int SDL_imageFilterClipToRange(unsigned char *Src1, unsigned char *Dest, int length, unsigned char Tmin,
			       unsigned char Tmax)
{
    unsigned int i, istart;
    unsigned char *cursrc1;
    unsigned char *curdest;

    if ((SDL_imageFilterMMXdetect()) && (length > 7)) {

	SDL_imageFilterClipToRangeMMX(Src1, Dest, length, Tmin, Tmax);

	/* Check for unaligned bytes */
	if ((length & 7) > 0) {
	    /* Setup to process unaligned bytes */
	    istart = length & 0xfffffff8;
	    cursrc1 = &Src1[istart];
	    curdest = &Dest[istart];
	} else {
	    /* No unaligned bytes - we are done */
	    return (0);
	}
    } else {
	/* Setup to process whole image */
	istart = 0;
	cursrc1 = Src1;
	curdest = Dest;
    }

    /* C routine to process image */
    for (i = istart; i < length; i++) {
	if (*cursrc1 < Tmin) {
	    *curdest = Tmin;
	} else if (*cursrc1 > Tmax) {
	    *curdest = Tmax;
	} else {
	    *curdest = *cursrc1;
	}
	/* Advance pointers */
	cursrc1++;
	curdest++;
    }

    return (0);
}

/*  SDL_imageFilterNormalizeLinearMMX: D = saturation255((Nmax - Nmin)/(Cmax - Cmin)*(S - Cmin) + Nmin) */
int SDL_imageFilterNormalizeLinearMMX(unsigned char *Src1, unsigned char *Dest, int length, int Cmin, int Cmax,
				      int Nmin, int Nmax)
{
#ifdef USE_MMX
    asm volatile
     ("pusha		     \n\t" "mov           %6, %%ax \n\t"	/* load Nmax in AX */
      "mov           %4, %%bx \n\t"	/* load Cmax in BX */
      "sub           %5, %%ax \n\t"	/* AX = Nmax - Nmin */
      "sub           %3, %%bx \n\t"	/* BX = Cmax - Cmin */
      "jz                  1f \n\t"	/* check division by zero */
      "xor         %%dx, %%dx \n\t"	/* prepare for division, zero DX */
      "div               %%bx \n\t"	/* AX = AX/BX */
      "jmp                 2f \n\t" "1:                     \n\t" "mov         $255, %%ax \n\t"	/* if div by zero, assume result max. byte value */
       "2:                    \n\t"	/* ** Duplicate AX in 4 words of MM0 ** */
       "mov         %%ax, %%bx \n\t"	/* copy AX into BX */
      "shl         $16, %%eax \n\t"	/* shift 2 bytes of EAX left */
      "mov         %%bx, %%ax \n\t"	/* copy BX into AX */
      "movd      %%eax, %%mm0 \n\t"	/* copy EAX into MM0 */
      "movd      %%eax, %%mm1 \n\t"	/* copy EAX into MM1 */
      "punpckldq %%mm1, %%mm0 \n\t"	/* fill higher words of MM0 with AX */
      /* ** Duplicate Cmin in 4 words of MM1 ** */
       "mov           %3, %%ax \n\t"	/* load Cmin into AX */
      "mov         %%ax, %%bx \n\t"	/* copy AX into BX */
      "shl         $16, %%eax \n\t"	/* shift 2 bytes of EAX left */
      "mov         %%bx, %%ax \n\t"	/* copy BX into AX */
      "movd      %%eax, %%mm1 \n\t"	/* copy EAX into MM1 */
      "movd      %%eax, %%mm2 \n\t"	/* copy EAX into MM2 */
      "punpckldq %%mm2, %%mm1 \n\t"	/* fill higher words of MM1 with Cmin */
      /* ** Duplicate Nmin in 4 words of MM2 ** */
       "mov           %5, %%ax \n\t"	/* load Nmin into AX */
      "mov         %%ax, %%bx \n\t"	/* copy AX into BX */
      "shl         $16, %%eax \n\t"	/* shift 2 bytes of EAX left */
      "mov         %%bx, %%ax \n\t"	/* copy BX into AX */
      "movd      %%eax, %%mm2 \n\t"	/* copy EAX into MM2 */
      "movd      %%eax, %%mm3 \n\t"	/* copy EAX into MM3 */
      "punpckldq %%mm3, %%mm2 \n\t"	/* fill higher words of MM2 with Nmin */
       "pxor      %%mm7, %%mm7 \n\t"	/* zero MM7 register */
       "mov          %1, %%eax \n\t"	/* load Src1 address into eax */
      "mov          %0, %%edi \n\t"	/* load Dest address into edi */
      "mov          %2, %%ecx \n\t"	/* load loop counter (SIZE) into ecx */
      "shr          $3, %%ecx \n\t"	/* counter/8 (MMX loads 8 bytes at a time) */
       ".align 16              \n\t"	/* 16 byte allignment of the loop entry */
      "1:                     \n\t" 
      "movq    (%%eax), %%mm3 \n\t"	/* load 8 bytes from Src1 into MM3 */
      "movq      %%mm3, %%mm4 \n\t"	/* copy MM3 into MM4  */
      "punpcklbw %%mm7, %%mm3 \n\t"	/* unpack low  bytes of SrcDest into words */
      "punpckhbw %%mm7, %%mm4 \n\t"	/* unpack high bytes of SrcDest into words */
      "psubusb   %%mm1, %%mm3 \n\t"	/* S-Cmin, low  bytes */
      "psubusb   %%mm1, %%mm4 \n\t"	/* S-Cmin, high bytes */
      "pmullw    %%mm0, %%mm3 \n\t"	/* MM0*(S-Cmin), low  bytes */
      "pmullw    %%mm0, %%mm4 \n\t"	/* MM0*(S-Cmin), high bytes */
      "paddusb   %%mm2, %%mm3 \n\t"	/* MM0*(S-Cmin)+Nmin, low  bytes */
      "paddusb   %%mm2, %%mm4 \n\t"	/* MM0*(S-Cmin)+Nmin, high bytes */
      /* ** Take abs value of the signed words ** */
       "movq      %%mm3, %%mm5 \n\t"	/* copy mm3 into mm5 */
      "movq      %%mm4, %%mm6 \n\t"	/* copy mm4 into mm6 */
      "psraw       $15, %%mm5 \n\t"	/* fill mm5 words with word sign bit */
      "psraw       $15, %%mm6 \n\t"	/* fill mm6 words with word sign bit */
      "pxor      %%mm5, %%mm3 \n\t"	/* take 1's compliment of only neg. words */
      "pxor      %%mm6, %%mm4 \n\t"	/* take 1's compliment of only neg. words */
      "psubsw    %%mm5, %%mm3 \n\t"	/* add 1 to only neg. words, W-(-1) or W-0 */
      "psubsw    %%mm6, %%mm4 \n\t"	/* add 1 to only neg. words, W-(-1) or W-0 */
      "packuswb  %%mm4, %%mm3 \n\t"	/* pack words back into bytes with saturation */
       "movq    %%mm3, (%%edi) \n\t"	/* store result in Dest */
       "add          $8, %%eax \n\t"	/* increase Src1 register pointer by 8 */
      "add          $8, %%edi \n\t"	/* increase Dest register pointer by 8 */
      "dec              %%ecx \n\t"	/* decrease loop counter */
      "jnz                 1b \n\t"	/* check loop termination, proceed if required */
       "emms                   \n\t"	/* exit MMX state */
      "popa                   \n\t":"=m" (Dest)	/* %0 */
      :"m"(Src1),		/* %1 */
      "m"(length),		/* %2 */
      "m"(Cmin),		/* %3 */
      "m"(Cmax),		/* %4 */
      "m"(Nmin),		/* %5 */
      "m"(Nmax)			/* %6 */
	);
#endif
    return (0);
}

/*  SDL_imageFilterNormalizeLinear: D = saturation255((Nmax - Nmin)/(Cmax - Cmin)*(S - Cmin) + Nmin) */
int SDL_imageFilterNormalizeLinear(unsigned char *Src1, unsigned char *Dest, int length, int Cmin, int Cmax, int Nmin,
				   int Nmax)
{
    unsigned int i, istart;
    unsigned char *cursrc1;
    unsigned char *curdest;
    int dN, dC, factor;
    int result;

    if ((SDL_imageFilterMMXdetect()) && (length > 7)) {

	SDL_imageFilterNormalizeLinearMMX(Src1, Dest, length, Cmin, Cmax, Nmin, Nmax);

	/* Check for unaligned bytes */
	if ((length & 7) > 0) {
	    /* Setup to process unaligned bytes */
	    istart = length & 0xfffffff8;
	    cursrc1 = &Src1[istart];
	    curdest = &Dest[istart];
	} else {
	    /* No unaligned bytes - we are done */
	    return (0);
	}
    } else {
	/* Setup to process whole image */
	istart = 0;
	cursrc1 = Src1;
	curdest = Dest;
    }

    /* C routine to process image */
    dC = Cmax - Cmin;
    if (dC == 0)
	return (0);
    dN = Nmax - Nmin;
    factor = dN / dC;
    for (i = istart; i < length; i++) {
	result = factor * ((int) (*cursrc1) - Cmin) + Nmin;
	if (result > 255)
	    result = 255;
	*curdest = (unsigned char) result;
	/* Advance pointers */
	cursrc1++;
	curdest++;
    }

    return (0);
}

/* ------------------------------------------------------------------------------------ */

/*  SDL_imageFilterConvolveKernel3x3Divide: Dij = saturation0and255( ... ) */
int SDL_imageFilterConvolveKernel3x3Divide(unsigned char *Src, unsigned char *Dest, int rows, int columns,
					   signed short *Kernel, unsigned char Divisor)
{
    if ((columns < 3) || (rows < 3) || (Divisor == 0))
	return (-1);

    if ((SDL_imageFilterMMXdetect())) {
#ifdef USE_MMX
	asm volatile
	 ("pusha		     \n\t" "pxor      %%mm0, %%mm0 \n\t"	/* zero MM0 */
	  "xor       %%ebx, %%ebx \n\t"	/* zero EBX */
	  "mov           %5, %%bl \n\t"	/* load Divisor into BL */
	   "mov          %4, %%edx \n\t"	/* load Kernel address into EDX */
	  "movq    (%%edx), %%mm5 \n\t"	/* MM5 = {0,K2,K1,K0} */
	  "add          $8, %%edx \n\t"	/* second row              |K0 K1 K2 0| */
	  "movq    (%%edx), %%mm6 \n\t"	/* MM6 = {0,K5,K4,K3}  K = |K3 K4 K5 0| */
	  "add          $8, %%edx \n\t"	/* third row               |K6 K7 K8 0| */
	  "movq    (%%edx), %%mm7 \n\t"	/* MM7 = {0,K8,K7,K6} */
/* --- */
	  "mov          %3, %%eax \n\t"	/* load columns into EAX */
	  "mov          %1, %%esi \n\t"	/* ESI = Src row 0 address */
	  "mov          %0, %%edi \n\t"	/* load Dest address to EDI */
	  "add       %%eax, %%edi \n\t"	/* EDI = EDI + columns */
	  "inc              %%edi \n\t"	/* 1 byte offset from the left edge */
	   "mov          %2, %%edx \n\t"	/* initialize ROWS counter */
	  "sub          $2, %%edx \n\t"	/* do not use first and last row */
/* --- */
	  ".L10320:               \n\t" "mov       %%eax, %%ecx \n\t"	/* initialize COLUMS counter */
	  "sub          $2, %%ecx \n\t"	/* do not use first and last column */
	   ".align 16              \n\t"	/* 16 byte allignment of the loop entry */
	  ".L10322:               \n\t"
/* --- */
	  "movq    (%%esi), %%mm1 \n\t"	/* load 8 bytes of the image first row */
	  "add       %%eax, %%esi \n\t"	/* move one row below */
	  "movq    (%%esi), %%mm2 \n\t"	/* load 8 bytes of the image second row */
	  "add       %%eax, %%esi \n\t"	/* move one row below */
	  "movq    (%%esi), %%mm3 \n\t"	/* load 8 bytes of the image third row */
	   "punpcklbw %%mm0, %%mm1 \n\t"	/* unpack first 4 bytes into words */
	  "punpcklbw %%mm0, %%mm2 \n\t"	/* unpack first 4 bytes into words */
	  "punpcklbw %%mm0, %%mm3 \n\t"	/* unpack first 4 bytes into words */
	   "pmullw    %%mm5, %%mm1 \n\t"	/* multiply words first row  image*Kernel */
	  "pmullw    %%mm6, %%mm2 \n\t"	/* multiply words second row image*Kernel */
	  "pmullw    %%mm7, %%mm3 \n\t"	/* multiply words third row  image*Kernel */
	   "paddsw    %%mm2, %%mm1 \n\t"	/* add 4 words of the first and second rows */
	  "paddsw    %%mm3, %%mm1 \n\t"	/* add 4 words of the third row and result */
	  "movq      %%mm1, %%mm2 \n\t"	/* copy MM1 into MM2 */
	  "psrlq       $32, %%mm1 \n\t"	/* shift 2 left words to the right */
	  "paddsw    %%mm2, %%mm1 \n\t"	/* add 2 left and 2 right result words */
	  "movq      %%mm1, %%mm3 \n\t"	/* copy MM1 into MM3 */
	  "psrlq       $16, %%mm1 \n\t"	/* shift 1 left word to the right */
	  "paddsw    %%mm3, %%mm1 \n\t"	/* add 1 left and 1 right result words */
/* -- */
	  "movd      %%eax, %%mm2 \n\t"	/* save EAX in MM2 */
	  "movd      %%edx, %%mm3 \n\t"	/* save EDX in MM3 */
	  "movd      %%mm1, %%eax \n\t"	/* copy MM1 into EAX */
	  "psraw       $15, %%mm1 \n\t"	/* spread sign bit of the result */
	  "movd      %%mm1, %%edx \n\t"	/* fill EDX with a sign bit */
	  "idivw             %%bx \n\t"	/* IDIV - VERY EXPENSIVE */
	  "movd      %%eax, %%mm1 \n\t"	/* move result of division into MM1 */
	  "packuswb  %%mm0, %%mm1 \n\t"	/* pack division result with saturation */
	  "movd      %%mm1, %%eax \n\t"	/* copy saturated result into EAX */
	  "mov      %%al, (%%edi) \n\t"	/* copy a byte result into Dest */
	  "movd      %%mm3, %%edx \n\t"	/* restore saved EDX */
	  "movd      %%mm2, %%eax \n\t"	/* restore saved EAX */
/* -- */
	  "sub       %%eax, %%esi \n\t"	/* move two rows up */
	  "sub       %%eax, %%esi \n\t"	/* */
	  "inc              %%esi \n\t"	/* move Src  pointer to the next pixel */
	  "inc              %%edi \n\t"	/* move Dest pointer to the next pixel */
/* --- */
	  "dec              %%ecx \n\t"	/* decrease loop counter COLUMNS */
	  "jnz            .L10322 \n\t"	/* check loop termination, proceed if required */
	   "add          $2, %%esi \n\t"	/* move to the next row in Src */
	  "add          $2, %%edi \n\t"	/* move to the next row in Dest */
	   "dec              %%edx \n\t"	/* decrease loop counter ROWS */
	  "jnz            .L10320 \n\t"	/* check loop termination, proceed if required */
/* --- */
	  "emms                   \n\t"	/* exit MMX state */
	  "popa                   \n\t":"=m" (Dest)	/* %0 */
	  :"m"(Src),		/* %1 */
	  "m"(rows),		/* %2 */
	  "m"(columns),		/* %3 */
	  "m"(Kernel),		/* %4 */
	  "m"(Divisor)		/* %5 */
	    );
#endif
	return (0);
    } else {
	/* No non-MMX implementation yet */
	return (-1);
    }
}

/*  SDL_imageFilterConvolveKernel5x5Divide: Dij = saturation0and255( ... ) */
int SDL_imageFilterConvolveKernel5x5Divide(unsigned char *Src, unsigned char *Dest, int rows, int columns,
					   signed short *Kernel, unsigned char Divisor)
{
    if ((columns < 5) || (rows < 5) || (Divisor == 0))
	return (-1);

    if ((SDL_imageFilterMMXdetect())) {
#ifdef USE_MMX
	asm volatile
	 ("pusha		     \n\t" "pxor      %%mm0, %%mm0 \n\t"	/* zero MM0 */
	   "xor       %%ebx, %%ebx \n\t"	/* zero EBX */
	  "mov           %5, %%bl \n\t"	/* load Divisor into BL */
	  "movd      %%ebx, %%mm5 \n\t"	/* copy Divisor into MM5 */
	   "mov          %4, %%edx \n\t"	/* load Kernel address into EDX */
	  "mov          %1, %%esi \n\t"	/* load Src  address to ESI */
	  "mov          %0, %%edi \n\t"	/* load Dest address to EDI */
	  "add          $2, %%edi \n\t"	/* 2 column offset from the left edge */
	  "mov          %3, %%eax \n\t"	/* load columns into EAX */
	  "shl          $1, %%eax \n\t"	/* EAX = columns * 2 */
	  "add       %%eax, %%edi \n\t"	/* 2 row offset from the top edge */
	  "shr          $1, %%eax \n\t"	/* EAX = columns */
	   "mov          %2, %%ebx \n\t"	/* initialize ROWS counter */
	  "sub          $4, %%ebx \n\t"	/* do not use first 2 and last 2 rows */
/* --- */
	  ".L10330:               \n\t" "mov       %%eax, %%ecx \n\t"	/* initialize COLUMNS counter */
	  "sub          $4, %%ecx \n\t"	/* do not use first 2 and last 2 columns */
	   ".align 16              \n\t"	/* 16 byte allignment of the loop entry */
	  ".L10332:               \n\t" "pxor      %%mm7, %%mm7 \n\t"	/* zero MM7 (accumulator) */
	  "movd      %%esi, %%mm6 \n\t"	/* save ESI in MM6 */
/* --- 1 */
	  "movq    (%%esi), %%mm1 \n\t"	/* load 8 bytes of the Src */
	  "movq      %%mm1, %%mm2 \n\t"	/* copy MM1 into MM2 */
	  "add       %%eax, %%esi \n\t"	/* move Src pointer 1 row below */
	  "movq    (%%edx), %%mm3 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	  "movq    (%%edx), %%mm4 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	   "punpcklbw %%mm0, %%mm1 \n\t"	/* unpack first  4 bytes into words */
	  "punpckhbw %%mm0, %%mm2 \n\t"	/* unpack second 4 bytes into words */
	   "pmullw    %%mm3, %%mm1 \n\t"	/* mult. 4 low  words of Src and Kernel */
	  "pmullw    %%mm4, %%mm2 \n\t"	/* mult. 4 high words of Src and Kernel */
	   "paddsw    %%mm2, %%mm1 \n\t"	/* add 4 words of the high and low bytes */
	  "paddsw    %%mm1, %%mm7 \n\t"	/* add MM1 to accumulator MM7 */
/* --- 2 */
	  "movq    (%%esi), %%mm1 \n\t"	/* load 8 bytes of the Src */
	  "movq      %%mm1, %%mm2 \n\t"	/* copy MM1 into MM2 */
	  "add       %%eax, %%esi \n\t"	/* move Src pointer 1 row below */
	  "movq    (%%edx), %%mm3 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	  "movq    (%%edx), %%mm4 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	   "punpcklbw %%mm0, %%mm1 \n\t"	/* unpack first  4 bytes into words */
	  "punpckhbw %%mm0, %%mm2 \n\t"	/* unpack second 4 bytes into words */
	   "pmullw    %%mm3, %%mm1 \n\t"	/* mult. 4 low  words of Src and Kernel */
	  "pmullw    %%mm4, %%mm2 \n\t"	/* mult. 4 high words of Src and Kernel */
	   "paddsw    %%mm2, %%mm1 \n\t"	/* add 4 words of the high and low bytes */
	  "paddsw    %%mm1, %%mm7 \n\t"	/* add MM1 to accumulator MM7 */
/* --- 3 */
	  "movq    (%%esi), %%mm1 \n\t"	/* load 8 bytes of the Src */
	  "movq      %%mm1, %%mm2 \n\t"	/* copy MM1 into MM2 */
	  "add       %%eax, %%esi \n\t"	/* move Src pointer 1 row below */
	  "movq    (%%edx), %%mm3 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	  "movq    (%%edx), %%mm4 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	   "punpcklbw %%mm0, %%mm1 \n\t"	/* unpack first  4 bytes into words */
	  "punpckhbw %%mm0, %%mm2 \n\t"	/* unpack second 4 bytes into words */
	   "pmullw    %%mm3, %%mm1 \n\t"	/* mult. 4 low  words of Src and Kernel */
	  "pmullw    %%mm4, %%mm2 \n\t"	/* mult. 4 high words of Src and Kernel */
	   "paddsw    %%mm2, %%mm1 \n\t"	/* add 4 words of the high and low bytes */
	  "paddsw    %%mm1, %%mm7 \n\t"	/* add MM1 to accumulator MM7 */
/* --- 4 */
	  "movq    (%%esi), %%mm1 \n\t"	/* load 8 bytes of the Src */
	  "movq      %%mm1, %%mm2 \n\t"	/* copy MM1 into MM2 */
	  "add       %%eax, %%esi \n\t"	/* move Src pointer 1 row below */
	  "movq    (%%edx), %%mm3 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	  "movq    (%%edx), %%mm4 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	   "punpcklbw %%mm0, %%mm1 \n\t"	/* unpack first  4 bytes into words */
	  "punpckhbw %%mm0, %%mm2 \n\t"	/* unpack second 4 bytes into words */
	   "pmullw    %%mm3, %%mm1 \n\t"	/* mult. 4 low  words of Src and Kernel */
	  "pmullw    %%mm4, %%mm2 \n\t"	/* mult. 4 high words of Src and Kernel */
	   "paddsw    %%mm2, %%mm1 \n\t"	/* add 4 words of the high and low bytes */
	  "paddsw    %%mm1, %%mm7 \n\t"	/* add MM1 to accumulator MM7 */
/* --- 5 */
	  "movq    (%%esi), %%mm1 \n\t"	/* load 8 bytes of the Src */
	  "movq      %%mm1, %%mm2 \n\t"	/* copy MM1 into MM2 */
	  "movq    (%%edx), %%mm3 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	  "movq    (%%edx), %%mm4 \n\t"	/* load 4 words of Kernel */
	   "punpcklbw %%mm0, %%mm1 \n\t"	/* unpack first  4 bytes into words */
	  "punpckhbw %%mm0, %%mm2 \n\t"	/* unpack second 4 bytes into words */
	   "pmullw    %%mm3, %%mm1 \n\t"	/* mult. 4 low  words of Src and Kernel */
	  "pmullw    %%mm4, %%mm2 \n\t"	/* mult. 4 high words of Src and Kernel */
	   "paddsw    %%mm2, %%mm1 \n\t"	/* add 4 words of the high and low bytes */
	  "paddsw    %%mm1, %%mm7 \n\t"	/* add MM1 to accumulator MM7 */
/* --- */
	  "movq      %%mm7, %%mm3 \n\t"	/* copy MM7 into MM3 */
	  "psrlq       $32, %%mm7 \n\t"	/* shift 2 left words to the right */
	  "paddsw    %%mm3, %%mm7 \n\t"	/* add 2 left and 2 right result words */
	  "movq      %%mm7, %%mm2 \n\t"	/* copy MM7 into MM2 */
	  "psrlq       $16, %%mm7 \n\t"	/* shift 1 left word to the right */
	  "paddsw    %%mm2, %%mm7 \n\t"	/* add 1 left and 1 right result words */
/* --- */
	  "movd      %%eax, %%mm1 \n\t"	/* save EDX in MM1 */
	  "movd      %%ebx, %%mm2 \n\t"	/* save EDX in MM2 */
	  "movd      %%edx, %%mm3 \n\t"	/* save EDX in MM3 */
	   "movd      %%mm7, %%eax \n\t"	/* load summation result into EAX */
	  "psraw       $15, %%mm7 \n\t"	/* spread sign bit of the result */
	  "movd      %%mm5, %%ebx \n\t"	/* load Divisor into EBX */
	  "movd      %%mm7, %%edx \n\t"	/* fill EDX with a sign bit */
	  "idivw             %%bx \n\t"	/* IDIV - VERY EXPENSIVE */
	  "movd      %%eax, %%mm7 \n\t"	/* move result of division into MM7 */
	  "packuswb  %%mm0, %%mm7 \n\t"	/* pack division result with saturation */
	   "movd      %%mm7, %%eax \n\t"	/* copy saturated result into EAX */
	  "mov      %%al, (%%edi) \n\t"	/* copy a byte result into Dest */
	   "movd      %%mm3, %%edx \n\t"	/* restore saved EDX */
	  "movd      %%mm2, %%ebx \n\t"	/* restore saved EBX */
	  "movd      %%mm1, %%eax \n\t"	/* restore saved EAX */
/* -- */
	  "movd      %%mm6, %%esi \n\t"	/* move Src pointer to the top pixel */
	  "sub         $72, %%edx \n\t"	/* EDX = Kernel address */
	  "inc              %%esi \n\t"	/* move Src  pointer to the next pixel */
	  "inc              %%edi \n\t"	/* move Dest pointer to the next pixel */
/* --- */
	  "dec              %%ecx \n\t"	/* decrease loop counter COLUMNS */
	  "jnz            .L10332 \n\t"	/* check loop termination, proceed if required */
	   "add          $4, %%esi \n\t"	/* move to the next row in Src */
	  "add          $4, %%edi \n\t"	/* move to the next row in Dest */
	   "dec              %%ebx \n\t"	/* decrease loop counter ROWS */
	  "jnz            .L10330 \n\t"	/* check loop termination, proceed if required */
/* --- */
	  "emms                   \n\t"	/* exit MMX state */
	  "popa                   \n\t":"=m" (Dest)	/* %0 */
	  :"m"(Src),		/* %1 */
	  "m"(rows),		/* %2 */
	  "m"(columns),		/* %3 */
	  "m"(Kernel),		/* %4 */
	  "m"(Divisor)		/* %5 */
	    );
#endif
	return (0);
    } else {
	/* No non-MMX implementation yet */
	return (-1);
    }
}

/*  SDL_imageFilterConvolveKernel7x7Divide: Dij = saturation0and255( ... ) */
int SDL_imageFilterConvolveKernel7x7Divide(unsigned char *Src, unsigned char *Dest, int rows, int columns,
					   signed short *Kernel, unsigned char Divisor)
{
    if ((columns < 7) || (rows < 7) || (Divisor == 0))
	return (-1);

    if ((SDL_imageFilterMMXdetect())) {
#ifdef USE_MMX
	asm volatile
	 ("pusha		     \n\t" "pxor      %%mm0, %%mm0 \n\t"	/* zero MM0 */
	   "xor       %%ebx, %%ebx \n\t"	/* zero EBX */
	  "mov           %5, %%bl \n\t"	/* load Divisor into BL */
	  "movd      %%ebx, %%mm5 \n\t"	/* copy Divisor into MM5 */
	   "mov          %4, %%edx \n\t"	/* load Kernel address into EDX */
	  "mov          %1, %%esi \n\t"	/* load Src  address to ESI */
	  "mov          %0, %%edi \n\t"	/* load Dest address to EDI */
	  "add          $3, %%edi \n\t"	/* 3 column offset from the left edge */
	  "mov          %3, %%eax \n\t"	/* load columns into EAX */
	  "add       %%eax, %%edi \n\t"	/* 3 row offset from the top edge */
	  "add       %%eax, %%edi \n\t" "add       %%eax, %%edi \n\t" "mov          %2, %%ebx \n\t"	/* initialize ROWS counter */
	  "sub          $6, %%ebx \n\t"	/* do not use first 3 and last 3 rows */
/* --- */
	  ".L10340:               \n\t" "mov       %%eax, %%ecx \n\t"	/* initialize COLUMNS counter */
	  "sub          $6, %%ecx \n\t"	/* do not use first 3 and last 3 columns */
	   ".align 16              \n\t"	/* 16 byte allignment of the loop entry */
	  ".L10342:               \n\t" "pxor      %%mm7, %%mm7 \n\t"	/* zero MM7 (accumulator) */
	  "movd      %%esi, %%mm6 \n\t"	/* save ESI in MM6 */
/* --- 1 */
	  "movq    (%%esi), %%mm1 \n\t"	/* load 8 bytes of the Src */
	  "movq      %%mm1, %%mm2 \n\t"	/* copy MM1 into MM2 */
	  "add       %%eax, %%esi \n\t"	/* move Src pointer 1 row below */
	  "movq    (%%edx), %%mm3 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	  "movq    (%%edx), %%mm4 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	   "punpcklbw %%mm0, %%mm1 \n\t"	/* unpack first  4 bytes into words */
	  "punpckhbw %%mm0, %%mm2 \n\t"	/* unpack second 4 bytes into words */
	   "pmullw    %%mm3, %%mm1 \n\t"	/* mult. 4 low  words of Src and Kernel */
	  "pmullw    %%mm4, %%mm2 \n\t"	/* mult. 4 high words of Src and Kernel */
	   "paddsw    %%mm2, %%mm1 \n\t"	/* add 4 words of the high and low bytes */
	  "paddsw    %%mm1, %%mm7 \n\t"	/* add MM1 to accumulator MM7 */
/* --- 2 */
	  "movq    (%%esi), %%mm1 \n\t"	/* load 8 bytes of the Src */
	  "movq      %%mm1, %%mm2 \n\t"	/* copy MM1 into MM2 */
	  "add       %%eax, %%esi \n\t"	/* move Src pointer 1 row below */
	  "movq    (%%edx), %%mm3 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	  "movq    (%%edx), %%mm4 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	   "punpcklbw %%mm0, %%mm1 \n\t"	/* unpack first  4 bytes into words */
	  "punpckhbw %%mm0, %%mm2 \n\t"	/* unpack second 4 bytes into words */
	   "pmullw    %%mm3, %%mm1 \n\t"	/* mult. 4 low  words of Src and Kernel */
	  "pmullw    %%mm4, %%mm2 \n\t"	/* mult. 4 high words of Src and Kernel */
	   "paddsw    %%mm2, %%mm1 \n\t"	/* add 4 words of the high and low bytes */
	  "paddsw    %%mm1, %%mm7 \n\t"	/* add MM1 to accumulator MM7 */
/* --- 3 */
	  "movq    (%%esi), %%mm1 \n\t"	/* load 8 bytes of the Src */
	  "movq      %%mm1, %%mm2 \n\t"	/* copy MM1 into MM2 */
	  "add       %%eax, %%esi \n\t"	/* move Src pointer 1 row below */
	  "movq    (%%edx), %%mm3 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	  "movq    (%%edx), %%mm4 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	   "punpcklbw %%mm0, %%mm1 \n\t"	/* unpack first  4 bytes into words */
	  "punpckhbw %%mm0, %%mm2 \n\t"	/* unpack second 4 bytes into words */
	   "pmullw    %%mm3, %%mm1 \n\t"	/* mult. 4 low  words of Src and Kernel */
	  "pmullw    %%mm4, %%mm2 \n\t"	/* mult. 4 high words of Src and Kernel */
	   "paddsw    %%mm2, %%mm1 \n\t"	/* add 4 words of the high and low bytes */
	  "paddsw    %%mm1, %%mm7 \n\t"	/* add MM1 to accumulator MM7 */
/* --- 4 */
	  "movq    (%%esi), %%mm1 \n\t"	/* load 8 bytes of the Src */
	  "movq      %%mm1, %%mm2 \n\t"	/* copy MM1 into MM2 */
	  "add       %%eax, %%esi \n\t"	/* move Src pointer 1 row below */
	  "movq    (%%edx), %%mm3 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	  "movq    (%%edx), %%mm4 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	   "punpcklbw %%mm0, %%mm1 \n\t"	/* unpack first  4 bytes into words */
	  "punpckhbw %%mm0, %%mm2 \n\t"	/* unpack second 4 bytes into words */
	   "pmullw    %%mm3, %%mm1 \n\t"	/* mult. 4 low  words of Src and Kernel */
	  "pmullw    %%mm4, %%mm2 \n\t"	/* mult. 4 high words of Src and Kernel */
	   "paddsw    %%mm2, %%mm1 \n\t"	/* add 4 words of the high and low bytes */
	  "paddsw    %%mm1, %%mm7 \n\t"	/* add MM1 to accumulator MM7 */
/* --- 5 */
	  "movq    (%%esi), %%mm1 \n\t"	/* load 8 bytes of the Src */
	  "movq      %%mm1, %%mm2 \n\t"	/* copy MM1 into MM2 */
	  "add       %%eax, %%esi \n\t"	/* move Src pointer 1 row below */
	  "movq    (%%edx), %%mm3 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	  "movq    (%%edx), %%mm4 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	   "punpcklbw %%mm0, %%mm1 \n\t"	/* unpack first  4 bytes into words */
	  "punpckhbw %%mm0, %%mm2 \n\t"	/* unpack second 4 bytes into words */
	   "pmullw    %%mm3, %%mm1 \n\t"	/* mult. 4 low  words of Src and Kernel */
	  "pmullw    %%mm4, %%mm2 \n\t"	/* mult. 4 high words of Src and Kernel */
	   "paddsw    %%mm2, %%mm1 \n\t"	/* add 4 words of the high and low bytes */
	  "paddsw    %%mm1, %%mm7 \n\t"	/* add MM1 to accumulator MM7 */
/* --- 6 */
	  "movq    (%%esi), %%mm1 \n\t"	/* load 8 bytes of the Src */
	  "movq      %%mm1, %%mm2 \n\t"	/* copy MM1 into MM2 */
	  "add       %%eax, %%esi \n\t"	/* move Src pointer 1 row below */
	  "movq    (%%edx), %%mm3 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	  "movq    (%%edx), %%mm4 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	   "punpcklbw %%mm0, %%mm1 \n\t"	/* unpack first  4 bytes into words */
	  "punpckhbw %%mm0, %%mm2 \n\t"	/* unpack second 4 bytes into words */
	   "pmullw    %%mm3, %%mm1 \n\t"	/* mult. 4 low  words of Src and Kernel */
	  "pmullw    %%mm4, %%mm2 \n\t"	/* mult. 4 high words of Src and Kernel */
	   "paddsw    %%mm2, %%mm1 \n\t"	/* add 4 words of the high and low bytes */
	  "paddsw    %%mm1, %%mm7 \n\t"	/* add MM1 to accumulator MM7 */
/* --- 7 */
	  "movq    (%%esi), %%mm1 \n\t"	/* load 8 bytes of the Src */
	  "movq      %%mm1, %%mm2 \n\t"	/* copy MM1 into MM2 */
	  "movq    (%%edx), %%mm3 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	  "movq    (%%edx), %%mm4 \n\t"	/* load 4 words of Kernel */
	   "punpcklbw %%mm0, %%mm1 \n\t"	/* unpack first  4 bytes into words */
	  "punpckhbw %%mm0, %%mm2 \n\t"	/* unpack second 4 bytes into words */
	   "pmullw    %%mm3, %%mm1 \n\t"	/* mult. 4 low  words of Src and Kernel */
	  "pmullw    %%mm4, %%mm2 \n\t"	/* mult. 4 high words of Src and Kernel */
	   "paddsw    %%mm2, %%mm1 \n\t"	/* add 4 words of the high and low bytes */
	  "paddsw    %%mm1, %%mm7 \n\t"	/* add MM1 to accumulator MM7 */
/* --- */
	  "movq      %%mm7, %%mm3 \n\t"	/* copy MM7 into MM3 */
	  "psrlq       $32, %%mm7 \n\t"	/* shift 2 left words to the right */
	  "paddsw    %%mm3, %%mm7 \n\t"	/* add 2 left and 2 right result words */
	  "movq      %%mm7, %%mm2 \n\t"	/* copy MM7 into MM2 */
	  "psrlq       $16, %%mm7 \n\t"	/* shift 1 left word to the right */
	  "paddsw    %%mm2, %%mm7 \n\t"	/* add 1 left and 1 right result words */
/* --- */
	  "movd      %%eax, %%mm1 \n\t"	/* save EDX in MM1 */
	  "movd      %%ebx, %%mm2 \n\t"	/* save EDX in MM2 */
	  "movd      %%edx, %%mm3 \n\t"	/* save EDX in MM3 */
	   "movd      %%mm7, %%eax \n\t"	/* load summation result into EAX */
	  "psraw       $15, %%mm7 \n\t"	/* spread sign bit of the result */
	  "movd      %%mm5, %%ebx \n\t"	/* load Divisor into EBX */
	  "movd      %%mm7, %%edx \n\t"	/* fill EDX with a sign bit */
	  "idivw             %%bx \n\t"	/* IDIV - VERY EXPENSIVE */
	  "movd      %%eax, %%mm7 \n\t"	/* move result of division into MM7 */
	  "packuswb  %%mm0, %%mm7 \n\t"	/* pack division result with saturation */
	   "movd      %%mm7, %%eax \n\t"	/* copy saturated result into EAX */
	  "mov      %%al, (%%edi) \n\t"	/* copy a byte result into Dest */
	   "movd      %%mm3, %%edx \n\t"	/* restore saved EDX */
	  "movd      %%mm2, %%ebx \n\t"	/* restore saved EBX */
	  "movd      %%mm1, %%eax \n\t"	/* restore saved EAX */
/* -- */
	  "movd      %%mm6, %%esi \n\t"	/* move Src pointer to the top pixel */
	  "sub        $104, %%edx \n\t"	/* EDX = Kernel address */
	  "inc              %%esi \n\t"	/* move Src  pointer to the next pixel */
	  "inc              %%edi \n\t"	/* move Dest pointer to the next pixel */
/* --- */
	  "dec              %%ecx \n\t"	/* decrease loop counter COLUMNS */
	  "jnz            .L10342 \n\t"	/* check loop termination, proceed if required */
	   "add          $6, %%esi \n\t"	/* move to the next row in Src */
	  "add          $6, %%edi \n\t"	/* move to the next row in Dest */
	   "dec              %%ebx \n\t"	/* decrease loop counter ROWS */
	  "jnz            .L10340 \n\t"	/* check loop termination, proceed if required */
/* --- */
	  "emms                   \n\t"	/* exit MMX state */
	  "popa                   \n\t":"=m" (Dest)	/* %0 */
	  :"m"(Src),		/* %1 */
	  "m"(rows),		/* %2 */
	  "m"(columns),		/* %3 */
	  "m"(Kernel),		/* %4 */
	  "m"(Divisor)		/* %5 */
	    );
#endif
	return (0);
    } else {
	/* No non-MMX implementation yet */
	return (-1);
    }
}

/*  SDL_imageFilterConvolveKernel9x9Divide: Dij = saturation0and255( ... ) */
int SDL_imageFilterConvolveKernel9x9Divide(unsigned char *Src, unsigned char *Dest, int rows, int columns,
					   signed short *Kernel, unsigned char Divisor)
{
    if ((columns < 9) || (rows < 9) || (Divisor == 0))
	return (-1);

    if ((SDL_imageFilterMMXdetect())) {
#ifdef USE_MMX
	asm volatile
	 ("pusha		     \n\t" "pxor      %%mm0, %%mm0 \n\t"	/* zero MM0 */
	   "xor       %%ebx, %%ebx \n\t"	/* zero EBX */
	  "mov           %5, %%bl \n\t"	/* load Divisor into BL */
	  "movd      %%ebx, %%mm5 \n\t"	/* copy Divisor into MM5 */
	   "mov          %4, %%edx \n\t"	/* load Kernel address into EDX */
	  "mov          %1, %%esi \n\t"	/* load Src  address to ESI */
	  "mov          %0, %%edi \n\t"	/* load Dest address to EDI */
	  "add          $4, %%edi \n\t"	/* 4 column offset from the left edge */
	  "mov          %3, %%eax \n\t"	/* load columns into EAX */
	  "add       %%eax, %%edi \n\t"	/* 4 row offset from the top edge */
	  "add       %%eax, %%edi \n\t" "add       %%eax, %%edi \n\t" "add       %%eax, %%edi \n\t" "mov          %2, %%ebx \n\t"	/* initialize ROWS counter */
	  "sub          $8, %%ebx \n\t"	/* do not use first 4 and last 4 rows */
/* --- */
	  ".L10350:               \n\t" "mov       %%eax, %%ecx \n\t"	/* initialize COLUMNS counter */
	  "sub          $8, %%ecx \n\t"	/* do not use first 4 and last 4 columns */
	   ".align 16              \n\t"	/* 16 byte allignment of the loop entry */
	  ".L10352:               \n\t" "pxor      %%mm7, %%mm7 \n\t"	/* zero MM7 (accumulator) */
	  "movd      %%esi, %%mm6 \n\t"	/* save ESI in MM6 */
/* --- 1 */
	  "movq    (%%esi), %%mm1 \n\t"	/* load 8 bytes of the Src */
	  "movq      %%mm1, %%mm2 \n\t"	/* copy MM1 into MM2 */
	  "inc              %%esi \n\t"	/* move pointer to the next 8 bytes of Src */
	  "movq    (%%edx), %%mm3 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	  "movq    (%%edx), %%mm4 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	   "punpcklbw %%mm0, %%mm1 \n\t"	/* unpack first  4 bytes into words */
	  "punpckhbw %%mm0, %%mm2 \n\t"	/* unpack second 4 bytes into words */
	   "pmullw    %%mm3, %%mm1 \n\t"	/* mult. 4 low  words of Src and Kernel */
	  "pmullw    %%mm4, %%mm2 \n\t"	/* mult. 4 high words of Src and Kernel */
	   "paddsw    %%mm2, %%mm1 \n\t"	/* add 4 words of the high and low bytes */
	  "paddsw    %%mm1, %%mm7 \n\t"	/* add MM1 to accumulator MM7 */
	   "movq    (%%esi), %%mm1 \n\t"	/* load 8 bytes of the Src */
	  "dec              %%esi \n\t" "add       %%eax, %%esi \n\t"	/* move Src pointer 1 row below */
	  "movq    (%%edx), %%mm3 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	   "punpcklbw %%mm0, %%mm1 \n\t"	/* unpack first  4 bytes into words */
	  "pmullw    %%mm3, %%mm1 \n\t"	/* mult. 4 low  words of Src and Kernel */
	  "paddsw    %%mm1, %%mm7 \n\t"	/* add MM1 to accumulator MM7 */
/* --- 2 */
	  "movq    (%%esi), %%mm1 \n\t"	/* load 8 bytes of the Src */
	  "movq      %%mm1, %%mm2 \n\t"	/* copy MM1 into MM2 */
	  "inc              %%esi \n\t"	/* move pointer to the next 8 bytes of Src */
	  "movq    (%%edx), %%mm3 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	  "movq    (%%edx), %%mm4 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	   "punpcklbw %%mm0, %%mm1 \n\t"	/* unpack first  4 bytes into words */
	  "punpckhbw %%mm0, %%mm2 \n\t"	/* unpack second 4 bytes into words */
	   "pmullw    %%mm3, %%mm1 \n\t"	/* mult. 4 low  words of Src and Kernel */
	  "pmullw    %%mm4, %%mm2 \n\t"	/* mult. 4 high words of Src and Kernel */
	   "paddsw    %%mm2, %%mm1 \n\t"	/* add 4 words of the high and low bytes */
	  "paddsw    %%mm1, %%mm7 \n\t"	/* add MM1 to accumulator MM7 */
	   "movq    (%%esi), %%mm1 \n\t"	/* load 8 bytes of the Src */
	  "dec              %%esi \n\t" "add       %%eax, %%esi \n\t"	/* move Src pointer 1 row below */
	  "movq    (%%edx), %%mm3 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	   "punpcklbw %%mm0, %%mm1 \n\t"	/* unpack first  4 bytes into words */
	  "pmullw    %%mm3, %%mm1 \n\t"	/* mult. 4 low  words of Src and Kernel */
	  "paddsw    %%mm1, %%mm7 \n\t"	/* add MM1 to accumulator MM7 */
/* --- 3 */
	  "movq    (%%esi), %%mm1 \n\t"	/* load 8 bytes of the Src */
	  "movq      %%mm1, %%mm2 \n\t"	/* copy MM1 into MM2 */
	  "inc              %%esi \n\t"	/* move pointer to the next 8 bytes of Src */
	  "movq    (%%edx), %%mm3 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	  "movq    (%%edx), %%mm4 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	   "punpcklbw %%mm0, %%mm1 \n\t"	/* unpack first  4 bytes into words */
	  "punpckhbw %%mm0, %%mm2 \n\t"	/* unpack second 4 bytes into words */
	   "pmullw    %%mm3, %%mm1 \n\t"	/* mult. 4 low  words of Src and Kernel */
	  "pmullw    %%mm4, %%mm2 \n\t"	/* mult. 4 high words of Src and Kernel */
	   "paddsw    %%mm2, %%mm1 \n\t"	/* add 4 words of the high and low bytes */
	  "paddsw    %%mm1, %%mm7 \n\t"	/* add MM1 to accumulator MM7 */
	   "movq    (%%esi), %%mm1 \n\t"	/* load 8 bytes of the Src */
	  "dec              %%esi \n\t" "add       %%eax, %%esi \n\t"	/* move Src pointer 1 row below */
	  "movq    (%%edx), %%mm3 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	   "punpcklbw %%mm0, %%mm1 \n\t"	/* unpack first  4 bytes into words */
	  "pmullw    %%mm3, %%mm1 \n\t"	/* mult. 4 low  words of Src and Kernel */
	  "paddsw    %%mm1, %%mm7 \n\t"	/* add MM1 to accumulator MM7 */
/* --- 4 */
	  "movq    (%%esi), %%mm1 \n\t"	/* load 8 bytes of the Src */
	  "movq      %%mm1, %%mm2 \n\t"	/* copy MM1 into MM2 */
	  "inc              %%esi \n\t"	/* move pointer to the next 8 bytes of Src */
	  "movq    (%%edx), %%mm3 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	  "movq    (%%edx), %%mm4 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	   "punpcklbw %%mm0, %%mm1 \n\t"	/* unpack first  4 bytes into words */
	  "punpckhbw %%mm0, %%mm2 \n\t"	/* unpack second 4 bytes into words */
	   "pmullw    %%mm3, %%mm1 \n\t"	/* mult. 4 low  words of Src and Kernel */
	  "pmullw    %%mm4, %%mm2 \n\t"	/* mult. 4 high words of Src and Kernel */
	   "paddsw    %%mm2, %%mm1 \n\t"	/* add 4 words of the high and low bytes */
	  "paddsw    %%mm1, %%mm7 \n\t"	/* add MM1 to accumulator MM7 */
	   "movq    (%%esi), %%mm1 \n\t"	/* load 8 bytes of the Src */
	  "dec              %%esi \n\t" "add       %%eax, %%esi \n\t"	/* move Src pointer 1 row below */
	  "movq    (%%edx), %%mm3 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	   "punpcklbw %%mm0, %%mm1 \n\t"	/* unpack first  4 bytes into words */
	  "pmullw    %%mm3, %%mm1 \n\t"	/* mult. 4 low  words of Src and Kernel */
	  "paddsw    %%mm1, %%mm7 \n\t"	/* add MM1 to accumulator MM7 */
/* --- 5 */
	  "movq    (%%esi), %%mm1 \n\t"	/* load 8 bytes of the Src */
	  "movq      %%mm1, %%mm2 \n\t"	/* copy MM1 into MM2 */
	  "inc              %%esi \n\t"	/* move pointer to the next 8 bytes of Src */
	  "movq    (%%edx), %%mm3 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	  "movq    (%%edx), %%mm4 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	   "punpcklbw %%mm0, %%mm1 \n\t"	/* unpack first  4 bytes into words */
	  "punpckhbw %%mm0, %%mm2 \n\t"	/* unpack second 4 bytes into words */
	   "pmullw    %%mm3, %%mm1 \n\t"	/* mult. 4 low  words of Src and Kernel */
	  "pmullw    %%mm4, %%mm2 \n\t"	/* mult. 4 high words of Src and Kernel */
	   "paddsw    %%mm2, %%mm1 \n\t"	/* add 4 words of the high and low bytes */
	  "paddsw    %%mm1, %%mm7 \n\t"	/* add MM1 to accumulator MM7 */
	   "movq    (%%esi), %%mm1 \n\t"	/* load 8 bytes of the Src */
	  "dec              %%esi \n\t" "add       %%eax, %%esi \n\t"	/* move Src pointer 1 row below */
	  "movq    (%%edx), %%mm3 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	   "punpcklbw %%mm0, %%mm1 \n\t"	/* unpack first  4 bytes into words */
	  "pmullw    %%mm3, %%mm1 \n\t"	/* mult. 4 low  words of Src and Kernel */
	  "paddsw    %%mm1, %%mm7 \n\t"	/* add MM1 to accumulator MM7 */
/* --- 6 */
	  "movq    (%%esi), %%mm1 \n\t"	/* load 8 bytes of the Src */
	  "movq      %%mm1, %%mm2 \n\t"	/* copy MM1 into MM2 */
	  "inc              %%esi \n\t"	/* move pointer to the next 8 bytes of Src */
	  "movq    (%%edx), %%mm3 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	  "movq    (%%edx), %%mm4 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	   "punpcklbw %%mm0, %%mm1 \n\t"	/* unpack first  4 bytes into words */
	  "punpckhbw %%mm0, %%mm2 \n\t"	/* unpack second 4 bytes into words */
	   "pmullw    %%mm3, %%mm1 \n\t"	/* mult. 4 low  words of Src and Kernel */
	  "pmullw    %%mm4, %%mm2 \n\t"	/* mult. 4 high words of Src and Kernel */
	   "paddsw    %%mm2, %%mm1 \n\t"	/* add 4 words of the high and low bytes */
	  "paddsw    %%mm1, %%mm7 \n\t"	/* add MM1 to accumulator MM7 */
	   "movq    (%%esi), %%mm1 \n\t"	/* load 8 bytes of the Src */
	  "dec              %%esi \n\t" "add       %%eax, %%esi \n\t"	/* move Src pointer 1 row below */
	  "movq    (%%edx), %%mm3 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	   "punpcklbw %%mm0, %%mm1 \n\t"	/* unpack first  4 bytes into words */
	  "pmullw    %%mm3, %%mm1 \n\t"	/* mult. 4 low  words of Src and Kernel */
	  "paddsw    %%mm1, %%mm7 \n\t"	/* add MM1 to accumulator MM7 */
/* --- 7 */
	  "movq    (%%esi), %%mm1 \n\t"	/* load 8 bytes of the Src */
	  "movq      %%mm1, %%mm2 \n\t"	/* copy MM1 into MM2 */
	  "inc              %%esi \n\t"	/* move pointer to the next 8 bytes of Src */
	  "movq    (%%edx), %%mm3 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	  "movq    (%%edx), %%mm4 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	   "punpcklbw %%mm0, %%mm1 \n\t"	/* unpack first  4 bytes into words */
	  "punpckhbw %%mm0, %%mm2 \n\t"	/* unpack second 4 bytes into words */
	   "pmullw    %%mm3, %%mm1 \n\t"	/* mult. 4 low  words of Src and Kernel */
	  "pmullw    %%mm4, %%mm2 \n\t"	/* mult. 4 high words of Src and Kernel */
	   "paddsw    %%mm2, %%mm1 \n\t"	/* add 4 words of the high and low bytes */
	  "paddsw    %%mm1, %%mm7 \n\t"	/* add MM1 to accumulator MM7 */
	   "movq    (%%esi), %%mm1 \n\t"	/* load 8 bytes of the Src */
	  "dec              %%esi \n\t" "add       %%eax, %%esi \n\t"	/* move Src pointer 1 row below */
	  "movq    (%%edx), %%mm3 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	   "punpcklbw %%mm0, %%mm1 \n\t"	/* unpack first  4 bytes into words */
	  "pmullw    %%mm3, %%mm1 \n\t"	/* mult. 4 low  words of Src and Kernel */
	  "paddsw    %%mm1, %%mm7 \n\t"	/* add MM1 to accumulator MM7 */
/* --- 8 */
	  "movq    (%%esi), %%mm1 \n\t"	/* load 8 bytes of the Src */
	  "movq      %%mm1, %%mm2 \n\t"	/* copy MM1 into MM2 */
	  "inc              %%esi \n\t"	/* move pointer to the next 8 bytes of Src */
	  "movq    (%%edx), %%mm3 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	  "movq    (%%edx), %%mm4 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	   "punpcklbw %%mm0, %%mm1 \n\t"	/* unpack first  4 bytes into words */
	  "punpckhbw %%mm0, %%mm2 \n\t"	/* unpack second 4 bytes into words */
	   "pmullw    %%mm3, %%mm1 \n\t"	/* mult. 4 low  words of Src and Kernel */
	  "pmullw    %%mm4, %%mm2 \n\t"	/* mult. 4 high words of Src and Kernel */
	   "paddsw    %%mm2, %%mm1 \n\t"	/* add 4 words of the high and low bytes */
	  "paddsw    %%mm1, %%mm7 \n\t"	/* add MM1 to accumulator MM7 */
	   "movq    (%%esi), %%mm1 \n\t"	/* load 8 bytes of the Src */
	  "dec              %%esi \n\t" "add       %%eax, %%esi \n\t"	/* move Src pointer 1 row below */
	  "movq    (%%edx), %%mm3 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	   "punpcklbw %%mm0, %%mm1 \n\t"	/* unpack first  4 bytes into words */
	  "pmullw    %%mm3, %%mm1 \n\t"	/* mult. 4 low  words of Src and Kernel */
	  "paddsw    %%mm1, %%mm7 \n\t"	/* add MM1 to accumulator MM7 */
/* --- 9 */
	  "movq    (%%esi), %%mm1 \n\t"	/* load 8 bytes of the Src */
	  "movq      %%mm1, %%mm2 \n\t"	/* copy MM1 into MM2 */
	  "inc              %%esi \n\t"	/* move pointer to the next 8 bytes of Src */
	  "movq    (%%edx), %%mm3 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	  "movq    (%%edx), %%mm4 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	   "punpcklbw %%mm0, %%mm1 \n\t"	/* unpack first  4 bytes into words */
	  "punpckhbw %%mm0, %%mm2 \n\t"	/* unpack second 4 bytes into words */
	   "pmullw    %%mm3, %%mm1 \n\t"	/* mult. 4 low  words of Src and Kernel */
	  "pmullw    %%mm4, %%mm2 \n\t"	/* mult. 4 high words of Src and Kernel */
	   "paddsw    %%mm2, %%mm1 \n\t"	/* add 4 words of the high and low bytes */
	  "paddsw    %%mm1, %%mm7 \n\t"	/* add MM1 to accumulator MM7 */
	   "movq    (%%esi), %%mm1 \n\t"	/* load 8 bytes of the Src */
	  "movq    (%%edx), %%mm3 \n\t"	/* load 4 words of Kernel */
	   "punpcklbw %%mm0, %%mm1 \n\t"	/* unpack first  4 bytes into words */
	  "pmullw    %%mm3, %%mm1 \n\t"	/* mult. 4 low  words of Src and Kernel */
	  "paddsw    %%mm1, %%mm7 \n\t"	/* add MM1 to accumulator MM7 */
/* --- */
	  "movq      %%mm7, %%mm3 \n\t"	/* copy MM7 into MM3 */
	  "psrlq       $32, %%mm7 \n\t"	/* shift 2 left words to the right */
	  "paddsw    %%mm3, %%mm7 \n\t"	/* add 2 left and 2 right result words */
	  "movq      %%mm7, %%mm2 \n\t"	/* copy MM7 into MM2 */
	  "psrlq       $16, %%mm7 \n\t"	/* shift 1 left word to the right */
	  "paddsw    %%mm2, %%mm7 \n\t"	/* add 1 left and 1 right result words */
/* --- */
	  "movd      %%eax, %%mm1 \n\t"	/* save EDX in MM1 */
	  "movd      %%ebx, %%mm2 \n\t"	/* save EDX in MM2 */
	  "movd      %%edx, %%mm3 \n\t"	/* save EDX in MM3 */
	   "movd      %%mm7, %%eax \n\t"	/* load summation result into EAX */
	  "psraw       $15, %%mm7 \n\t"	/* spread sign bit of the result */
	  "movd      %%mm5, %%ebx \n\t"	/* load Divisor into EBX */
	  "movd      %%mm7, %%edx \n\t"	/* fill EDX with a sign bit */
	  "idivw             %%bx \n\t"	/* IDIV - VERY EXPENSIVE */
	  "movd      %%eax, %%mm7 \n\t"	/* move result of division into MM7 */
	  "packuswb  %%mm0, %%mm7 \n\t"	/* pack division result with saturation */
	   "movd      %%mm7, %%eax \n\t"	/* copy saturated result into EAX */
	  "mov      %%al, (%%edi) \n\t"	/* copy a byte result into Dest */
	   "movd      %%mm3, %%edx \n\t"	/* restore saved EDX */
	  "movd      %%mm2, %%ebx \n\t"	/* restore saved EBX */
	  "movd      %%mm1, %%eax \n\t"	/* restore saved EAX */
/* -- */
	  "movd      %%mm6, %%esi \n\t"	/* move Src pointer to the top pixel */
	  "sub        $208, %%edx \n\t"	/* EDX = Kernel address */
	  "inc              %%esi \n\t"	/* move Src  pointer to the next pixel */
	  "inc              %%edi \n\t"	/* move Dest pointer to the next pixel */
/* --- */
	  "dec              %%ecx \n\t"	/* decrease loop counter COLUMNS */
	  "jnz            .L10352 \n\t"	/* check loop termination, proceed if required */
	   "add          $8, %%esi \n\t"	/* move to the next row in Src */
	  "add          $8, %%edi \n\t"	/* move to the next row in Dest */
	   "dec              %%ebx \n\t"	/* decrease loop counter ROWS */
	  "jnz            .L10350 \n\t"	/* check loop termination, proceed if required */
/* --- */
	  "emms                   \n\t"	/* exit MMX state */
	  "popa                   \n\t":"=m" (Dest)	/* %0 */
	  :"m"(Src),		/* %1 */
	  "m"(rows),		/* %2 */
	  "m"(columns),		/* %3 */
	  "m"(Kernel),		/* %4 */
	  "m"(Divisor)		/* %5 */
	    );
#endif
	return (0);
    } else {
	/* No non-MMX implementation yet */
	return (-1);
    }
}

/*  SDL_imageFilterConvolveKernel3x3ShiftRight: Dij = saturation0and255( ... ) */
int SDL_imageFilterConvolveKernel3x3ShiftRight(unsigned char *Src, unsigned char *Dest, int rows, int columns,
					       signed short *Kernel, unsigned char NRightShift)
{
    if ((columns < 3) || (rows < 3) || (NRightShift > 7))
	return (-1);

    if ((SDL_imageFilterMMXdetect())) {
#ifdef USE_MMX
	asm volatile
	 ("pusha		     \n\t" "pxor      %%mm0, %%mm0 \n\t"	/* zero MM0 */
	   "xor       %%ebx, %%ebx \n\t"	/* zero EBX */
	  "mov           %5, %%bl \n\t"	/* load NRightShift into BL */
	  "movd      %%ebx, %%mm4 \n\t"	/* copy NRightShift into MM4 */
	   "mov          %4, %%edx \n\t"	/* load Kernel address into EDX */
	  "movq    (%%edx), %%mm5 \n\t"	/* MM5 = {0,K2,K1,K0} */
	  "add          $8, %%edx \n\t"	/* second row              |K0 K1 K2 0| */
	  "movq    (%%edx), %%mm6 \n\t"	/* MM6 = {0,K5,K4,K3}  K = |K3 K4 K5 0| */
	  "add          $8, %%edx \n\t"	/* third row               |K6 K7 K8 0| */
	  "movq    (%%edx), %%mm7 \n\t"	/* MM7 = {0,K8,K7,K6} */
/* --- */
	  "mov          %3, %%eax \n\t"	/* load columns into EAX */
	  "mov          %1, %%esi \n\t"	/* ESI = Src row 0 address */
	  "mov          %0, %%edi \n\t"	/* load Dest address to EDI */
	  "add       %%eax, %%edi \n\t"	/* EDI = EDI + columns */
	  "inc              %%edi \n\t"	/* 1 byte offset from the left edge */
	   "mov          %2, %%edx \n\t"	/* initialize ROWS counter */
	  "sub          $2, %%edx \n\t"	/* do not use first and last row */
/* --- */
	  ".L10360:               \n\t" "mov       %%eax, %%ecx \n\t"	/* initialize COLUMS counter */
	  "sub          $2, %%ecx \n\t"	/* do not use first and last column */
	   ".align 16              \n\t"	/* 16 byte allignment of the loop entry */
	  ".L10362:               \n\t"
/* --- */
	  "movq    (%%esi), %%mm1 \n\t"	/* load 8 bytes of the image first row */
	  "add       %%eax, %%esi \n\t"	/* move one row below */
	  "movq    (%%esi), %%mm2 \n\t"	/* load 8 bytes of the image second row */
	  "add       %%eax, %%esi \n\t"	/* move one row below */
	  "movq    (%%esi), %%mm3 \n\t"	/* load 8 bytes of the image third row */
	   "punpcklbw %%mm0, %%mm1 \n\t"	/* unpack first 4 bytes into words */
	  "punpcklbw %%mm0, %%mm2 \n\t"	/* unpack first 4 bytes into words */
	  "punpcklbw %%mm0, %%mm3 \n\t"	/* unpack first 4 bytes into words */
	   "psrlw     %%mm4, %%mm1 \n\t"	/* shift right each pixel NshiftRight times */
	  "psrlw     %%mm4, %%mm2 \n\t"	/* shift right each pixel NshiftRight times */
	  "psrlw     %%mm4, %%mm3 \n\t"	/* shift right each pixel NshiftRight times */
	   "pmullw    %%mm5, %%mm1 \n\t"	/* multiply words first row  image*Kernel */
	  "pmullw    %%mm6, %%mm2 \n\t"	/* multiply words second row image*Kernel */
	  "pmullw    %%mm7, %%mm3 \n\t"	/* multiply words third row  image*Kernel */
	   "paddsw    %%mm2, %%mm1 \n\t"	/* add 4 words of the first and second rows */
	  "paddsw    %%mm3, %%mm1 \n\t"	/* add 4 words of the third row and result */
	  "movq      %%mm1, %%mm2 \n\t"	/* copy MM1 into MM2 */
	  "psrlq       $32, %%mm1 \n\t"	/* shift 2 left words to the right */
	  "paddsw    %%mm2, %%mm1 \n\t"	/* add 2 left and 2 right result words */
	  "movq      %%mm1, %%mm3 \n\t"	/* copy MM1 into MM3 */
	  "psrlq       $16, %%mm1 \n\t"	/* shift 1 left word to the right */
	  "paddsw    %%mm3, %%mm1 \n\t"	/* add 1 left and 1 right result words */
	  "packuswb  %%mm0, %%mm1 \n\t"	/* pack shift result with saturation */
	  "movd      %%mm1, %%ebx \n\t"	/* copy saturated result into EBX */
	  "mov      %%bl, (%%edi) \n\t"	/* copy a byte result into Dest */
/* -- */
	  "sub       %%eax, %%esi \n\t"	/* move two rows up */
	  "sub       %%eax, %%esi \n\t" "inc              %%esi \n\t"	/* move Src  pointer to the next pixel */
	  "inc              %%edi \n\t"	/* move Dest pointer to the next pixel */
/* --- */
	  "dec              %%ecx \n\t"	/* decrease loop counter COLUMNS */
	  "jnz            .L10362 \n\t"	/* check loop termination, proceed if required */
	   "add          $2, %%esi \n\t"	/* move to the next row in Src */
	  "add          $2, %%edi \n\t"	/* move to the next row in Dest */
	   "dec              %%edx \n\t"	/* decrease loop counter ROWS */
	  "jnz            .L10360 \n\t"	/* check loop termination, proceed if required */
/* --- */
	  "emms                   \n\t"	/* exit MMX state */
	  "popa                   \n\t":"=m" (Dest)	/* %0 */
	  :"m"(Src),		/* %1 */
	  "m"(rows),		/* %2 */
	  "m"(columns),		/* %3 */
	  "m"(Kernel),		/* %4 */
	  "m"(NRightShift)	/* %5 */
	    );
#endif
	return (0);
    } else {
	/* No non-MMX implementation yet */
	return (-1);
    }
}

/*  SDL_imageFilterConvolveKernel5x5ShiftRight: Dij = saturation0and255( ... ) */
int SDL_imageFilterConvolveKernel5x5ShiftRight(unsigned char *Src, unsigned char *Dest, int rows, int columns,
					       signed short *Kernel, unsigned char NRightShift)
{
    if ((columns < 5) || (rows < 5) || (NRightShift > 7))
	return (-1);

    if ((SDL_imageFilterMMXdetect())) {
#ifdef USE_MMX
	asm volatile
	 ("pusha		     \n\t" "pxor      %%mm0, %%mm0 \n\t"	/* zero MM0 */
	   "xor       %%ebx, %%ebx \n\t"	/* zero EBX */
	  "mov           %5, %%bl \n\t"	/* load NRightShift into BL */
	  "movd      %%ebx, %%mm5 \n\t"	/* copy NRightShift into MM5 */
	   "mov          %4, %%edx \n\t"	/* load Kernel address into EDX */
	  "mov          %1, %%esi \n\t"	/* load Src  address to ESI */
	  "mov          %0, %%edi \n\t"	/* load Dest address to EDI */
	  "add          $2, %%edi \n\t"	/* 2 column offset from the left edge */
	  "mov          %3, %%eax \n\t"	/* load columns into EAX */
	  "shl          $1, %%eax \n\t"	/* EAX = columns * 2 */
	  "add       %%eax, %%edi \n\t"	/* 2 row offset from the top edge */
	  "shr          $1, %%eax \n\t"	/* EAX = columns */
	   "mov          %2, %%ebx \n\t"	/* initialize ROWS counter */
	  "sub          $4, %%ebx \n\t"	/* do not use first 2 and last 2 rows */
/* --- */
	  ".L10370:               \n\t" "mov       %%eax, %%ecx \n\t"	/* initialize COLUMNS counter */
	  "sub          $4, %%ecx \n\t"	/* do not use first 2 and last 2 columns */
	   ".align 16              \n\t"	/* 16 byte allignment of the loop entry */
	  ".L10372:               \n\t" "pxor      %%mm7, %%mm7 \n\t"	/* zero MM7 (accumulator) */
	  "movd      %%esi, %%mm6 \n\t"	/* save ESI in MM6 */
/* --- 1 */
	  "movq    (%%esi), %%mm1 \n\t"	/* load 8 bytes of the Src */
	  "movq      %%mm1, %%mm2 \n\t"	/* copy MM1 into MM2 */
	  "add       %%eax, %%esi \n\t"	/* move Src pointer 1 row below */
	  "movq    (%%edx), %%mm3 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	  "movq    (%%edx), %%mm4 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	   "punpcklbw %%mm0, %%mm1 \n\t"	/* unpack first  4 bytes into words */
	  "punpckhbw %%mm0, %%mm2 \n\t"	/* unpack second 4 bytes into words */
	   "psrlw     %%mm5, %%mm1 \n\t"	/* shift right each pixel NshiftRight times */
	  "psrlw     %%mm5, %%mm2 \n\t"	/* shift right each pixel NshiftRight times */
	   "pmullw    %%mm3, %%mm1 \n\t"	/* mult. 4 low  words of Src and Kernel */
	  "pmullw    %%mm4, %%mm2 \n\t"	/* mult. 4 high words of Src and Kernel */
	   "paddsw    %%mm2, %%mm1 \n\t"	/* add 4 words of the high and low bytes */
	  "paddsw    %%mm1, %%mm7 \n\t"	/* add MM1 to accumulator MM7 */
/* --- 2 */
	  "movq    (%%esi), %%mm1 \n\t"	/* load 8 bytes of the Src */
	  "movq      %%mm1, %%mm2 \n\t"	/* copy MM1 into MM2 */
	  "add       %%eax, %%esi \n\t"	/* move Src pointer 1 row below */
	  "movq    (%%edx), %%mm3 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	  "movq    (%%edx), %%mm4 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	   "punpcklbw %%mm0, %%mm1 \n\t"	/* unpack first  4 bytes into words */
	  "punpckhbw %%mm0, %%mm2 \n\t"	/* unpack second 4 bytes into words */
	   "psrlw     %%mm5, %%mm1 \n\t"	/* shift right each pixel NshiftRight times */
	  "psrlw     %%mm5, %%mm2 \n\t"	/* shift right each pixel NshiftRight times */
	   "pmullw    %%mm3, %%mm1 \n\t"	/* mult. 4 low  words of Src and Kernel */
	  "pmullw    %%mm4, %%mm2 \n\t"	/* mult. 4 high words of Src and Kernel */
	   "paddsw    %%mm2, %%mm1 \n\t"	/* add 4 words of the high and low bytes */
	  "paddsw    %%mm1, %%mm7 \n\t"	/* add MM1 to accumulator MM7 */
/* --- 3 */
	  "movq    (%%esi), %%mm1 \n\t"	/* load 8 bytes of the Src */
	  "movq      %%mm1, %%mm2 \n\t"	/* copy MM1 into MM2 */
	  "add       %%eax, %%esi \n\t"	/* move Src pointer 1 row below */
	  "movq    (%%edx), %%mm3 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	  "movq    (%%edx), %%mm4 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	   "punpcklbw %%mm0, %%mm1 \n\t"	/* unpack first  4 bytes into words */
	  "punpckhbw %%mm0, %%mm2 \n\t"	/* unpack second 4 bytes into words */
	   "psrlw     %%mm5, %%mm1 \n\t"	/* shift right each pixel NshiftRight times */
	  "psrlw     %%mm5, %%mm2 \n\t"	/* shift right each pixel NshiftRight times */
	   "pmullw    %%mm3, %%mm1 \n\t"	/* mult. 4 low  words of Src and Kernel */
	  "pmullw    %%mm4, %%mm2 \n\t"	/* mult. 4 high words of Src and Kernel */
	   "paddsw    %%mm2, %%mm1 \n\t"	/* add 4 words of the high and low bytes */
	  "paddsw    %%mm1, %%mm7 \n\t"	/* add MM1 to accumulator MM7 */
/* --- 4 */
	  "movq    (%%esi), %%mm1 \n\t"	/* load 8 bytes of the Src */
	  "movq      %%mm1, %%mm2 \n\t"	/* copy MM1 into MM2 */
	  "add       %%eax, %%esi \n\t"	/* move Src pointer 1 row below */
	  "movq    (%%edx), %%mm3 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	  "movq    (%%edx), %%mm4 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	   "punpcklbw %%mm0, %%mm1 \n\t"	/* unpack first  4 bytes into words */
	  "punpckhbw %%mm0, %%mm2 \n\t"	/* unpack second 4 bytes into words */
	   "psrlw     %%mm5, %%mm1 \n\t"	/* shift right each pixel NshiftRight times */
	  "psrlw     %%mm5, %%mm2 \n\t"	/* shift right each pixel NshiftRight times */
	   "pmullw    %%mm3, %%mm1 \n\t"	/* mult. 4 low  words of Src and Kernel */
	  "pmullw    %%mm4, %%mm2 \n\t"	/* mult. 4 high words of Src and Kernel */
	   "paddsw    %%mm2, %%mm1 \n\t"	/* add 4 words of the high and low bytes */
	  "paddsw    %%mm1, %%mm7 \n\t"	/* add MM1 to accumulator MM7 */
/* --- 5 */
	  "movq    (%%esi), %%mm1 \n\t"	/* load 8 bytes of the Src */
	  "movq      %%mm1, %%mm2 \n\t"	/* copy MM1 into MM2 */
	  "movq    (%%edx), %%mm3 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	  "movq    (%%edx), %%mm4 \n\t"	/* load 4 words of Kernel */
	   "punpcklbw %%mm0, %%mm1 \n\t"	/* unpack first  4 bytes into words */
	  "punpckhbw %%mm0, %%mm2 \n\t"	/* unpack second 4 bytes into words */
	   "psrlw     %%mm5, %%mm1 \n\t"	/* shift right each pixel NshiftRight times */
	  "psrlw     %%mm5, %%mm2 \n\t"	/* shift right each pixel NshiftRight times */
	   "pmullw    %%mm3, %%mm1 \n\t"	/* mult. 4 low  words of Src and Kernel */
	  "pmullw    %%mm4, %%mm2 \n\t"	/* mult. 4 high words of Src and Kernel */
	   "paddsw    %%mm2, %%mm1 \n\t"	/* add 4 words of the high and low bytes */
	  "paddsw    %%mm1, %%mm7 \n\t"	/* add MM1 to accumulator MM7 */
/* --- */
	  "movq      %%mm7, %%mm3 \n\t"	/* copy MM7 into MM3 */
	  "psrlq       $32, %%mm7 \n\t"	/* shift 2 left words to the right */
	  "paddsw    %%mm3, %%mm7 \n\t"	/* add 2 left and 2 right result words */
	  "movq      %%mm7, %%mm2 \n\t"	/* copy MM7 into MM2 */
	  "psrlq       $16, %%mm7 \n\t"	/* shift 1 left word to the right */
	  "paddsw    %%mm2, %%mm7 \n\t"	/* add 1 left and 1 right result words */
	  "movd      %%eax, %%mm1 \n\t"	/* save EAX in MM1 */
	  "packuswb  %%mm0, %%mm7 \n\t"	/* pack division result with saturation */
	  "movd      %%mm7, %%eax \n\t"	/* copy saturated result into EAX */
	  "mov      %%al, (%%edi) \n\t"	/* copy a byte result into Dest */
	  "movd      %%mm1, %%eax \n\t"	/* restore saved EAX */
/* -- */
	  "movd      %%mm6, %%esi \n\t"	/* move Src pointer to the top pixel */
	  "sub         $72, %%edx \n\t"	/* EDX = Kernel address */
	  "inc              %%esi \n\t"	/* move Src  pointer to the next pixel */
	  "inc              %%edi \n\t"	/* move Dest pointer to the next pixel */
/* --- */
	  "dec              %%ecx \n\t"	/* decrease loop counter COLUMNS */
	  "jnz            .L10372 \n\t"	/* check loop termination, proceed if required */
	   "add          $4, %%esi \n\t"	/* move to the next row in Src */
	  "add          $4, %%edi \n\t"	/* move to the next row in Dest */
	   "dec              %%ebx \n\t"	/* decrease loop counter ROWS */
	  "jnz            .L10370 \n\t"	/* check loop termination, proceed if required */
/* --- */
	  "emms                   \n\t"	/* exit MMX state */
	  "popa                   \n\t":"=m" (Dest)	/* %0 */
	  :"m"(Src),		/* %1 */
	  "m"(rows),		/* %2 */
	  "m"(columns),		/* %3 */
	  "m"(Kernel),		/* %4 */
	  "m"(NRightShift)	/* %5 */
	    );
#endif
	return (0);
    } else {
	/* No non-MMX implementation yet */
	return (-1);
    }
}

/*  SDL_imageFilterConvolveKernel7x7ShiftRight: Dij = saturation0and255( ... ) */
int SDL_imageFilterConvolveKernel7x7ShiftRight(unsigned char *Src, unsigned char *Dest, int rows, int columns,
					       signed short *Kernel, unsigned char NRightShift)
{
    if ((columns < 7) || (rows < 7) || (NRightShift > 7))
	return (-1);

    if ((SDL_imageFilterMMXdetect())) {
#ifdef USE_MMX
	asm volatile
	 ("pusha		     \n\t" "pxor      %%mm0, %%mm0 \n\t"	/* zero MM0 */
	   "xor       %%ebx, %%ebx \n\t"	/* zero EBX */
	  "mov           %5, %%bl \n\t"	/* load NRightShift into BL */
	  "movd      %%ebx, %%mm5 \n\t"	/* copy NRightShift into MM5 */
	   "mov          %4, %%edx \n\t"	/* load Kernel address into EDX */
	  "mov          %1, %%esi \n\t"	/* load Src  address to ESI */
	  "mov          %0, %%edi \n\t"	/* load Dest address to EDI */
	  "add          $3, %%edi \n\t"	/* 3 column offset from the left edge */
	  "mov          %3, %%eax \n\t"	/* load columns into EAX */
	  "add       %%eax, %%edi \n\t"	/* 3 row offset from the top edge */
	  "add       %%eax, %%edi \n\t" "add       %%eax, %%edi \n\t" "mov          %2, %%ebx \n\t"	/* initialize ROWS counter */
	  "sub          $6, %%ebx \n\t"	/* do not use first 3 and last 3 rows */
/* --- */
	  ".L10380:               \n\t" "mov       %%eax, %%ecx \n\t"	/* initialize COLUMNS counter */
	  "sub          $6, %%ecx \n\t"	/* do not use first 3 and last 3 columns */
	   ".align 16              \n\t"	/* 16 byte allignment of the loop entry */
	  ".L10382:               \n\t" "pxor      %%mm7, %%mm7 \n\t"	/* zero MM7 (accumulator) */
	  "movd      %%esi, %%mm6 \n\t"	/* save ESI in MM6 */
/* --- 1 */
	  "movq    (%%esi), %%mm1 \n\t"	/* load 8 bytes of the Src */
	  "movq      %%mm1, %%mm2 \n\t"	/* copy MM1 into MM2 */
	  "add       %%eax, %%esi \n\t"	/* move Src pointer 1 row below */
	  "movq    (%%edx), %%mm3 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	  "movq    (%%edx), %%mm4 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	   "punpcklbw %%mm0, %%mm1 \n\t"	/* unpack first  4 bytes into words */
	  "punpckhbw %%mm0, %%mm2 \n\t"	/* unpack second 4 bytes into words */
	   "psrlw     %%mm5, %%mm1 \n\t"	/* shift right each pixel NshiftRight times */
	  "psrlw     %%mm5, %%mm2 \n\t"	/* shift right each pixel NshiftRight times */
	   "pmullw    %%mm3, %%mm1 \n\t"	/* mult. 4 low  words of Src and Kernel */
	  "pmullw    %%mm4, %%mm2 \n\t"	/* mult. 4 high words of Src and Kernel */
	   "paddsw    %%mm2, %%mm1 \n\t"	/* add 4 words of the high and low bytes */
	  "paddsw    %%mm1, %%mm7 \n\t"	/* add MM1 to accumulator MM7 */
/* --- 2 */
	  "movq    (%%esi), %%mm1 \n\t"	/* load 8 bytes of the Src */
	  "movq      %%mm1, %%mm2 \n\t"	/* copy MM1 into MM2 */
	  "add       %%eax, %%esi \n\t"	/* move Src pointer 1 row below */
	  "movq    (%%edx), %%mm3 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	  "movq    (%%edx), %%mm4 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	   "punpcklbw %%mm0, %%mm1 \n\t"	/* unpack first  4 bytes into words */
	  "punpckhbw %%mm0, %%mm2 \n\t"	/* unpack second 4 bytes into words */
	   "psrlw     %%mm5, %%mm1 \n\t"	/* shift right each pixel NshiftRight times */
	  "psrlw     %%mm5, %%mm2 \n\t"	/* shift right each pixel NshiftRight times */
	   "pmullw    %%mm3, %%mm1 \n\t"	/* mult. 4 low  words of Src and Kernel */
	  "pmullw    %%mm4, %%mm2 \n\t"	/* mult. 4 high words of Src and Kernel */
	   "paddsw    %%mm2, %%mm1 \n\t"	/* add 4 words of the high and low bytes */
	  "paddsw    %%mm1, %%mm7 \n\t"	/* add MM1 to accumulator MM7 */
/* --- 3 */
	  "movq    (%%esi), %%mm1 \n\t"	/* load 8 bytes of the Src */
	  "movq      %%mm1, %%mm2 \n\t"	/* copy MM1 into MM2 */
	  "add       %%eax, %%esi \n\t"	/* move Src pointer 1 row below */
	  "movq    (%%edx), %%mm3 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	  "movq    (%%edx), %%mm4 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	   "punpcklbw %%mm0, %%mm1 \n\t"	/* unpack first  4 bytes into words */
	  "punpckhbw %%mm0, %%mm2 \n\t"	/* unpack second 4 bytes into words */
	   "psrlw     %%mm5, %%mm1 \n\t"	/* shift right each pixel NshiftRight times */
	  "psrlw     %%mm5, %%mm2 \n\t"	/* shift right each pixel NshiftRight times */
	   "pmullw    %%mm3, %%mm1 \n\t"	/* mult. 4 low  words of Src and Kernel */
	  "pmullw    %%mm4, %%mm2 \n\t"	/* mult. 4 high words of Src and Kernel */
	   "paddsw    %%mm2, %%mm1 \n\t"	/* add 4 words of the high and low bytes */
	  "paddsw    %%mm1, %%mm7 \n\t"	/* add MM1 to accumulator MM7 */
/* --- 4 */
	  "movq    (%%esi), %%mm1 \n\t"	/* load 8 bytes of the Src */
	  "movq      %%mm1, %%mm2 \n\t"	/* copy MM1 into MM2 */
	  "add       %%eax, %%esi \n\t"	/* move Src pointer 1 row below */
	  "movq    (%%edx), %%mm3 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	  "movq    (%%edx), %%mm4 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	   "punpcklbw %%mm0, %%mm1 \n\t"	/* unpack first  4 bytes into words */
	  "punpckhbw %%mm0, %%mm2 \n\t"	/* unpack second 4 bytes into words */
	   "psrlw     %%mm5, %%mm1 \n\t"	/* shift right each pixel NshiftRight times */
	  "psrlw     %%mm5, %%mm2 \n\t"	/* shift right each pixel NshiftRight times */
	   "pmullw    %%mm3, %%mm1 \n\t"	/* mult. 4 low  words of Src and Kernel */
	  "pmullw    %%mm4, %%mm2 \n\t"	/* mult. 4 high words of Src and Kernel */
	   "paddsw    %%mm2, %%mm1 \n\t"	/* add 4 words of the high and low bytes */
	  "paddsw    %%mm1, %%mm7 \n\t"	/* add MM1 to accumulator MM7 */
/* --- 5 */
	  "movq    (%%esi), %%mm1 \n\t"	/* load 8 bytes of the Src */
	  "movq      %%mm1, %%mm2 \n\t"	/* copy MM1 into MM2 */
	  "add       %%eax, %%esi \n\t"	/* move Src pointer 1 row below */
	  "movq    (%%edx), %%mm3 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	  "movq    (%%edx), %%mm4 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	   "punpcklbw %%mm0, %%mm1 \n\t"	/* unpack first  4 bytes into words */
	  "punpckhbw %%mm0, %%mm2 \n\t"	/* unpack second 4 bytes into words */
	   "psrlw     %%mm5, %%mm1 \n\t"	/* shift right each pixel NshiftRight times */
	  "psrlw     %%mm5, %%mm2 \n\t"	/* shift right each pixel NshiftRight times */
	   "pmullw    %%mm3, %%mm1 \n\t"	/* mult. 4 low  words of Src and Kernel */
	  "pmullw    %%mm4, %%mm2 \n\t"	/* mult. 4 high words of Src and Kernel */
	   "paddsw    %%mm2, %%mm1 \n\t"	/* add 4 words of the high and low bytes */
	  "paddsw    %%mm1, %%mm7 \n\t"	/* add MM1 to accumulator MM7 */
/* --- 6 */
	  "movq    (%%esi), %%mm1 \n\t"	/* load 8 bytes of the Src */
	  "movq      %%mm1, %%mm2 \n\t"	/* copy MM1 into MM2 */
	  "add       %%eax, %%esi \n\t"	/* move Src pointer 1 row below */
	  "movq    (%%edx), %%mm3 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	  "movq    (%%edx), %%mm4 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	   "punpcklbw %%mm0, %%mm1 \n\t"	/* unpack first  4 bytes into words */
	  "punpckhbw %%mm0, %%mm2 \n\t"	/* unpack second 4 bytes into words */
	   "psrlw     %%mm5, %%mm1 \n\t"	/* shift right each pixel NshiftRight times */
	  "psrlw     %%mm5, %%mm2 \n\t"	/* shift right each pixel NshiftRight times */
	   "pmullw    %%mm3, %%mm1 \n\t"	/* mult. 4 low  words of Src and Kernel */
	  "pmullw    %%mm4, %%mm2 \n\t"	/* mult. 4 high words of Src and Kernel */
	   "paddsw    %%mm2, %%mm1 \n\t"	/* add 4 words of the high and low bytes */
	  "paddsw    %%mm1, %%mm7 \n\t"	/* add MM1 to accumulator MM7 */
/* --- 7 */
	  "movq    (%%esi), %%mm1 \n\t"	/* load 8 bytes of the Src */
	  "movq      %%mm1, %%mm2 \n\t"	/* copy MM1 into MM2 */
	  "movq    (%%edx), %%mm3 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	  "movq    (%%edx), %%mm4 \n\t"	/* load 4 words of Kernel */
	   "punpcklbw %%mm0, %%mm1 \n\t"	/* unpack first  4 bytes into words */
	  "punpckhbw %%mm0, %%mm2 \n\t"	/* unpack second 4 bytes into words */
	   "psrlw     %%mm5, %%mm1 \n\t"	/* shift right each pixel NshiftRight times */
	  "psrlw     %%mm5, %%mm2 \n\t"	/* shift right each pixel NshiftRight times */
	   "pmullw    %%mm3, %%mm1 \n\t"	/* mult. 4 low  words of Src and Kernel */
	  "pmullw    %%mm4, %%mm2 \n\t"	/* mult. 4 high words of Src and Kernel */
	   "paddsw    %%mm2, %%mm1 \n\t"	/* add 4 words of the high and low bytes */
	  "paddsw    %%mm1, %%mm7 \n\t"	/* add MM1 to accumulator MM7 */
/* --- */
	  "movq      %%mm7, %%mm3 \n\t"	/* copy MM7 into MM3 */
	  "psrlq       $32, %%mm7 \n\t"	/* shift 2 left words to the right */
	  "paddsw    %%mm3, %%mm7 \n\t"	/* add 2 left and 2 right result words */
	  "movq      %%mm7, %%mm2 \n\t"	/* copy MM7 into MM2 */
	  "psrlq       $16, %%mm7 \n\t"	/* shift 1 left word to the right */
	  "paddsw    %%mm2, %%mm7 \n\t"	/* add 1 left and 1 right result words */
	  "movd      %%eax, %%mm1 \n\t"	/* save EAX in MM1 */
	  "packuswb  %%mm0, %%mm7 \n\t"	/* pack division result with saturation */
	  "movd      %%mm7, %%eax \n\t"	/* copy saturated result into EAX */
	  "mov      %%al, (%%edi) \n\t"	/* copy a byte result into Dest */
	  "movd      %%mm1, %%eax \n\t"	/* restore saved EAX */
/* -- */
	  "movd      %%mm6, %%esi \n\t"	/* move Src pointer to the top pixel */
	  "sub        $104, %%edx \n\t"	/* EDX = Kernel address */
	  "inc              %%esi \n\t"	/* move Src  pointer to the next pixel */
	  "inc              %%edi \n\t"	/* move Dest pointer to the next pixel */
/* --- */
	  "dec              %%ecx \n\t"	/* decrease loop counter COLUMNS */
	  "jnz            .L10382 \n\t"	/* check loop termination, proceed if required */
	   "add          $6, %%esi \n\t"	/* move to the next row in Src */
	  "add          $6, %%edi \n\t"	/* move to the next row in Dest */
	   "dec              %%ebx \n\t"	/* decrease loop counter ROWS */
	  "jnz            .L10380 \n\t"	/* check loop termination, proceed if required */
/* --- */
	  "emms                   \n\t"	/* exit MMX state */
	  "popa                   \n\t":"=m" (Dest)	/* %0 */
	  :"m"(Src),		/* %1 */
	  "m"(rows),		/* %2 */
	  "m"(columns),		/* %3 */
	  "m"(Kernel),		/* %4 */
	  "m"(NRightShift)	/* %5 */
	    );
#endif
	return (0);
    } else {
	/* No non-MMX implementation yet */
	return (-1);
    }
}

/*  SDL_imageFilterConvolveKernel9x9ShiftRight: Dij = saturation0and255( ... ) */
int SDL_imageFilterConvolveKernel9x9ShiftRight(unsigned char *Src, unsigned char *Dest, int rows, int columns,
					       signed short *Kernel, unsigned char NRightShift)
{
    if ((columns < 9) || (rows < 9) || (NRightShift > 7))
	return (-1);

    if ((SDL_imageFilterMMXdetect())) {
#ifdef USE_MMX
	asm volatile
	 ("pusha		     \n\t" "pxor      %%mm0, %%mm0 \n\t"	/* zero MM0 */
	   "xor       %%ebx, %%ebx \n\t"	/* zero EBX */
	  "mov           %5, %%bl \n\t"	/* load NRightShift into BL */
	  "movd      %%ebx, %%mm5 \n\t"	/* copy NRightShift into MM5 */
	   "mov          %4, %%edx \n\t"	/* load Kernel address into EDX */
	  "mov          %1, %%esi \n\t"	/* load Src  address to ESI */
	  "mov          %0, %%edi \n\t"	/* load Dest address to EDI */
	  "add          $4, %%edi \n\t"	/* 4 column offset from the left edge */
	  "mov          %3, %%eax \n\t"	/* load columns into EAX */
	  "add       %%eax, %%edi \n\t"	/* 4 row offset from the top edge */
	  "add       %%eax, %%edi \n\t" "add       %%eax, %%edi \n\t" "add       %%eax, %%edi \n\t" "mov          %2, %%ebx \n\t"	/* initialize ROWS counter */
	  "sub          $8, %%ebx \n\t"	/* do not use first 4 and last 4 rows */
/* --- */
	  ".L10390:               \n\t" "mov       %%eax, %%ecx \n\t"	/* initialize COLUMNS counter */
	  "sub          $8, %%ecx \n\t"	/* do not use first 4 and last 4 columns */
	   ".align 16              \n\t"	/* 16 byte allignment of the loop entry */
	  ".L10392:               \n\t" "pxor      %%mm7, %%mm7 \n\t"	/* zero MM7 (accumulator) */
	  "movd      %%esi, %%mm6 \n\t"	/* save ESI in MM6 */
/* --- 1 */
	  "movq    (%%esi), %%mm1 \n\t"	/* load 8 bytes of the Src */
	  "movq      %%mm1, %%mm2 \n\t"	/* copy MM1 into MM2 */
	  "inc              %%esi \n\t"	/* move pointer to the next 8 bytes of Src */
	  "movq    (%%edx), %%mm3 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	  "movq    (%%edx), %%mm4 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	   "punpcklbw %%mm0, %%mm1 \n\t"	/* unpack first  4 bytes into words */
	  "punpckhbw %%mm0, %%mm2 \n\t"	/* unpack second 4 bytes into words */
	   "psrlw     %%mm5, %%mm1 \n\t"	/* shift right each pixel NshiftRight times */
	  "psrlw     %%mm5, %%mm2 \n\t"	/* shift right each pixel NshiftRight times */
	   "pmullw    %%mm3, %%mm1 \n\t"	/* mult. 4 low  words of Src and Kernel */
	  "pmullw    %%mm4, %%mm2 \n\t"	/* mult. 4 high words of Src and Kernel */
	   "paddsw    %%mm2, %%mm1 \n\t"	/* add 4 words of the high and low bytes */
	  "paddsw    %%mm1, %%mm7 \n\t"	/* add MM1 to accumulator MM7 */
	   "movq    (%%esi), %%mm1 \n\t"	/* load 8 bytes of the Src */
	  "dec              %%esi \n\t" "add       %%eax, %%esi \n\t"	/* move Src pointer 1 row below */
	  "movq    (%%edx), %%mm3 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	   "punpcklbw %%mm0, %%mm1 \n\t"	/* unpack first  4 bytes into words */
	  "psrlw     %%mm5, %%mm1 \n\t"	/* shift right each pixel NshiftRight times */
	  "pmullw    %%mm3, %%mm1 \n\t"	/* mult. 4 low  words of Src and Kernel */
	  "paddsw    %%mm1, %%mm7 \n\t"	/* add MM1 to accumulator MM7 */
/* --- 2 */
	  "movq    (%%esi), %%mm1 \n\t"	/* load 8 bytes of the Src */
	  "movq      %%mm1, %%mm2 \n\t"	/* copy MM1 into MM2 */
	  "inc              %%esi \n\t"	/* move pointer to the next 8 bytes of Src */
	  "movq    (%%edx), %%mm3 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	  "movq    (%%edx), %%mm4 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	   "punpcklbw %%mm0, %%mm1 \n\t"	/* unpack first  4 bytes into words */
	  "punpckhbw %%mm0, %%mm2 \n\t"	/* unpack second 4 bytes into words */
	   "psrlw     %%mm5, %%mm1 \n\t"	/* shift right each pixel NshiftRight times */
	  "psrlw     %%mm5, %%mm2 \n\t"	/* shift right each pixel NshiftRight times */
	   "pmullw    %%mm3, %%mm1 \n\t"	/* mult. 4 low  words of Src and Kernel */
	  "pmullw    %%mm4, %%mm2 \n\t"	/* mult. 4 high words of Src and Kernel */
	   "paddsw    %%mm2, %%mm1 \n\t"	/* add 4 words of the high and low bytes */
	  "paddsw    %%mm1, %%mm7 \n\t"	/* add MM1 to accumulator MM7 */
	   "movq    (%%esi), %%mm1 \n\t"	/* load 8 bytes of the Src */
	  "dec              %%esi \n\t" "add       %%eax, %%esi \n\t"	/* move Src pointer 1 row below */
	  "movq    (%%edx), %%mm3 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	   "punpcklbw %%mm0, %%mm1 \n\t"	/* unpack first  4 bytes into words */
	  "psrlw     %%mm5, %%mm1 \n\t"	/* shift right each pixel NshiftRight times */
	  "pmullw    %%mm3, %%mm1 \n\t"	/* mult. 4 low  words of Src and Kernel */
	  "paddsw    %%mm1, %%mm7 \n\t"	/* add MM1 to accumulator MM7 */
/* --- 3 */
	  "movq    (%%esi), %%mm1 \n\t"	/* load 8 bytes of the Src */
	  "movq      %%mm1, %%mm2 \n\t"	/* copy MM1 into MM2 */
	  "inc              %%esi \n\t"	/* move pointer to the next 8 bytes of Src */
	  "movq    (%%edx), %%mm3 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	  "movq    (%%edx), %%mm4 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	   "punpcklbw %%mm0, %%mm1 \n\t"	/* unpack first  4 bytes into words */
	  "punpckhbw %%mm0, %%mm2 \n\t"	/* unpack second 4 bytes into words */
	   "psrlw     %%mm5, %%mm1 \n\t"	/* shift right each pixel NshiftRight times */
	  "psrlw     %%mm5, %%mm2 \n\t"	/* shift right each pixel NshiftRight times */
	   "pmullw    %%mm3, %%mm1 \n\t"	/* mult. 4 low  words of Src and Kernel */
	  "pmullw    %%mm4, %%mm2 \n\t"	/* mult. 4 high words of Src and Kernel */
	   "paddsw    %%mm2, %%mm1 \n\t"	/* add 4 words of the high and low bytes */
	  "paddsw    %%mm1, %%mm7 \n\t"	/* add MM1 to accumulator MM7 */
	   "movq    (%%esi), %%mm1 \n\t"	/* load 8 bytes of the Src */
	  "dec              %%esi \n\t" "add       %%eax, %%esi \n\t"	/* move Src pointer 1 row below */
	  "movq    (%%edx), %%mm3 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	   "punpcklbw %%mm0, %%mm1 \n\t"	/* unpack first  4 bytes into words */
	  "psrlw     %%mm5, %%mm1 \n\t"	/* shift right each pixel NshiftRight times */
	  "pmullw    %%mm3, %%mm1 \n\t"	/* mult. 4 low  words of Src and Kernel */
	  "paddsw    %%mm1, %%mm7 \n\t"	/* add MM1 to accumulator MM7 */
/* --- 4 */
	  "movq    (%%esi), %%mm1 \n\t"	/* load 8 bytes of the Src */
	  "movq      %%mm1, %%mm2 \n\t"	/* copy MM1 into MM2 */
	  "inc              %%esi \n\t"	/* move pointer to the next 8 bytes of Src */
	  "movq    (%%edx), %%mm3 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	  "movq    (%%edx), %%mm4 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	   "punpcklbw %%mm0, %%mm1 \n\t"	/* unpack first  4 bytes into words */
	  "punpckhbw %%mm0, %%mm2 \n\t"	/* unpack second 4 bytes into words */
	   "psrlw     %%mm5, %%mm1 \n\t"	/* shift right each pixel NshiftRight times */
	  "psrlw     %%mm5, %%mm2 \n\t"	/* shift right each pixel NshiftRight times */
	   "pmullw    %%mm3, %%mm1 \n\t"	/* mult. 4 low  words of Src and Kernel */
	  "pmullw    %%mm4, %%mm2 \n\t"	/* mult. 4 high words of Src and Kernel */
	   "paddsw    %%mm2, %%mm1 \n\t"	/* add 4 words of the high and low bytes */
	  "paddsw    %%mm1, %%mm7 \n\t"	/* add MM1 to accumulator MM7 */
	   "movq    (%%esi), %%mm1 \n\t"	/* load 8 bytes of the Src */
	  "dec              %%esi \n\t" "add       %%eax, %%esi \n\t"	/* move Src pointer 1 row below */
	  "movq    (%%edx), %%mm3 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	   "punpcklbw %%mm0, %%mm1 \n\t"	/* unpack first  4 bytes into words */
	  "psrlw     %%mm5, %%mm1 \n\t"	/* shift right each pixel NshiftRight times */
	  "pmullw    %%mm3, %%mm1 \n\t"	/* mult. 4 low  words of Src and Kernel */
	  "paddsw    %%mm1, %%mm7 \n\t"	/* add MM1 to accumulator MM7 */
/* --- 5 */
	  "movq    (%%esi), %%mm1 \n\t"	/* load 8 bytes of the Src */
	  "movq      %%mm1, %%mm2 \n\t"	/* copy MM1 into MM2 */
	  "inc              %%esi \n\t"	/* move pointer to the next 8 bytes of Src */
	  "movq    (%%edx), %%mm3 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	  "movq    (%%edx), %%mm4 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	   "punpcklbw %%mm0, %%mm1 \n\t"	/* unpack first  4 bytes into words */
	  "punpckhbw %%mm0, %%mm2 \n\t"	/* unpack second 4 bytes into words */
	   "psrlw     %%mm5, %%mm1 \n\t"	/* shift right each pixel NshiftRight times */
	  "psrlw     %%mm5, %%mm2 \n\t"	/* shift right each pixel NshiftRight times */
	   "pmullw    %%mm3, %%mm1 \n\t"	/* mult. 4 low  words of Src and Kernel */
	  "pmullw    %%mm4, %%mm2 \n\t"	/* mult. 4 high words of Src and Kernel */
	   "paddsw    %%mm2, %%mm1 \n\t"	/* add 4 words of the high and low bytes */
	  "paddsw    %%mm1, %%mm7 \n\t"	/* add MM1 to accumulator MM7 */
	   "movq    (%%esi), %%mm1 \n\t"	/* load 8 bytes of the Src */
	  "dec              %%esi \n\t" "add       %%eax, %%esi \n\t"	/* move Src pointer 1 row below */
	  "movq    (%%edx), %%mm3 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	   "punpcklbw %%mm0, %%mm1 \n\t"	/* unpack first  4 bytes into words */
	  "psrlw     %%mm5, %%mm1 \n\t"	/* shift right each pixel NshiftRight times */
	  "pmullw    %%mm3, %%mm1 \n\t"	/* mult. 4 low  words of Src and Kernel */
	  "paddsw    %%mm1, %%mm7 \n\t"	/* add MM1 to accumulator MM7 */
/* --- 6 */
	  "movq    (%%esi), %%mm1 \n\t"	/* load 8 bytes of the Src */
	  "movq      %%mm1, %%mm2 \n\t"	/* copy MM1 into MM2 */
	  "inc              %%esi \n\t"	/* move pointer to the next 8 bytes of Src */
	  "movq    (%%edx), %%mm3 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	  "movq    (%%edx), %%mm4 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	   "punpcklbw %%mm0, %%mm1 \n\t"	/* unpack first  4 bytes into words */
	  "punpckhbw %%mm0, %%mm2 \n\t"	/* unpack second 4 bytes into words */
	   "psrlw     %%mm5, %%mm1 \n\t"	/* shift right each pixel NshiftRight times */
	  "psrlw     %%mm5, %%mm2 \n\t"	/* shift right each pixel NshiftRight times */
	   "pmullw    %%mm3, %%mm1 \n\t"	/* mult. 4 low  words of Src and Kernel */
	  "pmullw    %%mm4, %%mm2 \n\t"	/* mult. 4 high words of Src and Kernel */
	   "paddsw    %%mm2, %%mm1 \n\t"	/* add 4 words of the high and low bytes */
	  "paddsw    %%mm1, %%mm7 \n\t"	/* add MM1 to accumulator MM7 */
	   "movq    (%%esi), %%mm1 \n\t"	/* load 8 bytes of the Src */
	  "dec              %%esi \n\t" "add       %%eax, %%esi \n\t"	/* move Src pointer 1 row below */
	  "movq    (%%edx), %%mm3 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	   "punpcklbw %%mm0, %%mm1 \n\t"	/* unpack first  4 bytes into words */
	  "psrlw     %%mm5, %%mm1 \n\t"	/* shift right each pixel NshiftRight times */
	  "pmullw    %%mm3, %%mm1 \n\t"	/* mult. 4 low  words of Src and Kernel */
	  "paddsw    %%mm1, %%mm7 \n\t"	/* add MM1 to accumulator MM7 */
/* --- 7 */
	  "movq    (%%esi), %%mm1 \n\t"	/* load 8 bytes of the Src */
	  "movq      %%mm1, %%mm2 \n\t"	/* copy MM1 into MM2 */
	  "inc              %%esi \n\t"	/* move pointer to the next 8 bytes of Src */
	  "movq    (%%edx), %%mm3 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	  "movq    (%%edx), %%mm4 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	   "punpcklbw %%mm0, %%mm1 \n\t"	/* unpack first  4 bytes into words */
	  "punpckhbw %%mm0, %%mm2 \n\t"	/* unpack second 4 bytes into words */
	   "psrlw     %%mm5, %%mm1 \n\t"	/* shift right each pixel NshiftRight times */
	  "psrlw     %%mm5, %%mm2 \n\t"	/* shift right each pixel NshiftRight times */
	   "pmullw    %%mm3, %%mm1 \n\t"	/* mult. 4 low  words of Src and Kernel */
	  "pmullw    %%mm4, %%mm2 \n\t"	/* mult. 4 high words of Src and Kernel */
	   "paddsw    %%mm2, %%mm1 \n\t"	/* add 4 words of the high and low bytes */
	  "paddsw    %%mm1, %%mm7 \n\t"	/* add MM1 to accumulator MM7 */
	   "movq    (%%esi), %%mm1 \n\t"	/* load 8 bytes of the Src */
	  "dec              %%esi \n\t" "add       %%eax, %%esi \n\t"	/* move Src pointer 1 row below */
	  "movq    (%%edx), %%mm3 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	   "punpcklbw %%mm0, %%mm1 \n\t"	/* unpack first  4 bytes into words */
	  "psrlw     %%mm5, %%mm1 \n\t"	/* shift right each pixel NshiftRight times */
	  "pmullw    %%mm3, %%mm1 \n\t"	/* mult. 4 low  words of Src and Kernel */
	  "paddsw    %%mm1, %%mm7 \n\t"	/* add MM1 to accumulator MM7 */
/* --- 8 */
	  "movq    (%%esi), %%mm1 \n\t"	/* load 8 bytes of the Src */
	  "movq      %%mm1, %%mm2 \n\t"	/* copy MM1 into MM2 */
	  "inc              %%esi \n\t"	/* move pointer to the next 8 bytes of Src */
	  "movq    (%%edx), %%mm3 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	  "movq    (%%edx), %%mm4 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	   "punpcklbw %%mm0, %%mm1 \n\t"	/* unpack first  4 bytes into words */
	  "punpckhbw %%mm0, %%mm2 \n\t"	/* unpack second 4 bytes into words */
	   "psrlw     %%mm5, %%mm1 \n\t"	/* shift right each pixel NshiftRight times */
	  "psrlw     %%mm5, %%mm2 \n\t"	/* shift right each pixel NshiftRight times */
	   "pmullw    %%mm3, %%mm1 \n\t"	/* mult. 4 low  words of Src and Kernel */
	  "pmullw    %%mm4, %%mm2 \n\t"	/* mult. 4 high words of Src and Kernel */
	   "paddsw    %%mm2, %%mm1 \n\t"	/* add 4 words of the high and low bytes */
	  "paddsw    %%mm1, %%mm7 \n\t"	/* add MM1 to accumulator MM7 */
	   "movq    (%%esi), %%mm1 \n\t"	/* load 8 bytes of the Src */
	  "dec              %%esi \n\t" "add       %%eax, %%esi \n\t"	/* move Src pointer 1 row below */
	  "movq    (%%edx), %%mm3 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	   "punpcklbw %%mm0, %%mm1 \n\t"	/* unpack first  4 bytes into words */
	  "psrlw     %%mm5, %%mm1 \n\t"	/* shift right each pixel NshiftRight times */
	  "pmullw    %%mm3, %%mm1 \n\t"	/* mult. 4 low  words of Src and Kernel */
	  "paddsw    %%mm1, %%mm7 \n\t"	/* add MM1 to accumulator MM7 */
/* --- 9 */
	  "movq    (%%esi), %%mm1 \n\t"	/* load 8 bytes of the Src */
	  "movq      %%mm1, %%mm2 \n\t"	/* copy MM1 into MM2 */
	  "inc              %%esi \n\t"	/* move pointer to the next 8 bytes of Src */
	  "movq    (%%edx), %%mm3 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	  "movq    (%%edx), %%mm4 \n\t"	/* load 4 words of Kernel */
	  "add          $8, %%edx \n\t"	/* move pointer to other 4 words */
	   "punpcklbw %%mm0, %%mm1 \n\t"	/* unpack first  4 bytes into words */
	  "punpckhbw %%mm0, %%mm2 \n\t"	/* unpack second 4 bytes into words */
	   "psrlw     %%mm5, %%mm1 \n\t"	/* shift right each pixel NshiftRight times */
	  "psrlw     %%mm5, %%mm2 \n\t"	/* shift right each pixel NshiftRight times */
	   "pmullw    %%mm3, %%mm1 \n\t"	/* mult. 4 low  words of Src and Kernel */
	  "pmullw    %%mm4, %%mm2 \n\t"	/* mult. 4 high words of Src and Kernel */
	   "paddsw    %%mm2, %%mm1 \n\t"	/* add 4 words of the high and low bytes */
	  "paddsw    %%mm1, %%mm7 \n\t"	/* add MM1 to accumulator MM7 */
	   "movq    (%%esi), %%mm1 \n\t"	/* load 8 bytes of the Src */
	  "movq    (%%edx), %%mm3 \n\t"	/* load 4 words of Kernel */
	   "punpcklbw %%mm0, %%mm1 \n\t"	/* unpack first  4 bytes into words */
	  "psrlw     %%mm5, %%mm1 \n\t"	/* shift right each pixel NshiftRight times */
	  "pmullw    %%mm3, %%mm1 \n\t"	/* mult. 4 low  words of Src and Kernel */
	  "paddsw    %%mm1, %%mm7 \n\t"	/* add MM1 to accumulator MM7 */
/* --- */
	  "movq      %%mm7, %%mm3 \n\t"	/* copy MM7 into MM3 */
	  "psrlq       $32, %%mm7 \n\t"	/* shift 2 left words to the right */
	  "paddsw    %%mm3, %%mm7 \n\t"	/* add 2 left and 2 right result words */
	  "movq      %%mm7, %%mm2 \n\t"	/* copy MM7 into MM2 */
	  "psrlq       $16, %%mm7 \n\t"	/* shift 1 left word to the right */
	  "paddsw    %%mm2, %%mm7 \n\t"	/* add 1 left and 1 right result words */
	  "movd      %%eax, %%mm1 \n\t"	/* save EAX in MM1 */
	  "packuswb  %%mm0, %%mm7 \n\t"	/* pack division result with saturation */
	  "movd      %%mm7, %%eax \n\t"	/* copy saturated result into EAX */
	  "mov      %%al, (%%edi) \n\t"	/* copy a byte result into Dest */
	  "movd      %%mm1, %%eax \n\t"	/* restore saved EAX */
/* -- */
	  "movd      %%mm6, %%esi \n\t"	/* move Src pointer to the top pixel */
	  "sub        $208, %%edx \n\t"	/* EDX = Kernel address */
	  "inc              %%esi \n\t"	/* move Src  pointer to the next pixel */
	  "inc              %%edi \n\t"	/* move Dest pointer to the next pixel */
/* --- */
	  "dec              %%ecx \n\t"	/* decrease loop counter COLUMNS */
	  "jnz            .L10392 \n\t"	/* check loop termination, proceed if required */
	   "add          $8, %%esi \n\t"	/* move to the next row in Src */
	  "add          $8, %%edi \n\t"	/* move to the next row in Dest */
	   "dec              %%ebx \n\t"	/* decrease loop counter ROWS */
	  "jnz            .L10390 \n\t"	/* check loop termination, proceed if required */
/* --- */
	  "emms                   \n\t"	/* exit MMX state */
	  "popa                   \n\t":"=m" (Dest)	/* %0 */
	  :"m"(Src),		/* %1 */
	  "m"(rows),		/* %2 */
	  "m"(columns),		/* %3 */
	  "m"(Kernel),		/* %4 */
	  "m"(NRightShift)	/* %5 */
	    );
#endif
	return (0);
    } else {
	/* No non-MMX implementation yet */
	return (-1);
    }
}

/* ------------------------------------------------------------------------------------ */

/*  SDL_imageFilterSobelX: Dij = saturation255( ... ) */
int SDL_imageFilterSobelX(unsigned char *Src, unsigned char *Dest, int rows, int columns)
{
    if ((columns < 8) || (rows < 3))
	return (-1);

    if ((SDL_imageFilterMMXdetect())) {
#ifdef USE_MMX
	asm volatile
	 ("pusha		     \n\t" "pxor      %%mm0, %%mm0 \n\t"	/* zero MM0 */
	  "mov          %3, %%eax \n\t"	/* load columns into EAX */
/* --- */
	  "mov          %1, %%esi \n\t"	/* ESI = Src row 0 address */
	  "mov          %0, %%edi \n\t"	/* load Dest address to EDI */
	  "add       %%eax, %%edi \n\t"	/* EDI = EDI + columns */
	  "inc              %%edi \n\t"	/* 1 byte offset from the left edge */
	   "mov          %2, %%edx \n\t"	/* initialize ROWS counter */
	  "sub          $2, %%edx \n\t"	/* do not use first and last rows */
/* --- */
	  ".L10400:                \n\t" "mov       %%eax, %%ecx \n\t"	/* initialize COLUMS counter */
	  "shr          $3, %%ecx \n\t"	/* EBX/8 (MMX loads 8 bytes at a time) */
	   "mov       %%esi, %%ebx \n\t"	/* save ESI in EBX */
	  "movd      %%edi, %%mm1 \n\t"	/* save EDI in MM1 */
	   ".align 16              \n\t"	/* 16 byte allignment of the loop entry */
	  ".L10402:               \n\t"
/* --- */
	  "movq    (%%esi), %%mm4 \n\t"	/* load 8 bytes from Src */
	  "movq      %%mm4, %%mm5 \n\t"	/* save MM4 in MM5 */
	  "add          $2, %%esi \n\t"	/* move ESI pointer 2 bytes right */
	  "punpcklbw %%mm0, %%mm4 \n\t"	/* unpack 4 low  bytes into words */
	  "punpckhbw %%mm0, %%mm5 \n\t"	/* unpack 4 high bytes into words */
	   "movq    (%%esi), %%mm6 \n\t"	/* load 8 bytes from Src */
	  "movq      %%mm6, %%mm7 \n\t"	/* save MM6 in MM7 */
	  "sub          $2, %%esi \n\t"	/* move ESI pointer back 2 bytes left */
	   "punpcklbw %%mm0, %%mm6 \n\t"	/* unpack 4 low  bytes into words */
	  "punpckhbw %%mm0, %%mm7 \n\t"	/* unpack 4 high bytes into words */
	   "add       %%eax, %%esi \n\t"	/* move to the next row of Src */
	   "movq    (%%esi), %%mm2 \n\t"	/* load 8 bytes from Src */
	  "movq      %%mm2, %%mm3 \n\t"	/* save MM2 in MM3 */
	  "add          $2, %%esi \n\t"	/* move ESI pointer 2 bytes right */
	  "punpcklbw %%mm0, %%mm2 \n\t"	/* unpack 4 low  bytes into words */
	  "punpckhbw %%mm0, %%mm3 \n\t"	/* unpack 4 high bytes into words */
	   "paddw     %%mm2, %%mm4 \n\t"	/* add 4 low  bytes to accumolator MM4 */
	  "paddw     %%mm3, %%mm5 \n\t"	/* add 4 high bytes to accumolator MM5 */
	  "paddw     %%mm2, %%mm4 \n\t"	/* add 4 low  bytes to accumolator MM4 */
	  "paddw     %%mm3, %%mm5 \n\t"	/* add 4 high bytes to accumolator MM5 */
	   "movq    (%%esi), %%mm2 \n\t"	/* load 8 bytes from Src */
	  "movq      %%mm2, %%mm3 \n\t"	/* save MM2 in MM3 */
	  "sub          $2, %%esi \n\t"	/* move ESI pointer back 2 bytes left */
	  "punpcklbw %%mm0, %%mm2 \n\t"	/* unpack 4 low  bytes into words */
	  "punpckhbw %%mm0, %%mm3 \n\t"	/* unpack 4 high bytes into words */
	   "paddw     %%mm2, %%mm6 \n\t"	/* add 4 low  bytes to accumolator MM6 */
	  "paddw     %%mm3, %%mm7 \n\t"	/* add 4 high bytes to accumolator MM7 */
	  "paddw     %%mm2, %%mm6 \n\t"	/* add 4 low  bytes to accumolator MM6 */
	  "paddw     %%mm3, %%mm7 \n\t"	/* add 4 high bytes to accumolator MM7 */
	   "add       %%eax, %%esi \n\t"	/* move to the next row of Src */
	   "movq    (%%esi), %%mm2 \n\t"	/* load 8 bytes from Src */
	  "movq      %%mm2, %%mm3 \n\t"	/* save MM2 in MM3 */
	  "add          $2, %%esi \n\t"	/* move ESI pointer 2 bytes right */
	  "punpcklbw %%mm0, %%mm2 \n\t"	/* unpack 4 low  bytes into words */
	  "punpckhbw %%mm0, %%mm3 \n\t"	/* unpack 4 high bytes into words */
	   "paddw     %%mm2, %%mm4 \n\t"	/* add 4 low  bytes to accumolator MM4 */
	  "paddw     %%mm3, %%mm5 \n\t"	/* add 4 high bytes to accumolator MM5 */
	   "movq    (%%esi), %%mm2 \n\t"	/* load 8 bytes from Src */
	  "movq      %%mm2, %%mm3 \n\t"	/* save MM2 in MM3 */
	  "sub          $2, %%esi \n\t"	/* move ESI pointer back 2 bytes left */
	  "punpcklbw %%mm0, %%mm2 \n\t"	/* unpack 4 low  bytes into words */
	  "punpckhbw %%mm0, %%mm3 \n\t"	/* unpack 4 high bytes into words */
	   "paddw     %%mm2, %%mm6 \n\t"	/* add 4 low  bytes to accumolator MM6 */
	  "paddw     %%mm3, %%mm7 \n\t"	/* add 4 high bytes to accumolator MM7 */
/* --- */
	  "movq      %%mm4, %%mm2 \n\t"	/* copy MM4 into MM2 */
	  "psrlq       $32, %%mm4 \n\t"	/* shift 2 left words to the right */
	  "psubw     %%mm2, %%mm4 \n\t"	/* MM4 = MM4 - MM2 */
	  "movq      %%mm6, %%mm3 \n\t"	/* copy MM6 into MM3 */
	  "psrlq       $32, %%mm6 \n\t"	/* shift 2 left words to the right */
	  "psubw     %%mm3, %%mm6 \n\t"	/* MM6 = MM6 - MM3 */
	  "punpckldq %%mm6, %%mm4 \n\t"	/* combine 2 words of MM6 and 2 words of MM4 */
	   "movq      %%mm5, %%mm2 \n\t"	/* copy MM6 into MM2 */
	  "psrlq       $32, %%mm5 \n\t"	/* shift 2 left words to the right */
	  "psubw     %%mm2, %%mm5 \n\t"	/* MM5 = MM5 - MM2 */
	  "movq      %%mm7, %%mm3 \n\t"	/* copy MM7 into MM3 */
	  "psrlq       $32, %%mm7 \n\t"	/* shift 2 left words to the right */
	  "psubw     %%mm3, %%mm7 \n\t"	/* MM7 = MM7 - MM3 */
	  "punpckldq %%mm7, %%mm5 \n\t"	/* combine 2 words of MM7 and 2 words of MM5 */
	  /* Take abs values of MM4 and MM5 */
	  "movq      %%mm4, %%mm6 \n\t"	/* copy MM4 into MM6 */
	  "movq      %%mm5, %%mm7 \n\t"	/* copy MM5 into MM7 */
	  "psraw       $15, %%mm6 \n\t"	/* fill MM6 words with word sign bit */
	  "psraw       $15, %%mm7 \n\t"	/* fill MM7 words with word sign bit */
	  "pxor      %%mm6, %%mm4 \n\t"	/* take 1's compliment of only neg. words */
	  "pxor      %%mm7, %%mm5 \n\t"	/* take 1's compliment of only neg. words */
	  "psubsw    %%mm6, %%mm4 \n\t"	/* add 1 to only neg. words, W-(-1) or W-0 */
	  "psubsw    %%mm7, %%mm5 \n\t"	/* add 1 to only neg. words, W-(-1) or W-0 */
	   "packuswb  %%mm5, %%mm4 \n\t"	/* combine and pack/saturate MM5 and MM4 */
	  "movq    %%mm4, (%%edi) \n\t"	/* store result in Dest */
/* --- */
	  "sub       %%eax, %%esi \n\t"	/* move to the current top row in Src */
	  "sub       %%eax, %%esi \n\t" "add $8,          %%esi \n\t"	/* move Src  pointer to the next 8 pixels */
	  "add $8,          %%edi \n\t"	/* move Dest pointer to the next 8 pixels */
/* --- */
	  "dec              %%ecx \n\t"	/* decrease loop counter COLUMNS */
	  "jnz            .L10402 \n\t"	/* check loop termination, proceed if required */
	   "mov       %%ebx, %%esi \n\t"	/* restore most left current row Src  address */
	  "movd      %%mm1, %%edi \n\t"	/* restore most left current row Dest address */
	  "add       %%eax, %%esi \n\t"	/* move to the next row in Src */
	  "add       %%eax, %%edi \n\t"	/* move to the next row in Dest */
	   "dec              %%edx \n\t"	/* decrease loop counter ROWS */
	  "jnz            .L10400 \n\t"	/* check loop termination, proceed if required */
/* --- */
	  "emms                   \n\t"	/* exit MMX state */
	  "popa                   \n\t":"=m" (Dest)	/* %0 */
	  :"m"(Src),		/* %1 */
	  "m"(rows),		/* %2 */
	  "m"(columns)		/* %3 */
	    );
#endif
	return (0);
    } else {
	/* No non-MMX implementation yet */
	return (-1);
    }
}

/*  SDL_imageFilterSobelXShiftRight: Dij = saturation255( ... ) */
int SDL_imageFilterSobelXShiftRight(unsigned char *Src, unsigned char *Dest, int rows, int columns,
				    unsigned char NRightShift)
{
    if ((columns < 8) || (rows < 3) || (NRightShift > 7))
	return (-1);

    if ((SDL_imageFilterMMXdetect())) {
#ifdef USE_MMX
	asm volatile
	 ("pusha		     \n\t" "pxor      %%mm0, %%mm0 \n\t"	/* zero MM0 */
	  "mov          %3, %%eax \n\t"	/* load columns into EAX */
	   "xor       %%ebx, %%ebx \n\t"	/* zero EBX */
	  "mov           %4, %%bl \n\t"	/* load NRightShift into BL */
	  "movd      %%ebx, %%mm1 \n\t"	/* copy NRightShift into MM1 */
/* --- */
	  "mov          %1, %%esi \n\t"	/* ESI = Src row 0 address */
	  "mov          %0, %%edi \n\t"	/* load Dest address to EDI */
	  "add       %%eax, %%edi \n\t"	/* EDI = EDI + columns */
	  "inc              %%edi \n\t"	/* 1 byte offset from the left edge */
	  /* initialize ROWS counter */
	  "subl            $2, %2 \n\t"	/* do not use first and last rows */
/* --- */
	  ".L10410:                \n\t" "mov       %%eax, %%ecx \n\t"	/* initialize COLUMS counter */
	  "shr          $3, %%ecx \n\t"	/* EBX/8 (MMX loads 8 bytes at a time) */
	   "mov       %%esi, %%ebx \n\t"	/* save ESI in EBX */
	  "mov       %%edi, %%edx \n\t"	/* save EDI in EDX */
	   ".align 16              \n\t"	/* 16 byte allignment of the loop entry */
	  ".L10412:               \n\t"
/* --- */
	  "movq    (%%esi), %%mm4 \n\t"	/* load 8 bytes from Src */
	  "movq      %%mm4, %%mm5 \n\t"	/* save MM4 in MM5 */
	  "add          $2, %%esi \n\t"	/* move ESI pointer 2 bytes right */
	  "punpcklbw %%mm0, %%mm4 \n\t"	/* unpack 4 low  bytes into words */
	  "punpckhbw %%mm0, %%mm5 \n\t"	/* unpack 4 high bytes into words */
	   "psrlw     %%mm1, %%mm4 \n\t"	/* shift right each pixel NshiftRight times */
	  "psrlw     %%mm1, %%mm5 \n\t"	/* shift right each pixel NshiftRight times */
	   "movq    (%%esi), %%mm6 \n\t"	/* load 8 bytes from Src */
	  "movq      %%mm6, %%mm7 \n\t"	/* save MM6 in MM7 */
	  "sub          $2, %%esi \n\t"	/* move ESI pointer back 2 bytes left */
	   "punpcklbw %%mm0, %%mm6 \n\t"	/* unpack 4 low  bytes into words */
	  "punpckhbw %%mm0, %%mm7 \n\t"	/* unpack 4 high bytes into words */
	   "psrlw     %%mm1, %%mm6 \n\t"	/* shift right each pixel NshiftRight times */
	  "psrlw     %%mm1, %%mm7 \n\t"	/* shift right each pixel NshiftRight times */
	   "add       %%eax, %%esi \n\t"	/* move to the next row of Src */
	   "movq    (%%esi), %%mm2 \n\t"	/* load 8 bytes from Src */
	  "movq      %%mm2, %%mm3 \n\t"	/* save MM2 in MM3 */
	  "add          $2, %%esi \n\t"	/* move ESI pointer 2 bytes right */
	  "punpcklbw %%mm0, %%mm2 \n\t"	/* unpack 4 low  bytes into words */
	  "punpckhbw %%mm0, %%mm3 \n\t"	/* unpack 4 high bytes into words */
	   "psrlw     %%mm1, %%mm2 \n\t"	/* shift right each pixel NshiftRight times */
	  "psrlw     %%mm1, %%mm3 \n\t"	/* shift right each pixel NshiftRight times */
	   "paddw     %%mm2, %%mm4 \n\t"	/* add 4 low  bytes to accumolator MM4 */
	  "paddw     %%mm3, %%mm5 \n\t"	/* add 4 high bytes to accumolator MM5 */
	  "paddw     %%mm2, %%mm4 \n\t"	/* add 4 low  bytes to accumolator MM4 */
	  "paddw     %%mm3, %%mm5 \n\t"	/* add 4 high bytes to accumolator MM5 */
	   "movq    (%%esi), %%mm2 \n\t"	/* load 8 bytes from Src */
	  "movq      %%mm2, %%mm3 \n\t"	/* save MM2 in MM3 */
	  "sub          $2, %%esi \n\t"	/* move ESI pointer back 2 bytes left */
	  "punpcklbw %%mm0, %%mm2 \n\t"	/* unpack 4 low  bytes into words */
	  "punpckhbw %%mm0, %%mm3 \n\t"	/* unpack 4 high bytes into words */
	   "psrlw     %%mm1, %%mm2 \n\t"	/* shift right each pixel NshiftRight times */
	  "psrlw     %%mm1, %%mm3 \n\t"	/* shift right each pixel NshiftRight times */
	   "paddw     %%mm2, %%mm6 \n\t"	/* add 4 low  bytes to accumolator MM6 */
	  "paddw     %%mm3, %%mm7 \n\t"	/* add 4 high bytes to accumolator MM7 */
	  "paddw     %%mm2, %%mm6 \n\t"	/* add 4 low  bytes to accumolator MM6 */
	  "paddw     %%mm3, %%mm7 \n\t"	/* add 4 high bytes to accumolator MM7 */
	   "add       %%eax, %%esi \n\t"	/* move to the next row of Src */
	   "movq    (%%esi), %%mm2 \n\t"	/* load 8 bytes from Src */
	  "movq      %%mm2, %%mm3 \n\t"	/* save MM2 in MM3 */
	  "add          $2, %%esi \n\t"	/* move ESI pointer 2 bytes right */
	  "punpcklbw %%mm0, %%mm2 \n\t"	/* unpack 4 low  bytes into words */
	  "punpckhbw %%mm0, %%mm3 \n\t"	/* unpack 4 high bytes into words */
	   "psrlw     %%mm1, %%mm2 \n\t"	/* shift right each pixel NshiftRight times */
	  "psrlw     %%mm1, %%mm3 \n\t"	/* shift right each pixel NshiftRight times */
	   "paddw     %%mm2, %%mm4 \n\t"	/* add 4 low  bytes to accumolator MM4 */
	  "paddw     %%mm3, %%mm5 \n\t"	/* add 4 high bytes to accumolator MM5 */
	   "movq    (%%esi), %%mm2 \n\t"	/* load 8 bytes from Src */
	  "movq      %%mm2, %%mm3 \n\t"	/* save MM2 in MM3 */
	  "sub          $2, %%esi \n\t"	/* move ESI pointer back 2 bytes left */
	  "punpcklbw %%mm0, %%mm2 \n\t"	/* unpack 4 low  bytes into words */
	  "punpckhbw %%mm0, %%mm3 \n\t"	/* unpack 4 high bytes into words */
	   "psrlw     %%mm1, %%mm2 \n\t"	/* shift right each pixel NshiftRight times */
	  "psrlw     %%mm1, %%mm3 \n\t"	/* shift right each pixel NshiftRight times */
	   "paddw     %%mm2, %%mm6 \n\t"	/* add 4 low  bytes to accumolator MM6 */
	  "paddw     %%mm3, %%mm7 \n\t"	/* add 4 high bytes to accumolator MM7 */
/* --- */
	  "movq      %%mm4, %%mm2 \n\t"	/* copy MM4 into MM2 */
	  "psrlq       $32, %%mm4 \n\t"	/* shift 2 left words to the right */
	  "psubw     %%mm2, %%mm4 \n\t"	/* MM4 = MM4 - MM2 */
	  "movq      %%mm6, %%mm3 \n\t"	/* copy MM6 into MM3 */
	  "psrlq       $32, %%mm6 \n\t"	/* shift 2 left words to the right */
	  "psubw     %%mm3, %%mm6 \n\t"	/* MM6 = MM6 - MM3 */
	  "punpckldq %%mm6, %%mm4 \n\t"	/* combine 2 words of MM6 and 2 words of MM4 */
	   "movq      %%mm5, %%mm2 \n\t"	/* copy MM6 into MM2 */
	  "psrlq       $32, %%mm5 \n\t"	/* shift 2 left words to the right */
	  "psubw     %%mm2, %%mm5 \n\t"	/* MM5 = MM5 - MM2 */
	  "movq      %%mm7, %%mm3 \n\t"	/* copy MM7 into MM3 */
	  "psrlq       $32, %%mm7 \n\t"	/* shift 2 left words to the right */
	  "psubw     %%mm3, %%mm7 \n\t"	/* MM7 = MM7 - MM3 */
	  "punpckldq %%mm7, %%mm5 \n\t"	/* combine 2 words of MM7 and 2 words of MM5 */
	  /* Take abs values of MM4 and MM5 */
	  "movq      %%mm4, %%mm6 \n\t"	/* copy MM4 into MM6 */
	  "movq      %%mm5, %%mm7 \n\t"	/* copy MM5 into MM7 */
	  "psraw       $15, %%mm6 \n\t"	/* fill MM6 words with word sign bit */
	  "psraw       $15, %%mm7 \n\t"	/* fill MM7 words with word sign bit */
	  "pxor      %%mm6, %%mm4 \n\t"	/* take 1's compliment of only neg. words */
	  "pxor      %%mm7, %%mm5 \n\t"	/* take 1's compliment of only neg. words */
	  "psubsw    %%mm6, %%mm4 \n\t"	/* add 1 to only neg. words, W-(-1) or W-0 */
	  "psubsw    %%mm7, %%mm5 \n\t"	/* add 1 to only neg. words, W-(-1) or W-0 */
	   "packuswb  %%mm5, %%mm4 \n\t"	/* combine and pack/saturate MM5 and MM4 */
	  "movq    %%mm4, (%%edi) \n\t"	/* store result in Dest */
/* --- */
	  "sub       %%eax, %%esi \n\t"	/* move to the current top row in Src */
	  "sub       %%eax, %%esi \n\t" "add $8,          %%esi \n\t"	/* move Src  pointer to the next 8 pixels */
	  "add $8,          %%edi \n\t"	/* move Dest pointer to the next 8 pixels */
/* --- */
	  "dec              %%ecx \n\t"	/* decrease loop counter COLUMNS */
	  "jnz            .L10412 \n\t"	/* check loop termination, proceed if required */
	   "mov       %%ebx, %%esi \n\t"	/* restore most left current row Src  address */
	  "mov       %%edx, %%edi \n\t"	/* restore most left current row Dest address */
	  "add       %%eax, %%esi \n\t"	/* move to the next row in Src */
	  "add       %%eax, %%edi \n\t"	/* move to the next row in Dest */
	   "decl                %2 \n\t"	/* decrease loop counter ROWS */
	  "jnz            .L10410 \n\t"	/* check loop termination, proceed if required */
/* --- */
	  "emms                   \n\t"	/* exit MMX state */
	  "popa                   \n\t":"=m" (Dest)	/* %0 */
	  :"m"(Src),		/* %1 */
	  "m"(rows),		/* %2 */
	  "m"(columns),		/* %3 */
	  "m"(NRightShift)	/* %4 */
	    );
#endif
	return (0);
    } else {
	/* No non-MMX implementation yet */
	return (-1);
    }
}

/* Align stack to 32 byte boundary */
void SDL_imageFilterAlignStack(void)
{
#ifdef USE_MMX
    asm volatile
     (				/* --- stack alignment --- */
	 "mov       %%esp, %%ebx \n\t"	/* load ESP into EBX */
	 "sub          $4, %%ebx \n\t"	/* reserve space on stack for old value of ESP */
	 "and        $-32, %%ebx \n\t"	/* align EBX along a 32 byte boundary */
	 "mov     %%esp, (%%ebx) \n\t"	/* save old value of ESP in stack, behind the bndry */
	 "mov       %%ebx, %%esp \n\t"	/* align ESP along a 32 byte boundary */
	 ::);
#endif
}

/* Restore previously aligned stack */
void SDL_imageFilterRestoreStack(void)
{
#ifdef USE_MMX
    asm volatile
     (				/* --- restoring old stack --- */
	 "mov     (%%esp), %%ebx \n\t"	/* load old value of ESP */
	 "mov       %%ebx, %%esp \n\t"	/* restore old value of ESP */
	 ::);
#endif
}
