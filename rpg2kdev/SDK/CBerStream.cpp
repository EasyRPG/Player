/**
	@file
	@brief		BER compressed integer input/output using CBasicStream.\n
				Read about BER compressed integer in this URI:\n
				http://www.stellar.ac/~komai/unix/ber/index.html

	@author		sue445
*/
#include "CBerStream.h"
#include "CBer.h"


//=============================================================================
/**
	ストリームからBER圧縮整数を1つ取り出す

	@param	lpReadByte		[out] ストリームから読み込んだバイト数が返る(NULL指定時は返らない)
http://kanjidict.stc.cx/4160
	@return					BER圧縮整数を普通の整数に直した値
*/
//=============================================================================
int CBerStream::ReadBerNumber(int* lpReadByte)
{
	unsigned char c;
	int data = 0, readbyte = 0;


	while(!IsEof()){
		if(Read(&c, 1)==0)	break;
		readbyte++;

		// End of block
		if((c & 0x80)==0){
			data = (data<<7) + c;
			break;
		}
		else{
			data = (data<<7) + (c & 0x7F);
		}
	}

	// 実際に読み込んだバイトを返す
	if(lpReadByte)	*lpReadByte = readbyte;

	return data;
}

//=============================================================================
/**
	BER圧縮整数を1つ書き出す

	@param	nData	[in] 変換元の整数
	@param	bCalc	[in] trueの時は必要サイズの計算だけを行いストリームには書き出さない

	@return			ストリームに書き出したバイト数(ストリームに書き出すのに必要なバイト数)
*/
//=============================================================================
int CBerStream::WriteBerNumber(int nData, bool bCalc)
{
	// BERの構造上一度バッファに書き出しておく必要がある
	smart_buffer buf = CBer::SetBerNumber(nData);
	if(buf.GetSize()>0 && !bCalc){
		Write(buf, buf.GetSize());
	}

	return buf.GetSize();
}

