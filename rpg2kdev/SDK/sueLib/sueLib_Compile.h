/**
	@file
	@brief		Settings used on debug compilation only
	@author		sue445
*/
#ifndef _INC_SUELIB_COMPILE
#define _INC_SUELIB_COMPILE

#ifdef _DEBUG
# pragma warning(disable:4786) // Disables warning where the identifier is too long
#endif

#if (_MSC_VER>1200)	// VC++.NET or later
# pragma warning(disable:4996) // Disables warning where and old type is declared
#endif

#endif
