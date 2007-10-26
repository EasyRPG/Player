/**
	@file
	@brief		BER compressed integer input/output using CBasicStream.\n
				Read about BER compressed integer in this URI:\n
				http://www.stellar.ac/~komai/unix/ber/index.html

	@author		sue445
*/
#ifndef _INC_CBERSTREAM
#define _INC_CBERSTREAM

#include "sueLib/CBasicStream.h"
#include "sueLib/smart_buffer.h"

using namespace sueLib;


/// Input/output of BER compressed integer
class CBerStream : public CBasicStream{
public:
	CBerStream(){}										///< Constructor
	~CBerStream(){}										///< Destructor

	int ReadBerNumber(int* lpReadByte=NULL);			///< Reads 1 BER compressed integer
	int WriteBerNumber(int nData, bool bCalc=false);	///< Writes 1 BER compressed integer
};


#endif
