/**
	@file
	@brief		Parses BER compressed integers in data streams
	@author		sue445
*/
#ifndef _INC_CBER
#define _INC_CBER

#include <stdio.h>
#include "sueLib/smart_buffer.h"

using namespace sueLib;

/// BER management
class CBer{
private:
	static	int  int2ber(int nData, void* pDst=NULL, unsigned int nSize=0);	///< int -> BER

public:
	static	void MemoryReverse(void* pData, unsigned int nSize);	///< Reverses memory contents

	static	int		GetBerNumber(smart_buffer& buf);	///< Gets BER compressed integer from the buffer
	static	smart_buffer	SetBerNumber(int nData);		///< Sets BER compressed integer to the buffer
};

#endif
