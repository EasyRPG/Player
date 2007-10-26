/**
	@file
	@brief		SDK main include file
	@author		sue445
*/
#ifndef _INC_RPG2KDEVSDK_INDEX
#define _INC_RPG2KDEVSDK_INDEX

#include "CRpgLsd.h"
#include "CRpgLdb.h"
#include "CRpgLmu.h"
#include "CRpgImage.h"

#ifdef _MSC_VER
# ifndef _DEBUG
//#  pragma comment( linker , "/NODEFAULTLIB:libcmt.lib" )
#  pragma comment( linker , "/NODEFAULTLIB:libc.lib" )
# else
#  pragma comment( linker , "/NODEFAULTLIB:libcd.lib" )
# endif
#endif

#endif
