/**
	@file
	@brief		Save??.lsd (LcfSaveData) class manager
	@author		sue445
*/
#ifndef _INC_CRPGLSD
#define _INC_CRPGLSD

#include "CRpgIOBase.h"

const int MAX_SAVEDATA = 15;	///< Max number of save data


/// Save??.lsd (LcfSaveData) class manager
class CRpgLsd : public CRpgIOBase{
private:
	const char* GetHeader(){ return "LcfSaveData"; }	///< specific file header (overrides CRpgIOBase default empty return value)

public:
	CRpgArray1		ArrayLoading;			///< 0x64: Information on save data selection scene

public:
	CRpgLsd(){}						///< Constructor
	~CRpgLsd(){}						///< Destructor

	bool Init(int nSaveNum, const char* szDir="");		///< Initialization
};

#endif
