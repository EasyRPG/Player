/**
	@file
	@brief		RPG_RT.ldb (LcfDataBase) class manager
	@author		sue445
*/
#include "CRpgLdb.h"

//=============================================================================
/**
	Initialization

	@param	szDir		[in] Reading folder

	@retval			Successfully reading database
	@retval			Failed reading database
*/
//=============================================================================
bool CRpgLdb::Init(const char* szDir)
{
	int type, id, max;
	smart_buffer	buf;
	string			strFile;
	CRpgArray2		array2;


	bInit = false;
	if(strlen(szDir)){
		strFile += szDir;
		strFile += "\\";
	}
	strFile += "RPG_RT.ldb";

	// No save data
	if(!OpenFile(strFile.c_str()))		return false;


	// Data reading
	while(!IsEof()){
		type = ReadBerNumber();
		buf = ReadData();

		switch(type){
		case 0x14:	// ChipSet
			array2 = CRpgUtil::GetArray2(buf);
			max = array2.GetMaxRow();
			saChipSet.Resize(max+1);
			for(id = 1; id<=max; id++){
				saChipSet[id].strName = array2.GetString(id, 0x01); // Name
				saChipSet[id].strFile = array2.GetString(id, 0x02); // File name
			}
			break;
		}
	}

	bInit = true;
	return true;
}

