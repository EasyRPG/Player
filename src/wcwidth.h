#ifndef _EASYRPG_WCWIDTH_H_
#define _EASYRPG_WCWIDTH_H_

#include "system.h"

int mk_wcwidth(wchar_t ucs);
int mk_wcswidth(const wchar_t *pwcs, size_t n);
int mk_wcwidth_cjk(wchar_t ucs);
int mk_wcswidth_cjk(const wchar_t *pwcs, size_t n);

#endif
