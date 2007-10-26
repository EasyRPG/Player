/**
	@file
	@brief		RPG_RT.ldb (LcfDataBase) class manager
	@author		sue445
*/
#ifndef _INC_CRPGLDB
#define _INC_CRPGLDB

#include "CRpgIOBase.h"

/// RPG_RT.ldb (LcfDataBase) class manager
class CRpgLdb : public CRpgIOBase{
private:
	const char* GetHeader(){ return "LcfDataBase"; }	///< specific file header (overrides CRpgIOBase default empty return value)

public:
	/// ChipSet
	struct ChipSet{
		string strName;		///< 0x01: Name
		string strFile;		///< 0x02: File name
	};
	smart_array< ChipSet >	saChipSet;

public:
	CRpgLdb(){}								///< Constructor
	~CRpgLdb(){}							///< Destructor

	bool Init(const char* szDir="");		///< Initialization
};


#endif
