/*
* $Id: math-sll.c,v 1.15 2002/08/20 18:01:54 andrewm Exp $
*
* Change: CHUI
*
* Purpose
*    A fixed point (31.32 bit) math library.
*
* Description
*    Floating point packs the most accuracy in the available bits, but it
*    often provides more accuracy than is required.  It is time consuming to
*    carry the extra precision around, particularly on platforms that don't
*    have a dedicated floating point processor.
*
*    This library is a compromise.  All math is done using the 64 bit signed
*    "long long" format (sll), and is not intended to be portable, just as
*    fast as possible.  Since "long long" is a elementary type, it can be
*    passed around without resorting to the use of pointers.  Since the
*    format used is fixed point, there is never a need to do time consuming
*    checks and adjustments to maintain normalized numbers, as is the case
*    in floating point.
*
*    Simply put, this library is limited to handling numbers with a whole
*    part of up to 2^31 - 1 = 2.147483647e9 in magnitude, and fractional
*    parts down to 2^-32 = 2.3283064365e-10 in magnitude.  This yields a
*    decent range and accuracy for many applications.
*
* IMPORTANT
*    No checking for arguments out of range (error).
*    No checking for divide by zero (error).
*    No checking for overflow (error).
*    No checking for underflow (warning).
*    Chops, doesn't round.
*
* Functions
*    sll dbl2sll(double x)            double -> sll
*    double slldbl(sll x)            sll -> double
*
*    sll slladd(sll x, sll y)        x + y
*    sll sllsub(sll x, sll y)        x - y
*    sll sllmul(sll x, sll y)        x * y
*    sll slldiv(sll x, sll y)        x / y
*
*    sll sllinv(sll v)            1 / x
*    sll sllmul2(sll x)            x * 2
*    sll sllmul4(sll x)            x * 4
*    sll sllmul2n(sll x, int n)        x * 2^n, 0 <= n <= 31
*    sll slldiv2(sll x)            x / 2
*    sll slldiv4(sll x)            x / 4
*    sll slldiv2n(sll x, int n)        x / 2^n, 0 <= n <= 31
*
*    sll sllcos(sll x)            cos x
*    sll sllsin(sll x)            sin x
*    sll slltan(sll x)            tan x
*    sll sllatan(sll x)            atan x
*
*    sll sllexp(sll x)            e^x
*    sll slllog(sll x)            ln x
*
*    sll sllpow(sll x, sll y)        x^y
*    sll sllsqrt(sll x)            x^(1 / 2)
*
* History
*    * Aug 20 2002 Nicolas Pitre <nico@cam.org> v1.15
*    - Replaced all shifting assembly with C equivalents
*    - Reformated ARM asm and changed comments to begin with @
*    - Updated C version of sllmul()
*    - Removed the unsupported architecture #error - should be portable now
*
*    * Aug 17 2002 Andrew E. Mileski <andrewm@isoar.ca> v1.14
*    - Fixed sign handling of ARM sllmul()
*    - Ported sllmul() to x86 - it can be inlined now
*    - Updated the sllmul() comments to reflect my changes
*    - Updated the header comments
*
*    * Aug 17 2002 Nicolas Pitre <nico@cam.org> v1.13
*    - Corrected and expanded upon Andrew's sllmul() comments
*    - Added in an non-optimal but portable C version of sllmul()
*
*    * Aug 16 2002 Andrew E. Mileski <andrewm@isoar.ca> v1.12
*    - Added in corrected optimized sllmul() for ARM by Nicolas Pitre
*    - Changed comments on multiplication to describe Nicolas's method
*
*    * Jun 17 2002 Andrew E. Mileski <andrewm@isoar.ca> v1.11
*    - Reverted optimized sllmul() for ARM because of bug
*
*    * Jun 17 2002 Andrew E. Mileski <andrewm@isoar.ca> v1.10
*    - Added in optimized sllmul() for ARM by Nicolas Pitre
*    - Changed comments on multiplication to describe Nicolas's method
*    - Optimized multiplications and divisions by powers of 2
*
*    * Feb  5 2002 Andrew E. Mileski <andrewm@isoar.ca> v1.9
*    - Optimized multiplcations and divisions by powers of 2
*
*    * Feb  5 2002 Andrew E. Mileski <andrewm@isoar.ca> v1.8
*    - Consolidated constants
*    - Added macro for _slladd() _sllsub()
*    - Removed __inline__ from slladd() sllsub()
*    - Renamed umul() to ullmul() and made global
*    - Added function prototypes
*    - Corrected header comment about fractional range
*    - Added warning for non-Linux operating systems
*
*    * Feb  5 2002 Andrew E. Mileski <andrewm@isoar.ca> v1.7
*    - Corrected some i386 assembly comments
*    - Renamed calc_*() to _sll*()
*    - Moved _sllexp() closer to sllexp()
*
*    * Feb  5 2002 Andrew E. Mileski <andrewm@isoar.ca> v1.6
*    - Added sllmul2() sllmul4() sllmul2n() for i386
*    - Added slldiv2() slldiv4() slldiv2n() for i386
*    - Removed input constraints on sllmul2() sllmul4() sllmul2n() for ARM
*    - Removed input constraints on slldiv2() slldiv4() slldiv2n() for ARM
*    - Modified ARM assembly for WYSIWYG output
*    - Changed asm to __asm__
*
*    * Feb  5 2002 Andrew E. Mileski <andrewm@isoar.ca> v1.5
*    - Fixed umul() for i386
*    - Fixed dbl2sll() and sll2dbl() - I forgot ARM doubles are big-endian
*    - Very lightly tested on ARM and i386 and it seems okay
*
*    * Feb  4 2002 Andrew E. Mileski <andrewm@isoar.ca> v1.4
*    - Added umul() for i386
*
*    * Jan 20 2002 Andrew E. Mileski <andrewm@isoar.ca> v1.3
*    - Added fast multiplication functions sllmul2(), sllmul4(), sllmul2n()
*    - Added fast division functions slldiv2() slldiv(), slldiv4n()
*    - Added square root function sllsqrt()
*    - Added library roll-call
*    - Reformatted the history to RPM format (ick)
*    - Moved sllexp() closer to related functions
*    - Added algorithm description to sllpow()
*
*    * Jan 19 2002 Andrew E. Mileski <andrewm@isoar.ca> v1.1
*    - Corrected constants, thanks to Mark A. Lisher for noticing
*    - Put source under CVS control
*
*    * Jan 18 2002 Andrew E. Mileski <andrewm@isoar.ca>
*    - Added some more explanation to calc_cos() and calc_sin()
*    - Added sllatan() and documented it fairly verbosely
*
*    * July 13 2000 Andrew E. Mileski <andrewm@isoar.ca>
*    - Corrected documentation for multiplication algorithm
*
*    * May 21 2000 Andrew E. Mileski <andrewm@isoar.ca>
*    - Rewrote slltanx() to avoid scaling argument for both sine and cosine
*
*    * May 19 2000  Andrew E. Mileski <andrewm@isoar.ca>
*    - Unrolled loops
*    - Added sllinv(), and sllneg()
*    - Changed constants to type "LL" (was "UL" - oops)
*    - Changed all routines to use inverse constants instead of division
*
*    * May 15, 2000 - Andrew E. Mileski <andrewm@isoar.ca>
*    - Fixed slltan() - used sin/cos instead of sllsin/sllcos.  Doh!
*    - Added slllog(x) and sllpow(x,y)
*
*    * May 11, 2000 - Andrew E. Mileski <andrewm@isoar.ca>
*    - Added simple tan(x) that could stand some optimization
*    - Added more constants (see <math.h>)
*
*    * May 3, 2000 - Andrew E. Mileski <andrewm@isoar.ca>
*    - Added sllsin(), sllcos(), and trig constants
*
*    * May 2, 2000 - Andrew E. Mileski <andrewm@isoar.ca>
*    - All routines and macros now have sll their identifiers
*    - Changed mul() to umul() and added sllmul() to handle signed numbers
*    - Added and tested sllexp(), sllint(), and sllfrac()
*    - Added some constants
*
*    * Apr 26, 2000 - Andrew E. Mileski <andrewm@isoar.ca>
*    - Added mul(), and began testing it (unsigned only)
*
*    * Apr 25, 2000 - Andrew E. Mileski <andrewm@isoar.ca>
*    - Added sll2dbl() [convert a signed long long to a double]
*    - Began testing.  Well gee whiz it works! :)
*
*    * Apr 24, 2000 - Andrew E. Mileski <andrewm@isoar.ca>
*    - Added dbl2sll() [convert a double to signed long long]
*    - Began documenting
*
*    * Apr ??, 2000 - Andrew E. Mileski <andrewm@isoar.ca>
*    - Conceived, written, and fiddled with
*
*
*        Copyright (C) 2000 Andrew E. Mileski
*
*  This library is free software; you can redistribute it and/or
*  modify it under the terms of the GNU Lesser General Public
*  License version 2 as published by the Free Software Foundation.
*
*  This library is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*  Lesser General Public License for more details.
*
*  You should have received a copy of the GNU Lesser General Public
*  License along with this library; if not, write to the Free Software
*  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
*/

