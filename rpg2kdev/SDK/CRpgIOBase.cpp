/**
	@file
	@brief		Basic RPG Maker 2000/2003 IO class file
	@note		Even this class is inherited from various places, you can use it as a single unit.

	@author		sue445
*/
#include "CRpgIOBase.h"


//=============================================================================
/**
	File opening (with header checking)

	@param	szFile		[in] File to open

	@retval	true		Successfully opening file
	@retval	false		Failed opening file
*/
//=============================================================================
bool CRpgIOBase::OpenFile(const char* szFile)
{
	// Open file fails
	if(!OpenFromFile(szFile, false))	return false;

	// Header file doesn't match
	if(ReadString() != GetHeader())		return false;

	return true;
}

//=============================================================================
/**
	RPG Maker data reading ([data size][raw data])

	@return	buf		Output buffer
*/
//=============================================================================
smart_buffer CRpgIOBase::ReadData()
{
	smart_buffer buf;
	int length = ReadBerNumber();	// Buffer length

	if(length>0){
		buf.New(length);
		Read(buf, length);
	}

	return buf;
}

//=============================================================================
/**
	RPG Maker character string reading

	@return	str		Acquired character string
*/
//=============================================================================
string CRpgIOBase::ReadString()
{
	smart_buffer buf = ReadData();
	string str;

	if(buf.GetSize()>0)		str.assign(buf.GetPtr(), buf.GetSize());

	return str;
}

//=============================================================================
/**
	unsigned char -> bool”z—ñ

	@param	num		[in] source input data
	@param	flag		[out] destination output array
*/
//=============================================================================
void CRpgIOBase::ByteToBool(unsigned char num, bool flag[8]) const
{
	/*
	ZeroMemory(flag, sizeof(bool)*8);
	if(num & 0x01)	flag[0] = true;
	if(num & 0x02)	flag[1] = true;
	if(num & 0x04)	flag[2] = true;
	if(num & 0x08)	flag[3] = true;
	if(num & 0x10)	flag[4] = true;
	if(num & 0x20)	flag[5] = true;
	if(num & 0x40)	flag[6] = true;
	if(num & 0x80)	flag[7] = true;
	*/

	/*
	flag[0] = ((num & 0x01) != 0);
	flag[1] = ((num & 0x02) != 0);
	flag[2] = ((num & 0x04) != 0);
	flag[3] = ((num & 0x08) != 0);
	flag[4] = ((num & 0x10) != 0);
	flag[5] = ((num & 0x20) != 0);
	flag[6] = ((num & 0x40) != 0);
	flag[7] = ((num & 0x80) != 0);
	*/

	unsigned int i, mask = 1;
	for(i = 0; i < 8; i++){
		flag[i] = ((num & mask) != 0);
		mask <<= 1;
	}
}

