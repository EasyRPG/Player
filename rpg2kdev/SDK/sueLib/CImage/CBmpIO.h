/**
	@file
	@brief		BMPファイルの入出力を行うクラス
	@author		sue445
*/
#ifndef _INC_CBMPIO
#define _INC_CBMPIO

#include "CImageIO.h"


namespace sueLib {

/// BMPファイルの入出力を行うクラス
class CBmpIO : virtual CImageIO {
private:

#pragma pack(push)
#pragma pack(1) //構造体のメンバ境界を1バイトにする
	/// ビットマップファイルヘッダ
	typedef struct tagBITMAP_FILE_HEADER {
		unsigned short	bfType;			///< "BM"
		unsigned long	bfSize;			///< ファイルサイズ
		unsigned short	bfReserved1;	///< 予約 Zero
		unsigned short	bfReserved2;	///< 予約 Zero
		unsigned long	bfOffBits;		///< 先頭からイメージデータまでのオフセット
	} BITMAPFILEHEADER;

	// ========= WIN BMP/DIB FORMAT
	/// WINイメージデータ情報
	typedef struct tagBITMAP_INFO_HEADER {
		unsigned long	biSize;				///< この構造体のサイズ
		long			biWidth;			///< 画像の横幅
		long			biHeight;			///< 画像の高さ
		unsigned short	biPlanes;			///< 予約 1
		unsigned short	biBitCount;			///< 色数
		unsigned long	biCompression;		///< 圧縮の種類
		unsigned long	biSizeImage;		///< イメージのバイト数
		long			biXPelsPerMeter;	///< 水平解像度
		long			biYPelsPerMeter;	///< 垂直解像度
		unsigned long	biClrUsed;			///< 使用するカラーインデックスの数
		unsigned long	biClrImportant;		///< 減色しないカラーインデックス数
	} BITMAPINFOHEADER;

	/*
	/// WINカラーテーブル要素
	typedef struct tagRGBQUAD {
		unsigned char		rgbBlue;		///< 青
		unsigned char		rgbGreen;		///< 緑
		unsigned char		rgbRed;			///< 赤
		unsigned char		rgbReserved;	///< 予約
	} RGBQUAD;
	*/

	/// WINヘッダ情報
	typedef struct tagBITMAP_INFO {
		BITMAPINFOHEADER	bmiHeader;		///< ビットマップヘッダ
		RGBQUAD	bmiColors[1];				///< カラーテーブルの一番目
	} BITMAPINFO;

	// ========= OS/2 BMP/DIB FORMAT
	/// OS/2 イメージデータ情報
	typedef struct tagBITMAP_CORE_HEADER {
		unsigned long	biSize;			///< この構造体のサイズ
		unsigned short	biWidth;		///< 画像の横幅
		unsigned short	biHeight;		///< 画像の高さ
		unsigned short	biPlanes;		///< 予約 1
		unsigned short	biBitCount;		///< 色数
	} BITMAPCOREHEADER;

	/*
	/// OS/2 カラーテーブル要素
	typedef struct tagRGBTRIPLE {
		unsigned char		rgbBlue;	///< 青
		unsigned char		rgbGreen;	///< 緑
		unsigned char		rgbRed;		///< 赤
	} RGBTRIPLE;
	*/

	/// OS/2 ヘッダ情報
	typedef struct tagBITMAP_CORE_INFO {
		BITMAPCOREHEADER	bmiHeader;		///< ビットマップヘッダ
		RGBTRIPLE			bmiColors[1];	///< カラーテーブルの一番目
	} BITMAPCOREINFO;
#pragma pack(pop)

public:
	//=============================================================================
	/**
		画像の色数を取得

		@param	szFile			[in] ファイル名

		@retval	非0				画像の色数
		@retval	0				非対応
	*/
	//=============================================================================
	static int GetBitCount(const char* szFile)
	{
		CBasicStream file;
		BITMAPFILEHEADER header;
		BITMAPINFOHEADER info;
		BITMAPCOREHEADER core;


		if(!file.OpenFromFile(szFile))		return 0;	// ファイルが開けない

		file.Read(&header, sizeof(header));
		if(header.bfType!=0x4D42)			return 0;	// ヘッダが一致しない

		file.Read(&info, sizeof(info));
		// WIN
		if(info.biSize==sizeof(BITMAPINFOHEADER)){
			return info.biBitCount;
		}
		// OS/2
		else if(info.biSize==sizeof(BITMAPCOREHEADER)){
			// WINではなかったので読み込みしなおす
			file.SetSeek(sizeof(BITMAPFILEHEADER), SEEK_SET);
			file.Read(&core, sizeof(core));

			return core.biBitCount;
		}

		return 0;
	}

