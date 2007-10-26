/**
	@file
	@brief		XYZファイルの入出力を行うクラス
	@author		sue445
*/
#include "CXyzIO.h"
#include "../smart_buffer.h"
#include "CBmpImage.h"

namespace sueLib {

//=============================================================================
/**
	画像を読み込む

	@param	szFile			[in] ファイル名
	@param	img				[out] 出力先のCImage8(XYZは8ビットオンリーのため)

	@retval	true			画像の読み込みに成功
	@retval	false			画像の読み込みに失敗
*/
//=============================================================================
bool CXyzIO::Load(const char* szFile, CImage8& img)
{
	CBasicStream file;
	XYZ_FILEHEADER header;
	unsigned int i;


	if(!file.OpenFromFile(szFile))			return false;	// ファイルが開けない

	file.Read(&header, sizeof(header));
	if(memcmp(header.cID, "XYZ1", 4)!=0)	return false;	// ヘッダが一致しない

	img.Create(header.shWidth, header.shHeight);

	// zlib展開用のバッファを確保
	smart_buffer buf(header.shWidth*header.shHeight + sizeof(XYZ_RGB)*256);

	CBasicStream dec;	// zlibに渡す出力用のストリーム
	dec.OpenFromMemory(buf.GetPtr(), buf.GetSize(), true);

	if(!Decompress(&file, &dec)){
		return false;
	}

	// パレット部
	const XYZ_RGB* plt = (XYZ_RGB*)buf.GetPtr();
	for(i = 0; i < 256; i++){
		img.SetPalette(i, plt[i].red, plt[i].green, plt[i].blue);
	}

	// 画像データ部
	const unsigned char* pData = (const unsigned char*)(buf.GetPtr()+sizeof(XYZ_RGB)*256);
	memcpy(img.Top(), pData, header.shWidth*header.shHeight);


	return true;
}


//=============================================================================
/**
	画像を書き出す

	@param	szFile			[in] ファイル名
	@param	img				[in] 入力元のCImage8(XYZは8ビットオンリーのため)
	@param	nCompLv			[in] 保存時の圧縮レベル(0(無圧縮)～9(最高圧縮), -1で標準圧縮)

	@retval	true			画像の書き出しに成功
	@retval	false			画像の書き出しに失敗
*/
//=============================================================================
bool CXyzIO::Save(const char* szFile, const CImage8& img, int nCompLv)
{
	CBasicStream file;
	XYZ_FILEHEADER	header;
	unsigned int i;


	if(!img.IsInit())						return false;	// 初期化されてない

	if(!file.OpenFromFile(szFile, true))	return false;	// ファイルが開けない

	// ヘッダの準備
	memcpy(header.cID, "XYZ1", 4);
	header.shWidth  = img.GetWidth();
	header.shHeight = img.GetHeight();

	file.Write(&header, sizeof(header));

	// zlib圧縮用のバッファを確保
	smart_buffer buf(header.shWidth*header.shHeight + sizeof(XYZ_RGB)*256);

	// パレット部
	XYZ_RGB* plt = (XYZ_RGB*)buf.GetPtr();
	for(i = 0; i < 256; i++){
		img.GetPalette(i, &plt[i].red, &plt[i].green, &plt[i].blue);
	}

	// 画像データ部
	unsigned char* pData = (unsigned char*)(buf.GetPtr()+sizeof(XYZ_RGB)*256);
	memcpy(pData, img.GetPtr(0, 0), header.shWidth*header.shHeight);

	CBasicStream dec;	// zlibに渡す入力用のストリーム
	dec.OpenFromMemory(buf.GetPtr(), buf.GetSize());

	if(!Compress(&dec, &file, nCompLv)){
		return false;
	}

	return true;
}


} // end of namespace sueLib

