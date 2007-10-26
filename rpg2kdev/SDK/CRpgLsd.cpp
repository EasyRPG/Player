/**
	@file
	@brief		Save??.lsd (LcfSaveData) class manager
	@author		sue445
*/
#include "CRpgLsd.h"


//=============================================================================
/**
	Initialization

	@param	nSaveNum	[in] Save data number (1...15)
	@param	szDir		[in] Reading folder

	@retval			Successfully reading save data
	@retval			Failed reading save data
*/
//=============================================================================
bool CRpgLsd::Init(int nSaveNum, const char* szDir)
{
	int type;
	char file[256];
	smart_buffer	buf;
	string			strFile;


	// Unexpected number of save data
	if(nSaveNum<1 || nSaveNum>MAX_SAVEDATA)		return false;

	bInit = false;
	sprintf(file, "Save%02d.lsd", nSaveNum);
	if(strlen(szDir)){
		strFile += szDir;
		strFile += "\\";
	}
	strFile += file;

	// No save data
	if(!OpenFile(strFile.c_str()))		return false;


	// Data Reading
	while(!IsEof()){
		type = ReadBerNumber();
		buf = ReadData();

		switch(type){
		case 0x64:	// Information on save data selection scene
			ArrayLoading = CRpgUtil::GetArray1(buf);
			break;
		case 0x65:	// Flag information, etc.
			break;
		case 0x66:
			break;
		case 0x67:
			break;
		case 0x68:	// Save point
			break;
		case 0x69:
			break;
		case 0x6A:
			break;
		case 0x6B:
			break;
		case 0x6C:	// party[actor number][attribute]
			break;
		case 0x6D:	// Item information, etc.
			break;
		case 0x6E:
			break;
		case 0x6F:
			break;
		case 0x70:
			break;
		case 0x71:
			break;
		case 0x72:
			break;
		}
	}

	bInit = true;
	return true;
}

