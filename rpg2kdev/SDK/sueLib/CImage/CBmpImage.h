/**
	@file
	@brief		CBasicImageをHBITMAPに変換する
	@author		sue445
*/
#ifndef _INC_CBMPIMAGE
#define _INC_CBMPIMAGE

#include <windows.h>
#include "CImage.h"
#include "../smart_buffer.h"

namespace sueLib {

/*
/// アルファチャンネルの無効化(32bits以外では実行しない)
template< typename PIXEL > struct AlphaDisable;

template<>
struct AlphaDisable< unsigned long >{
	AlphaDisable(CBasicImage< unsigned long >& img)
	{
		unsigned long* p = img.Top();
		while(!img.IsEof()){
			if((*p & 0xFF000000)==0)		*p = 0x00FFFFFF;
			p = img.Next();
		}
	}
};
template< typename PIXEL >
struct AlphaDisable{
	AlphaDisable(CBasicImage< PIXEL >& img){}
};
*/

template< typename PIXEL >
class CBmpImage : public CBasicImage< PIXEL >{
private:
	/// 上下反転(4バイト補正有り)
	void MirrorY(CBasicImage< PIXEL >& imgDst, CBasicImage< PIXEL >& imgSrc)
	{
		imgDst.Create(((imgSrc.GetWidth()*sizeof(PIXEL)+3) & ~3)/sizeof(PIXEL), imgSrc.GetHeight());
		imgDst.Locate(0, 0);					// 上から下へ
		imgSrc.Locate(0, imgSrc.GetHeight()-1);	// 下から上へ
		DWORD widthsize = imgSrc.GetWidth()*sizeof(PIXEL);
		while(!imgDst.IsEof()){
			memcpy(imgDst.Index(), imgSrc.Index(), widthsize);
			imgDst.NextLine();
			imgSrc.PrevLine();
		}
	}
	void AlphaDisable(CImage32& img)
	{
		unsigned long* p = img.Top();
		while(!img.IsEof()){
			if((*p & 0xFF000000)==0)		*p = 0x00FFFFFF;
			p = img.Next();
		}
	}
	void AlphaDisable(CImage24& img){}
	void AlphaDisable(CImage16& img){}
	void AlphaDisable(CImage8& img){}

public:
	/// デフォルトコンストラクタ
	CBmpImage(ImgLength width=0, ImgLength height=0) : CBasicImage< PIXEL >(width,height) {}

	/// コピーコンストラクタ
	CBmpImage(const CBasicImage< PIXEL >& obj){ operator=(obj); }

	/// 代入コンストラクタ
	CBmpImage& operator=(const CBasicImage< PIXEL >& obj)
	{
		ImgLength x, y;
		unsigned char r, g, b;
		unsigned short i;

		// CBasicImage内でfriendしてもprivateにアクセスできない(汗
		Create(obj.GetWidth(), obj.GetHeight());

		for(y = 0; y < GetHeight(); y++){
			for(x = 0; x < GetWidth(); x++){
				SetPixel(x, y, obj.GetPixel(x, y));
			}
		}

		CopyPalette(obj);

		return *this;
	}

	/// ビットマップを取得
	HBITMAP GetBitmap(HWND hWnd, int nLeft=0, int nTop=0, int nWidth=0, int nHeight=0)
	{
		CBasicImage< PIXEL > imgTemp;	// 4バイト補正したDIBデータ
		BITMAPINFO* pinfo = (BITMAPINFO*)malloc( sizeof(BITMAPINFOHEADER)+GetPalSize()*sizeof(RGBQUAD) );

		if(nLeft<0)		nLeft = 0;
		if(nTop<0)		nTop = 0;
		if(nWidth<=0  || nWidth>GetWidth()-nLeft)	nWidth  = GetWidth() - nLeft;
		if(nHeight<=0 || nHeight>GetHeight()-nTop)	nHeight = GetHeight() - nTop;

		if(nLeft==0 && nTop==0 && nWidth==GetWidth() && nHeight==GetHeight()){
			MirrorY(imgTemp, *this);
			AlphaDisable(imgTemp);
		}
		else{
			CBasicImage< PIXEL > img;	// 短径選択用
			img.Create(nWidth, nHeight);
			Locate(nLeft, nTop);
			DWORD widthsize = nWidth * sizeof(PIXEL);
			for(int i = 0; i < nHeight; i++){
				memcpy(img.Index(), Index(), widthsize);
				NextLine();
				img.NextLine();
			}
			MirrorY(imgTemp, img);
			AlphaDisable(imgTemp);
		}

		// ビットマップ情報を作成
		ZeroMemory(pinfo, sizeof(BITMAPINFO));
		pinfo->bmiHeader.biSize				= sizeof(BITMAPINFOHEADER);
		pinfo->bmiHeader.biWidth			= nWidth;
		pinfo->bmiHeader.biHeight			= nHeight;
		pinfo->bmiHeader.biPlanes			= 1;
		pinfo->bmiHeader.biBitCount			= (WORD)GetBitCount();
		pinfo->bmiHeader.biCompression		= BI_RGB;
		pinfo->bmiHeader.biSizeImage		= imgTemp.GetWidth()*imgTemp.GetHeight()*sizeof(PIXEL);
		pinfo->bmiHeader.biXPelsPerMeter	= 0;
		pinfo->bmiHeader.biYPelsPerMeter	= 0;
		pinfo->bmiHeader.biClrUsed			= 0;
		pinfo->bmiHeader.biClrImportant		= 0;

		// パレットをコピー
		if(GetPalSize()>0){
			int max = GetPalSize();
			RGBQUAD* rgb = (RGBQUAD*)&pinfo->bmiColors[0];
			for(int i = 0; i < max; i++){
				GetPalette(i, &rgb->rgbRed, &rgb->rgbGreen, &rgb->rgbBlue);
				rgb++;
			}
		}

		PAINTSTRUCT	ps;
		HDC			hdc = BeginPaint(hWnd, &ps);

		HBITMAP hRet = CreateDIBitmap(hdc, &pinfo->bmiHeader, CBM_INIT, imgTemp.Top(), pinfo, DIB_RGB_COLORS);
		EndPaint(hWnd, &ps);

//		free(pinfo);
		return hRet;
	}
};

typedef CBmpImage< unsigned char  >			CBmp8;	///< 8bits  color only
typedef CBmpImage< unsigned short >			CBmp16;	///< 16bits color only
typedef CBmpImage< sueLib::RGBTRIPLE >		CBmp24;	///< 24bits color only
//typedef CBmpImage< RGBTRIPLE >			CBmp24;	///< 24bits color only
typedef CBmpImage< unsigned long  >			CBmp32;	///< 32bits color only

} // end of namespace sueLib

#endif
