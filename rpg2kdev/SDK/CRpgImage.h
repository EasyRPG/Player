/**
	@file
	@brief		bmp,png,xyzを自動的に判別して読み込んだりするクラス
	@note		ファイル名に拡張子が不要なのは妙かもしれませんが、ツクール内部で格納されている
				ピクチャなどのファイル名は全て拡張子が含まれていないのでその仕様に合わせています。
	@author		sue445
*/
#ifndef _INC_CRPGIMAGE
#define _INC_CRPGIMAGE

#include "sueLib/CImage/CImage.h"

using namespace sueLib;

/// ツクールで扱える画像形式
enum eRpgImage{
	IMG_BMP,	///< BMP
	IMG_PNG,	///< PNG
	IMG_XYZ,	///< XYZ
};

/// bmp,png,xyzを自動的に判別して読み込んだりするクラス
class CRpgImage : public CImage8{
public:
	bool Load(const char* szFile);								///< 読み込む
	bool Save(const char* szFile, eRpgImage eSaveType) const;	///< 保存する
};

#endif