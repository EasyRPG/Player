/**
	@file
	@brief		画像ファイル入出力の基本となるクラス
	@author		sue445
*/
#ifndef _INC_CIMAGEIO
#define _INC_CIMAGEIO

#include "../sueLib_Compile.h"
#include "CImage.h"
#include "../CBasicStream.h"


namespace sueLib {

/// 画像ファイル入出力の基本となるクラス
class CImageIO{
public:
	template< typename PIXEL >
	static bool Load(const char* szFile, CBasicImage< PIXEL >& img){ return false; }	///< 画像を読み込む

	template< typename PIXEL >
	static bool Save(const char* szFile, const CBasicImage< PIXEL >& img){ return false; }	///< 画像を書き出す

	static int GetBitCount(const char* szFile){ return 0; }								///< 画像の色数を取得
};

} // end of namespace sueLib

#endif
