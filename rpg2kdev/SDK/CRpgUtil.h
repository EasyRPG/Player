/**
	@file
	@brief		ストリームを介せずにバッファから直接データを操作する
	@author		sue445
*/
#ifndef _INC_CRPGUTIL
#define _INC_CRPGUTIL

#include "CBer.h"
#include "CRpgArray.h"


/// ストリームを介せずにバッファから直接データを操作する
class CRpgUtil : public CBer{
public:
	static	smart_buffer	GetData(  const smart_buffer& buf);
	static	string			GetString(const smart_buffer& buf);
	static	CRpgArray1		GetArray1(const smart_buffer& buf);
	static	CRpgArray2		GetArray2(const smart_buffer& buf);
};

#endif