	//=============================================================================
	/**
		画像を読み込む

		@param	szFile			[in] ファイル名
		@param	img				[out] 出力先のCBasicImage

		@retval	true			画像の読み込みに成功
		@retval	false			画像の読み込みに失敗
	*/
	//=============================================================================
	template< typename PIXEL >
	static bool Load(const char* szFile, CBasicImage< PIXEL >& img)
	{
		CBasicStream file;
		BITMAPFILEHEADER header;
		BITMAPINFOHEADER info;
		BITMAPCOREHEADER core;
		unsigned int i;


		if(!file.OpenFromFile(szFile))		return false;	// ファイルが開けない

		file.Read(&header, sizeof(header));
		if(header.bfType!=0x4D42)			return false;	// ヘッダが一致しない

		file.Read(&info, sizeof(info));
		// WIN
		if(info.biSize==sizeof(BITMAPINFOHEADER)){
			if(info.biBitCount!=img.GetBitCount())			return false;	// 色数が違う
			if(info.biWidth>65535 || info.biHeight>65535)	return false;	// 画像が大きすぎる(ないと思うがｗ)

			img.Create((unsigned short)info.biWidth, (unsigned short)info.biHeight);

			// パレットを読み込む
			RGBQUAD rgb;
			for(i = 0; i < img.GetPalSize(); i++){
				file.Read(&rgb, sizeof(rgb));
				img.SetPalette(i, rgb.rgbRed, rgb.rgbGreen, rgb.rgbBlue);
			}
		}
		// OS/2
		else if(info.biSize==sizeof(BITMAPCOREHEADER)){
			// WINではなかったので読み込みしなおす
			file.SetSeek(sizeof(BITMAPFILEHEADER), SEEK_SET);
			file.Read(&core, sizeof(core));

			if(core.biBitCount!=img.GetBitCount())			return false;	// 色数が違う

			img.Create(core.biWidth, core.biHeight);

			// パレットを読み込む
			RGBTRIPLE rgb;
			for(i = 0; i < img.GetPalSize(); i++){
				file.Read(&rgb, sizeof(rgb));
				img.SetPalette(i, rgb.rgbRed, rgb.rgbGreen, rgb.rgbBlue);
			}
		}
		else{
			return false;
		}

		// 念のために自分で読み込み位置をセットする
		file.SetSeek(header.bfOffBits, SEEK_SET);

		// BMPファイルは左下のピクセルから始まっているので、逆から格納する
		PIXEL* p = img.Locate(0, img.GetHeight()-1);
		const unsigned int linesize = sizeof(PIXEL) * img.GetWidth();	// 1行分のデータ長
		const unsigned int lineskip = (4 - linesize % 4) % 4;
		for(i = 0; i < img.GetHeight(); i++){
			file.Read(p, linesize);
			p = img.PrevLine();

			// 4バイト境界
			if(lineskip>0)	file.SetSeek(lineskip, SEEK_CUR);
		}

		return true;
	}	///< 画像を読み込む


	//=============================================================================
	/**
		画像を書き出す

		@param	szFile			[in] ファイル名
		@param	img				[in] 入力元のCBasicImage

		@retval	true			画像の書き出しに成功
		@retval	false			画像の書き出しに失敗
	*/
	//=============================================================================
	template< typename PIXEL >
	static bool Save(const char* szFile, const CBasicImage< PIXEL >& img)
	{
		CBasicStream file;
		BITMAPFILEHEADER header;
		BITMAPINFOHEADER info;
		unsigned int i;


		if(!img.IsInit())						return false;	// 初期化されてない

		if(!file.OpenFromFile(szFile, true))	return false;	// ファイルが開けない

		// BITMAPINFOHEADER
		info.biSize				= sizeof(BITMAPINFOHEADER);
		info.biWidth			= img.GetWidth();
		info.biHeight			= img.GetHeight();
		info.biPlanes			= 1;
		info.biBitCount			= img.GetBitCount();
		info.biCompression		= 0;
		info.biSizeImage		= 0;
		info.biXPelsPerMeter	= 0;
		info.biYPelsPerMeter	= 0;
		info.biClrUsed			= img.GetPalSize();
		info.biClrImportant		= 0;

		// BITMAPFILEHEADER
		header.bfType			= 0x4D42;
		header.bfSize			= sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + ((img.GetWidth() * sizeof(PIXEL) +3) & ~3)*img.GetHeight() + sizeof(RGBQUAD)*img.GetPalSize();
		header.bfReserved1		= 0;
		header.bfReserved2		= 0;
		header.bfOffBits		= sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD)*img.GetPalSize();


		file.Write(&header, sizeof(header));
		file.Write(&info  , sizeof(info));

		RGBQUAD rgb = {0};
		for(i = 0; i < img.GetPalSize(); i++){
			img.GetPalette(i, &rgb.rgbRed, &rgb.rgbGreen, &rgb.rgbBlue);
			file.Write(&rgb, sizeof(rgb));
		}

		// BMPファイルは左下のピクセルから始まっているので、逆から格納する
		const PIXEL* p = img.GetPtr(0, img.GetHeight()-1);
		const unsigned int linesize = sizeof(PIXEL) * img.GetWidth();	// 1行分のデータ長
		const unsigned int lineskip = (4 - linesize % 4) % 4;
		const unsigned char tmp[3] = {0, 0, 0};
		for(i = 0; i < img.GetHeight(); i++){
			file.Write(p, linesize);
			p -= img.GetWidth();

			// 4バイト境界
			if(lineskip>0)	file.Write(tmp, lineskip);
		}

		return true;
	}	///< 画像を書き出す
};

} // end of namespace sueLib

#endif