#ifndef MATHSLL_H
#define MATHSLL_H

#ifdef __cplusplus
extern "C" {
#endif


#ifndef USE_FIXED_POINT

#include<math.h>
    
typedef double sll;
typedef double ull;

#define sllvalue(X)     ((double)X)
#define int2sll(X)    ((sll)(X))
#define sll2int(X)    ((int)(X))
#define sll_abs(X)    (fabs(X))
#define sllneg(X)       (-(X))


static __inline__ double sll2dbl(sll x)
{
    return (double)x;
}

static __inline__ sll slladd(sll x, sll y)
{
    return x + y;
}

static __inline__ sll sllsub(sll x, sll y)
{
    return x - y;
}

static __inline__ sll sllmul(sll x, sll y)
{
    return x * y;
}

static __inline__ sll slldiv(sll x, sll y)
{
    return x / y;
}

static __inline__ sll sllinv(sll x)
{
    return 1 / x;
}


static __inline__ float sll2float(sll s)
{
    return (float)s;
}

static __inline__ sll float2sll(float f)
{
    return (sll)f;
}

static __inline__ sll sllsin(sll x)
{
    return sin(x);
}

static __inline__ sll sllcos(sll x)
{
    return cos(x);
}

static __inline__ sll slltan(sll x)
{
    return tan(x);
}

static __inline__ sll sllsqrt(sll x)
{
    return sqrt(x);
}

static __inline__ sll dbl2sll(double dbl)
{
    return (float)dbl;
}

static __inline__ sll sllpow(sll x, sll y)
{
    return pow(x,y);
}

#else

/* Data types */
typedef signed long long sll;
typedef unsigned long long ull;

/* Macros */
#define int2sll(X)    (((sll) (X)) << 32)
#define sllvalue(X)     (X)
#define sll2int(X)    ((int) ((X) >> 32))
#define sll_abs(X)    ((X) & 0xefffffffffffffffLL)
#define sllint(X)    ((X) & 0xffffffff00000000LL)
#define sllfrac(X)    ((X) & 0x00000000ffffffffLL)
#define sllneg(X)    (-(X))
#define _slladd(X,Y)    ((X) + (Y))
#define _sllsub(X,Y)    ((X) - (Y))

/* Constants (converted from double) */
#define CONST_0        0x0000000000000000LL
#define CONST_1        0x0000000100000000LL
#define CONST_2        0x0000000200000000LL
#define CONST_3        0x0000000300000000LL
#define CONST_4        0x0000000400000000LL
#define CONST_10    0x0000000a00000000LL
#define CONST_1_2    0x0000000080000000LL
#define CONST_1_3    0x0000000055555555LL
#define CONST_1_4    0x0000000040000000LL
#define CONST_1_5    0x0000000033333333LL
#define CONST_1_6    0x000000002aaaaaaaLL
#define CONST_1_7    0x0000000024924924LL
#define CONST_1_8    0x0000000020000000LL
#define CONST_1_9    0x000000001c71c71cLL
#define CONST_1_10    0x0000000019999999LL
#define CONST_1_11    0x000000001745d174LL
#define CONST_1_12    0x0000000015555555LL
#define CONST_1_20    0x000000000cccccccLL
#define CONST_1_30    0x0000000008888888LL
#define CONST_1_42    0x0000000006186186LL
#define CONST_1_56    0x0000000004924924LL
#define CONST_1_72    0x00000000038e38e3LL
#define CONST_1_90    0x0000000002d82d82LL
#define CONST_1_110    0x000000000253c825LL
#define CONST_1_132    0x0000000001f07c1fLL
#define CONST_1_156    0x0000000001a41a41LL
#define CONST_E        0x00000002b7e15162LL
#define CONST_1_E    0x000000005e2d58d8LL
#define CONST_SQRTE    0x00000001a61298e1LL
#define CONST_1_SQRTE    0x000000009b4597e3LL
#define CONST_LOG2_E    0x0000000171547652LL
#define CONST_LOG10_E    0x000000006f2dec54LL
#define CONST_LN2    0x00000000b17217f7LL
#define CONST_LN10    0x000000024d763776LL
#define CONST_PI    0x00000003243f6a88LL
#define CONST_PI_2    0x00000001921fb544LL
#define CONST_PI_4    0x00000000c90fdaa2LL
#define CONST_1_PI    0x00000000517cc1b7LL
#define CONST_2_PI    0x00000000a2f9836eLL
#define CONST_2_SQRTPI    0x0000000120dd7504LL
#define CONST_SQRT2    0x000000016a09e667LL
#define CONST_1_SQRT2    0x00000000b504f333LL

static __inline__ sll slladd(sll x, sll y)
{
    return (x + y);
}

static __inline__ sll sllsub(sll x, sll y)
{
    return (x - y);
}

/*
* Let a = A * 2^32 + a_hi * 2^0 + a_lo * 2^(-32)
* Let b = B * 2^32 + b_hi * 2^0 + b_lo * 2^(-32)
*
* Where:
*   *_hi is the integer part
*   *_lo the fractional part
*   A and B are the sign (0 for positive, -1 for negative).
*
* a * b = (A * 2^32 + a_hi * 2^0 + a_lo * 2^-32)
*       * (B * 2^32 + b_hi * 2^0 + b_lo * 2^-32)
*
* Expanding the terms, we get:
*
*     = A * B * 2^64 + A * b_h * 2^32 + A * b_l * 2^0
*     + a_h * B * 2^32 + a_h * b_h * 2^0 + a_h * b_l * 2^-32
*     + a_l * B * 2^0 + a_l * b_h * 2^-32 + a_l * b_l * 2^-64
*
* Grouping by powers of 2, we get:
*
*     = A * B * 2^64
*     Meaningless overflow from sign extension - ignore
*
*     + (A * b_h + a_h * B) * 2^32
*     Overflow which we can't handle - ignore
*
*     + (A * b_l + a_h * b_h + a_l * B) * 2^0
*     We only need the low 32 bits of this term, as the rest is overflow
*
*     + (a_h * b_l + a_l * b_h) * 2^-32
*     We need all 64 bits of this term
*
*     +  a_l * b_l * 2^-64
*     We only need the high 32 bits of this term, as the rest is underflow
*
* Note that:
*   a > 0 && b > 0: A =  0, B =  0 and the third term is a_h * b_h
*   a < 0 && b > 0: A = -1, B =  0 and the third term is a_h * b_h - b_l
*   a > 0 && b < 0: A =  0, B = -1 and the third term is a_h * b_h - a_l
*   a < 0 && b < 0: A = -1, B = -1 and the third term is a_h * b_h - a_l - b_l
*/
#if defined(__arm__)
static __inline__ sll sllmul(sll left, sll right)
{
    /*
     * From gcc/config/arm/arm.h:
     *   In a pair of registers containing a DI or DF value the 'Q'
     *   operand returns the register number of the register containing
     *   the least significant part of the value.  The 'R' operand returns
     *   the register number of the register containing the most
     *   significant part of the value.
     */
    sll retval;

    __asm__ (
        "@ sllmul\n\t"
        "umull    %R0, %Q0, %Q1, %Q2\n\t"
        "mul    %R0, %R1, %R2\n\t"
        "umlal    %Q0, %R0, %Q1, %R2\n\t"
        "umlal    %Q0, %R0, %Q2, %R1\n\t"
            "tst    %R1, #0x80000000\n\t"
            "subne    %R0, %R0, %Q2\n\t"
            "tst    %R2, #0x80000000\n\t"
            "subne    %R0, %R0, %Q1\n\t"
        : "=&r" (retval)
        : "%r" (left), "r" (right)
        : "cc"
    );

    return retval;
}
#elif defined(__i386__)
static __inline__ sll sllmul(sll left, sll right)
{
    register sll retval;
    __asm__(
        "# sllmul\n\t"
        "    movl    %1, %%eax\n\t"
        "    mull     %3\n\t"
        "    movl    %%edx, %%ebx\n\t"
        "\n\t"
        "    movl    %2, %%eax\n\t"
        "    mull     %4\n\t"
        "    movl    %%eax, %%ecx\n\t"
        "\n\t"
        "    movl    %1, %%eax\n\t"
        "    mull    %4\n\t"
        "    addl    %%eax, %%ebx\n\t"
        "    adcl    %%edx, %%ecx\n\t"
        "\n\t"
        "    movl    %2, %%eax\n\t"
        "    mull    %3\n\t"
        "    addl    %%ebx, %%eax\n\t"
        "    adcl    %%ecx, %%edx\n\t"
        "\n\t"
        "    btl    $31, %2\n\t"
        "    jnc    1f\n\t"
        "    subl    %3, %%edx\n\t"
        "1:    btl    $31, %4\n\t"
        "    jnc    1f\n\t"
        "    subl    %1, %%edx\n\t"
        "1:\n\t"
        : "=&A" (retval)
        : "m" (left), "m" (((unsigned *) &left)[1]),
          "m" (right), "m" (((unsigned *) &right)[1])
        : "ebx", "ecx", "cc"
    );
    return retval;
}
#else
/* Plain C version: not optimal but portable. */
#warning Fixed Point no optimal
static __inline__ sll sllmul(sll a, sll b)
{
    unsigned int a_lo, b_lo;
    signed int a_hi, b_hi;
    sll x;

    a_lo = a;
    a_hi = (ull) a >> 32;
    b_lo = b;
    b_hi = (ull) b >> 32;

    x = ((ull) (a_hi * b_hi) << 32)
      + (((ull) a_lo * b_lo) >> 32)
      + (sll) a_lo * b_hi
      + (sll) b_lo * a_hi;

    return x;
}
#endif

static __inline__ sll sllinv(sll v)
{
    int sgn = 0;
    sll u;
    ull s = -1;

    /* Use positive numbers, or the approximation won't work */
    if (v < CONST_0) {
        v = sllneg(v);
        sgn = 1;
    }

    /* An approximation - must be larger than the actual value */
    for (u = v; u; ((ull)u) >>= 1)
        s >>= 1;

    /* Newton's Method */
    u = sllmul(s, _sllsub(CONST_2, sllmul(v, s)));
    u = sllmul(u, _sllsub(CONST_2, sllmul(v, u)));
    u = sllmul(u, _sllsub(CONST_2, sllmul(v, u)));
    u = sllmul(u, _sllsub(CONST_2, sllmul(v, u)));
    u = sllmul(u, _sllsub(CONST_2, sllmul(v, u)));
    u = sllmul(u, _sllsub(CONST_2, sllmul(v, u)));

    return ((sgn) ? sllneg(u): u);
}

static __inline__ sll slldiv(sll left, sll right)
{
    return sllmul(left, sllinv(right));
}

static __inline__ sll sllmul2(sll x)
{
    return x << 1;
}

static __inline__ sll sllmul4(sll x)
{
    return x << 2;
}

static __inline__ sll sllmul2n(sll x, int n)
{
    sll y;

#if defined(__arm__)
    /*
     * On ARM we need to do explicit assembly since the compiler
     * doesn't know the range of n is limited and decides to call
     * a library function instead.
     */
    __asm__ (
        "@ sllmul2n\n\t"
        "mov    %R0, %R1, lsl %2\n\t"
        "orr    %R0, %R0, %Q1, lsr %3\n\t"
        "mov    %Q0, %Q1, lsl %2\n\t"
        : "=r" (y)
        : "r" (x), "rM" (n), "rM" (32 - n)
    );
#else
    y = x << n;
#endif

    return y;
}

static __inline__ sll slldiv2(sll x)
{
    return x >> 1;
}

static __inline__ sll slldiv4(sll x)
{
    return x >> 2;
}

static __inline__ sll slldiv2n(sll x, int n)
{
    sll y;

#if defined(__arm__)
    /*
     * On ARM we need to do explicit assembly since the compiler
     * doesn't know the range of n is limited and decides to call
     * a library function instead.
     */
    __asm__ (
        "@ slldiv2n\n\t"
        "mov    %Q0, %Q1, lsr %2\n\t"
        "orr    %Q0, %Q0, %R1, lsl %3\n\t"
        "mov    %R0, %R1, asr %2\n\t"
        : "=r" (y)
        : "r" (x), "rM" (n), "rM" (32 - n)
    );
#else
    y = x >> n;
#endif

    return y;
}

/*
* Unpack the IEEE floating point double format and put it in fixed point
* sll format.
*/
static __inline__ sll dbl2sll(double dbl)
{
    union {
        double d;
        unsigned u[2];
        ull ull;
        sll sll;
    } in, retval;
    register unsigned exp;

    /* Move into memory as args might be passed in regs */
    in.d = dbl;

#if defined(__arm__)

    /* ARM architecture has a big-endian double */
    exp = in.u[0];
    in.u[0] = in.u[1];
    in.u[1] = exp;

#endif /* defined(__arm__) */

    /* Leading 1 is assumed by IEEE */
    retval.u[1] = 0x40000000;

    /* Unpack the mantissa into the unsigned long */
    retval.u[1] |= (in.u[1] << 10) & 0x3ffffc00;
    retval.u[1] |= (in.u[0] >> 22) & 0x000003ff;
    retval.u[0] = in.u[0] << 10;

    /* Extract the exponent and align the decimals */
    exp = (in.u[1] >> 20) & 0x7ff;
    if (exp)
        retval.ull >>= 1053 - exp;
    else
        return 0L;

    /* Negate if negative flag set */
    if (in.u[1] & 0x80000000)
        retval.sll = -retval.sll;

    return retval.sll;
}

static __inline__ sll float2sll(float f)
{
    return dbl2sll((double)f);
}

static __inline__ double sll2dbl(sll s)
{
    union {
        double d;
        unsigned u[2];
        ull ull;
        sll sll;
    } in, retval;
    register unsigned exp;
    register unsigned flag;

    if (s == 0)
        return 0.0;

    /* Move into memory as args might be passed in regs */
    in.sll = s;

    /* Handle the negative flag */
    if (in.sll < 1) {
        flag = 0x80000000;
        in.ull = sllneg(in.sll);
    } else
        flag = 0x00000000;

    /* Normalize */
    for (exp = 1053; in.ull && (in.u[1] & 0x80000000) == 0; exp--) {
        in.ull <<= 1;
    }
    in.ull <<= 1;
    exp++;
    in.ull >>= 12;
    retval.ull = in.ull;
    retval.u[1] |= flag | (exp << 20);

#if defined(__arm__)

    /* ARM architecture has a big-endian double */
    exp = retval.u[0];
    retval.u[0] = retval.u[1];
    retval.u[1] = exp;

#endif /* defined(__arm__) */

    return retval.d;
}

static __inline__ float sll2float(sll s)
{
    return ((float)sll2dbl(s));
}

/*
* Calculate cos x where -pi/4 <= x <= pi/4
*
* Description:
*    cos x = 1 - x^2 / 2! + x^4 / 4! - ... + x^(2N) / (2N)!
*    Note that (pi/4)^12 / 12! < 2^-32 which is the smallest possible number.
*/
static __inline__ sll _sllcos(sll x)
{
    sll retval, x2;
    x2 = sllmul(x, x);
    /*
     * cos x = t0 + t1 + t2 + t3 + t4 + t5 + t6
     *
     * f0 =  0! =  1
     * f1 =  2! =  2 *  1 * f0 =   2 * f0
     * f2 =  4! =  4 *  3 * f1 =  12 x f1
     * f3 =  6! =  6 *  5 * f2 =  30 * f2
     * f4 =  8! =  8 *  7 * f3 =  56 * f3
     * f5 = 10! = 10 *  9 * f4 =  90 * f4
     * f6 = 12! = 12 * 11 * f5 = 132 * f5
     *
     * t0 = 1
     * t1 = -t0 * x2 /   2 = -t0 * x2 * CONST_1_2
     * t2 = -t1 * x2 /  12 = -t1 * x2 * CONST_1_12
     * t3 = -t2 * x2 /  30 = -t2 * x2 * CONST_1_30
     * t4 = -t3 * x2 /  56 = -t3 * x2 * CONST_1_56
     * t5 = -t4 * x2 /  90 = -t4 * x2 * CONST_1_90
     * t6 = -t5 * x2 / 132 = -t5 * x2 * CONST_1_132
     */
    retval = _sllsub(CONST_1, sllmul(x2, CONST_1_132));
    retval = _sllsub(CONST_1, sllmul(sllmul(x2, retval), CONST_1_90));
    retval = _sllsub(CONST_1, sllmul(sllmul(x2, retval), CONST_1_56));
    retval = _sllsub(CONST_1, sllmul(sllmul(x2, retval), CONST_1_30));
    retval = _sllsub(CONST_1, sllmul(sllmul(x2, retval), CONST_1_12));
    retval = _sllsub(CONST_1, slldiv2(sllmul(x2, retval)));
    return retval;
}

/*
* Calculate sin x where -pi/4 <= x <= pi/4
*
* Description:
*    sin x = x - x^3 / 3! + x^5 / 5! - ... + x^(2N+1) / (2N+1)!
*    Note that (pi/4)^13 / 13! < 2^-32 which is the smallest possible number.
*/
static __inline__ sll _sllsin(sll x)
{
    sll retval, x2;
    x2 = sllmul(x, x);
    /*
     * sin x = t0 + t1 + t2 + t3 + t4 + t5 + t6
     *
     * f0 =  0! =  1
     * f1 =  3! =  3 *  2 * f0 =   6 * f0
     * f2 =  5! =  5 *  4 * f1 =  20 x f1
     * f3 =  7! =  7 *  6 * f2 =  42 * f2
     * f4 =  9! =  9 *  8 * f3 =  72 * f3
     * f5 = 11! = 11 * 10 * f4 = 110 * f4
     * f6 = 13! = 13 * 12 * f5 = 156 * f5
     *
     * t0 = 1
     * t1 = -t0 * x2 /   6 = -t0 * x2 * CONST_1_6
     * t2 = -t1 * x2 /  20 = -t1 * x2 * CONST_1_20
     * t3 = -t2 * x2 /  42 = -t2 * x2 * CONST_1_42
     * t4 = -t3 * x2 /  72 = -t3 * x2 * CONST_1_72
     * t5 = -t4 * x2 / 110 = -t4 * x2 * CONST_1_110
     * t6 = -t5 * x2 / 156 = -t5 * x2 * CONST_1_156
     */
    retval = _sllsub(x, sllmul(x2, CONST_1_156));
    retval = _sllsub(x, sllmul(sllmul(x2, retval), CONST_1_110));
    retval = _sllsub(x, sllmul(sllmul(x2, retval), CONST_1_72));
    retval = _sllsub(x, sllmul(sllmul(x2, retval), CONST_1_42));
    retval = _sllsub(x, sllmul(sllmul(x2, retval), CONST_1_20));
    retval = _sllsub(x, sllmul(sllmul(x2, retval), CONST_1_6));
    return retval;
}

static __inline__ sll sllcos(sll x)
{
    int i;
    sll retval;

    /* Calculate cos (x - i * pi/2), where -pi/4 <= x - i * pi/2 <= pi/4  */
    i = sll2int(_slladd(sllmul(x, CONST_2_PI), CONST_1_2));
    x = _sllsub(x, sllmul(int2sll(i), CONST_PI_2));

    switch (i & 3) {
        default:
        case 0:
            retval = _sllcos(x);
            break;
        case 1:
            retval = sllneg(_sllsin(x));
            break;
        case 2:
            retval = sllneg(_sllcos(x));
            break;
        case 3:
            retval = _sllsin(x);
            break;
    }
    return retval;
}

static __inline__ sll sllsin(sll x)
{
    int i;
    sll retval;

    /* Calculate sin (x - n * pi/2), where -pi/4 <= x - i * pi/2 <= pi/4 */
    i = sll2int(_slladd(sllmul(x, CONST_2_PI), CONST_1_2));
    x = _sllsub(x, sllmul(int2sll(i), CONST_PI_2));

    switch (i & 3) {
        default:
        case 0:
            retval = _sllsin(x);
            break;
        case 1:
            retval = _sllcos(x);
            break;
        case 2:
            retval = sllneg(_sllsin(x));
            break;
        case 3:
            retval = sllneg(_sllcos(x));
            break;
    }
    return retval;
}

static __inline__ sll slltan(sll x)
{
    int i;
    sll retval;
    i = sll2int(_slladd(sllmul(x, CONST_2_PI), CONST_1_2));
    x = _sllsub(x, sllmul(int2sll(i), CONST_PI_2));
    switch (i & 3) {
        default:
        case 0:
        case 2:
            retval = slldiv(_sllsin(x), _sllcos(x));
            break;
        case 1:
        case 3:
            retval = sllneg(slldiv(_sllcos(x), _sllsin(x)));
            break;
    }
    return retval;
}

/*
* atan x = SUM[n=0,) (-1)^n * x^(2n + 1)/(2n + 1), |x| < 1
*
* Two term approximation
*    a = x - x^3/3
* Gives us
*    atan x = a + ??
* Let ?? = arctan ?
*    atan x = a + arctan ?
* Rearrange
*    atan x - a = arctan ?
* Apply tan to both sides
*    tan (atan x - a) = tan arctan ?
*    tan (atan x - a) = ?
* Applying the standard formula
*    tan (u - v) = (tan u - tan v) / (1 + tan u * tan v)
* Gives us
*    tan (atan x - a) = (tan atan x - tan a) / (1 + tan arctan x * tan a)
* Let t = tan a
*    tan (atan x - a) = (x - t) / (1 + x * t)
* So finally
*    arctan x = a + arctan ((tan x - t) / (1 + x * t))
* And the typical worst case is x = 1.0 which converges in 3 iterations.
*/
static __inline__ sll _sllatan(sll x)
{
    sll a, t, retval;

    /* First iteration */
    a = sllmul(x, _sllsub(CONST_1, sllmul(x, sllmul(x, CONST_1_3))));
    retval = a;

    /* Second iteration */
    t = slldiv(_sllsin(a), _sllcos(a));
    x = slldiv(_sllsub(x, t), _slladd(CONST_1, sllmul(t, x)));
    a = sllmul(x, _sllsub(CONST_1, sllmul(x, sllmul(x, CONST_1_3))));
    retval = _slladd(retval, a);

    /* Third  iteration */
    t = slldiv(_sllsin(a), _sllcos(a));
    x = slldiv(_sllsub(x, t), _slladd(CONST_1, sllmul(t, x)));
    a = sllmul(x, _sllsub(CONST_1, sllmul(x, sllmul(x, CONST_1_3))));
    return _slladd(retval, a);
}

static __inline__ sll sllatan(sll x)
{
    sll retval;

    if (x < -sllneg(CONST_1))
        retval = sllneg(CONST_PI_2);
    else if (x > CONST_1)
        retval = CONST_PI_2;
    else
        return _sllatan(x);
    return _sllsub(retval, _sllatan(sllinv(x)));
}

/*
* Calculate e^x where -0.5 <= x <= 0.5
*
* Description:
*    e^x = x^0 / 0! + x^1 / 1! + ... + x^N / N!
*    Note that 0.5^11 / 11! < 2^-32 which is the smallest possible number.
*/
static __inline__ sll _sllexp(sll x)
{
    sll retval;
    retval = _slladd(CONST_1, sllmul(0, sllmul(x, CONST_1_11)));
    retval = _slladd(CONST_1, sllmul(retval, sllmul(x, CONST_1_11)));
    retval = _slladd(CONST_1, sllmul(retval, sllmul(x, CONST_1_10)));
    retval = _slladd(CONST_1, sllmul(retval, sllmul(x, CONST_1_9)));
    retval = _slladd(CONST_1, sllmul(retval, slldiv2n(x, 3)));
    retval = _slladd(CONST_1, sllmul(retval, sllmul(x, CONST_1_7)));
    retval = _slladd(CONST_1, sllmul(retval, sllmul(x, CONST_1_6)));
    retval = _slladd(CONST_1, sllmul(retval, sllmul(x, CONST_1_5)));
    retval = _slladd(CONST_1, sllmul(retval, slldiv4(x)));
    retval = _slladd(CONST_1, sllmul(retval, sllmul(x, CONST_1_3)));
    retval = _slladd(CONST_1, sllmul(retval, slldiv2(x)));
    return retval;
}

/*
* Calculate e^x where x is arbitrary
*/
static __inline__ sll sllexp(sll x)
{
    int i;
    sll e, retval;

    e = CONST_E;

    /* -0.5 <= x <= 0.5  */
    i = sll2int(_slladd(x, CONST_1_2));
    retval = _sllexp(_sllsub(x, int2sll(i)));

    /* i >= 0 */
    if (i < 0) {
        i = -i;
        e = CONST_1_E;
    }

    /* Scale the result */
    for (;i; i >>= 1) {
        if (i & 1)
            retval = sllmul(retval, e);
        e = sllmul(e, e);
    }
    return retval;
}

/*
* Calculate natural logarithm using Netwton-Raphson method
*/
static __inline__ sll slllog(sll x)
{
    sll x1, ln = 0;

    /* Scale: e^(-1/2) <= x <= e^(1/2) */
    while (x < CONST_1_SQRTE) {
        ln = _sllsub(ln, CONST_1);
        x = sllmul(x, CONST_E);
    }
    while (x > CONST_SQRTE) {
        ln = _slladd(ln, CONST_1);
        x = sllmul(x, CONST_1_E);
    }

    /* First iteration */
    x1 = sllmul(_sllsub(x, CONST_1), slldiv2(_sllsub(x, CONST_3)));
    ln = _sllsub(ln, x1);
    x = sllmul(x, _sllexp(x1));

    /* Second iteration */
    x1 = sllmul(_sllsub(x, CONST_1), slldiv2(_sllsub(x, CONST_3)));
    ln = _sllsub(ln, x1);
    x = sllmul(x, _sllexp(x1));

    /* Third iteration */
    x1 = sllmul(_sllsub(x, CONST_1), slldiv2(_sllsub(x, CONST_3)));
    ln = _sllsub(ln, x1);

    return ln;
}

/*
* ln x^y = y * log x
* e^(ln x^y) = e^(y * log x)
* x^y = e^(y * ln x)
*/
static __inline__ sll sllpow(sll x, sll y)
{
    if (y == CONST_0)
        return CONST_1;
    return sllexp(sllmul(y, slllog(x)));
}

/*
* Consider a parabola centered on the y-axis
*     y = a * x^2 + b
* Has zeros (y = 0)  at
*    a * x^2 + b = 0
*    a * x^2 = -b
*    x^2 = -b / a
*    x = +- (-b / a)^(1 / 2)
* Letting a = 1 and b = -X
*    y = x^2 - X
*    x = +- X^(1 / 2)
* Which is convenient since we want to find the square root of X, and we can
* use Newton's Method to find the zeros of any f(x)
*    xn = x - f(x) / f'(x)
* Applied Newton's Method to our parabola
*    f(x) = x^2 - X
*    xn = x - (x^2 - X) / (2 * x)
*    xn = x - (x - X / x) / 2
* To make this converge quickly, we scale X so that
*    X = 4^N * z
* Taking the roots of both sides
*    X^(1 / 2) = (4^n * z)^(1 / 2)
*    X^(1 / 2) = 2^n * z^(1 / 2)
* Let N = 2^n
*    x^(1 / 2) = N * z^(1 / 2)
* We want this to converge to the positive root, so we must start at a point
*    0 < start <= x^(1 / 2)
* or
*    x^(1/2) <= start <= infinity
* since
*    (1/2)^(1/2) = 0.707
*    2^(1/2) = 1.414
* A good choice is 1 which lies in the middle, and takes 4 iterations to
* converge from either extreme.
*/
static __inline__ sll sllsqrt(sll x)
{
    sll n, xn;
      
    /* Start with a scaling factor of 1 */
    n = CONST_1;

    /* Quick solutions for the simple cases */
    if (x <= CONST_0 || x == CONST_1)
        return x;

    /* Scale x so that 0.5 <= x < 2 */
    while (x >= CONST_2) {
        x = slldiv4(x);
        n = sllmul2(n);
    }
    while (x < CONST_1_2) {
        x = sllmul4(x);
        n = slldiv2(n);
    }

    /* Simple solution if x = 4^n */
    if (x == CONST_1)
        return n;

    /* The starting point */
    xn = CONST_1;

    /* Four iterations will be enough */
    xn = _sllsub(xn, slldiv2(_sllsub(xn, slldiv(x, xn))));
    xn = _sllsub(xn, slldiv2(_sllsub(xn, slldiv(x, xn))));
    xn = _sllsub(xn, slldiv2(_sllsub(xn, slldiv(x, xn))));
    xn = _sllsub(xn, slldiv2(_sllsub(xn, slldiv(x, xn))));

    /* Scale the result */
    return sllmul(n, xn);
}

#endif

#ifdef __cplusplus
}
#endif

#endif /* MATHSLL_H */
