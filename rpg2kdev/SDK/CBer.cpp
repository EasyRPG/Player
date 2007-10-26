/**
	@file
	@brief		Parses BER compressed integers in data streams
	@author		sue445
*/
#include "CBer.h"
#include "CBerStream.h"


//=============================================================================
/**
	int to BER conversion (First the size. Second, the data)

	@param	nData	[in] Original integer to convert
	@param	pDst	[out] Output buffer destination (if NULL is input, returns size)
	@param	nSize	[in] Output buffer destination size

	@return		Number of bytes written out to the stream (number of bytes required to write out to the stream)
*/
//=============================================================================
int CBer::int2ber(int nData, void* pDst, unsigned int nSize)
{
	unsigned int data = (unsigned int)nData;
	unsigned int i = 0, j;
	unsigned char* c = reinterpret_cast< unsigned char* >( pDst );

	for(;;){
		j = data & 0x7F;

		if(pDst && i<nSize){
			if(i==0)	c[i] = (unsigned char)j;
			else		c[i] = (unsigned char)(j|0x80);
		}
		i++;

		data>>=7;
		if(data==0)		break;
	}

	if(pDst)	MemoryReverse(pDst, nSize);

	return i;
}

//=============================================================================
/**
	Reverses memory contents

	@param	pData	[in,out] Memory data to reverse
	@param	nSize	[in] Size of memory data
*/
//=============================================================================
void CBer::MemoryReverse(void* pData, unsigned int nSize)
{
	if(pData==NULL || nSize<2)		return;

	unsigned char* p = reinterpret_cast< unsigned char* >( pData );
	unsigned char  c;
	unsigned int max = nSize / 2;

	for(unsigned int i = 0; i < max; i++){
		c				= p[i];
		p[i]			= p[nSize-1-i];
		p[nSize-1-i]	= c;
	}
}

//=============================================================================
/**
	Gets BER compressed integer directly from the buffer

	@param	buf	[in] The buffer where BER compressed integer is stored

	@return		The value which corrected BER compressed integer in normal integer
*/
//=============================================================================
int CBer::GetBerNumber(smart_buffer& buf)
{
	CBerStream st;

	if(st.OpenFromMemory(buf, buf.GetSize())){
		return st.ReadBerNumber();
	}

	return 0;
}

//=============================================================================
/**
	Sets BER compressed integer to the buffer

	@param	nData	[in] Original integer to convert

	@return		Output buffer
*/
//=============================================================================
smart_buffer CBer::SetBerNumber(int nData)
{
	smart_buffer buf;
	int size = int2ber(nData);

	if(size>0){
		buf.New(size);
		int2ber(nData, buf, buf.GetSize());
		return buf;
	}

	return buf;
}
