/**
	@file
	@brief		XYZファイルの入出力を行うクラス
	@author		sue445
*/
#ifndef _INC_CXYZIO
#define _INC_CXYZIO

#include "CImageIO.h"
#include "../CZLib.h"


namespace sueLib {

/// XYZファイルの入出力を行うクラス
class CXyzIO : virtual CImageIO, private CZLib {
private:
	/// XYZのファイルヘッダ
	struct XYZ_FILEHEADER{
		unsigned char  cID[4];		// 識別用ID(XYZ1)
		unsigned short shWidth;		// 画像の幅
		unsigned short shHeight;	// 画像の高さ
	};
	/// XYZのパレット情報
	struct XYZ_RGB{
		unsigned char red;
		unsigned char green;
		unsigned char blue;
	};

public:
	static bool Load(const char* szFile, CImage8& img);					///< 画像を読み込む
	static bool Save(const char* szFile, const CImage8& img, int nCompLv=-1);	///< 画像を書き出す
	static int  GetBitCount(const char* szFile)							///< 画像の色数を取得
	{
		CImage8 img;
		return (Load(szFile,img)) ? 8 : 0;
	}
};

} // end of namespace sueLib


#endif
