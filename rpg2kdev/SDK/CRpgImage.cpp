/**
	@file
	@brief		bmp,png,xyzを自動的に判別して読み込んだりするクラス
	@note		ファイル名に拡張子が不要なのは妙かもしれませんが、ツクール内部で格納されている
				ピクチャなどのファイル名は全て拡張子が含まれていないのでその仕様に合わせています。
	@author		sue445
*/
#include "CRpgImage.h"
#include <string>
#include "sueLib/CImage/CBmpIO.h"
#include "sueLib/CImage/CPngIO.h"
#include "sueLib/CImage/CXyzIO.h"


using std::string;
using namespace sueLib;

//=============================================================================
/**
	画像を読み込む

	@param	szFile			[in] ファイル名(拡張子は不要)

	@retval	true			画像の読み込みに成功
	@retval	false			画像の読み込みに失敗
*/
//=============================================================================
bool CRpgImage::Load(const char* szFile)
{
	string str;


	// bmp
	str = szFile;
	str += ".bmp";
	if(CBmpIO::Load(str.c_str(), *this))		return true;

	// png
	str = szFile;
	str += ".png";
	if(CPngIO::Load(str.c_str(), *this, false))	return true;

	// xyz
	str = szFile;
	str += ".xyz";
	if(CXyzIO::Load(str.c_str(), *this))		return true;

	return false;
}

//=============================================================================
/**
	画像を保存する

	@param	eSaveType		[in] 保存形式
	@param	szFile			[in] ファイル名(拡張子は不要)

	@retval	true			画像の保存に成功
	@retval	false			画像の保存に失敗
*/
//=============================================================================
bool CRpgImage::Save(const char* szFile, eRpgImage eSaveType) const
{
	string str = szFile;


	switch(eSaveType){
	// bmp
	case IMG_BMP:
		str += ".bmp";
		return CBmpIO::Save(str.c_str(), *this);
	// png
	case IMG_PNG:
		str += ".png";
		return CPngIO::Save(str.c_str(), *this);
	// xyz
	case IMG_XYZ:
		str += ".xyz";
		return CXyzIO::Save(str.c_str(), *this);
	}

	return false;
}
