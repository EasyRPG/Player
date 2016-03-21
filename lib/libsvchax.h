#ifndef __SVCHAX_H__
#define _SVCHAX_H__

/*
 * requirements when calling svchax_init():
 *
 * - gfxInit was already called.
 * - new 3DS higher clockrate and L2 cache are disabled.
 * - at least 64kb (16 pages) of unallocated linear memory.
 *
 * __ctr_svchax will contain 1 on success and 0 on failure
 *
 * svchax_init() will grant full svc access to the main thread,
 * up to system version 10.6 (kernel version 2.50-11), by using:
 * - memchunkhax1 for kernel version <= 2.46-0
 * - memchunkhax2 for 2.46-0 < kernel version <= 2.50-11
 *
 * svchax assumes that CIA builds already have acces to svcBackdoor
 * and will skip running memchunkhax there.
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

u8 haxInit();

#ifdef __cplusplus
}
#endif


#endif //_SVCHAX_H__
