/**
	@file
	@brief		PNGファイルの入出力を行うクラス
	@note		<参考URL> http://www.kcrt.net/program/index.html
	@author		sue445
*/
#ifndef _INC_CPNGIO
#define _INC_CPNGIO

#include "CImageIO.h"
#include <png.h>

// fopen系で読み込むかどうか
//#define LIBPNG_STDIO

/*
zlib version 1.2.3：Copyright (C) 1995-2005 Jean-loup Gailly and Mark Adler
<URL> http://www.gzip.org/zlib/

libpng version 1.2.12：Copyright (c) 1998-2006 Glenn Randers-Pehrson
<URL> http://www.libpng.org/pub/png/
*/

// リリースコンパイル時
#ifndef _DEBUG
	#pragma comment( lib, "libpng.lib" )
	#pragma comment( lib, "zlib.lib" )
// デバッグコンパイル時
#else
	#pragma comment( lib, "libpngd.lib" )
	#pragma comment( lib, "zlibd.lib" )
#endif

namespace sueLib {

/// png_structとpng_infoを自動的に解放するクラス
class ktPngStructReleaser{
public:
	ktPngStructReleaser(png_structp p, png_infop q, bool write){
		m_p = p;
		m_q = q;
		m_bWrite = write;
	}
	~ktPngStructReleaser(){
		if(!m_bWrite)	png_destroy_read_struct( &m_p, &m_q, NULL);
		else			png_destroy_write_struct(&m_p, &m_q);
	}

private:
	png_structp	m_p;
	png_infop	m_q;
	bool m_bWrite;
};

/// PNGファイルの入出力を行うクラス
class CPngIO : virtual CImageIO {
private:

#ifndef LIBPNG_STDIO
	//=============================================================================
	/**
		PNG読み込み時に呼び出されるコールバック関数

		@param	png_ptr		[in] png_structへのポインタ
		@param	buf			[out] 読み込み先バッファ
		@param	size		[in] 読み込むサイズ
	*/
	//=============================================================================
	static void PngReadFunc(png_struct *png_ptr, png_bytep buf, png_size_t size)
	{
		CBasicStream* pFile = (CBasicStream*)png_get_io_ptr(png_ptr);
		pFile->Read(buf, size);
	}

	//=============================================================================
	/**
		PNG書き出し時に呼び出されるコールバック関数

		@param	png_ptr		[in] png_structへのポインタ
		@param	buf			[in] 書き出し元バッファ
		@param	size		[in] 書き出すサイズ
	*/
	//=============================================================================
	static void PngWriteFunc(png_struct *png_ptr, png_bytep buf, png_size_t size)
	{
		CBasicStream* pFile = (CBasicStream*)png_get_io_ptr(png_ptr);
		pFile->Write(buf, size);
	}
#endif

public:
	//=============================================================================
	/**
		画像の色数を取得

		@param	szFile			[in] ファイル名
		@param	bAlpha			[in] αチャンネルを展開するかどうか

		@retval	非0				画像の色数
		@retval	0				非対応
	*/
	//=============================================================================
	static int GetBitCount(const char* szFile, bool bAlpha=false)
	{
		char buf[8];
		png_structp	png_ptr;
		png_infop	info_ptr;

#ifdef LIBPNG_STDIO
		FILE* fp = fopen(szFile, "rb");
		if(!fp)									return false;
#else
		CBasicStream file;
		if(!file.OpenFromFile(szFile))			return false;	// ファイルが開けない
		file.Read(buf, sizeof(buf));
#endif

		if(!png_check_sig((png_bytep)buf, sizeof(buf)))	return 0;	// PNGではない

		if(!(png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL))){
			return 0;
		}
#ifdef LIBPNG_STDIO
		png_init_io(png_ptr, fp);
#endif

		if(!(info_ptr = png_create_info_struct(png_ptr))){
			png_destroy_read_struct( &png_ptr, NULL, NULL);
			return 0;
		}

		ktPngStructReleaser pngreleaser(png_ptr, info_ptr, false);

#ifdef LIBPNG_STDIO
		fseek(fp, 0, SEEK_SET);
#else
		// ファイルチェック用に一度読み込んでいるので、読み込みポインタを元に戻す
		file.SetSeek(0, SEEK_SET);

		// 読み込み用のコールバック関数をlibpngに教える
		png_set_read_fn(png_ptr,(png_voidp)&file,(png_rw_ptr)PngReadFunc);
#endif
		png_read_info(png_ptr, info_ptr);

		// アルファチャンネル無視
		if(!bAlpha)		png_set_strip_alpha(png_ptr);
		png_read_update_info(png_ptr, info_ptr);

		png_uint_32 PngWidth;
		png_uint_32 PngHeight;
		int bpp;
		int ColorType;
		png_get_IHDR(png_ptr, info_ptr, &PngWidth, &PngHeight, &bpp, &ColorType, NULL, NULL, NULL);

		int bitcount = png_get_channels(png_ptr, info_ptr) * bpp;
		// αチャンネルが含まれていれば32bits強制
		if(bAlpha && png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))		bitcount = 32;

#ifdef LIBPNG_STDIO
	fclose(fp);
#endif
		return bitcount;
	}

	//=============================================================================
	/**
		画像を読み込む

		@param	szFile			[in] ファイル名
		@param	img				[out] 出力先のCBasicImage
		@param	bAlpha			[in] αチャンネルを展開するかどうか

		@retval	true			画像の読み込みに成功
		@retval	false			画像の読み込みに失敗
	*/
	//=============================================================================
	template< typename PIXEL >
	static bool Load(const char* szFile, CBasicImage< PIXEL >& img, bool bAlpha=false)
	{
		char buf[8];
		png_structp	png_ptr;
		png_infop	info_ptr;
		unsigned int i;

#ifdef LIBPNG_STDIO
		FILE* fp = fopen(szFile, "rb");
		if(!fp)									return false;
		fread(buf, sizeof(buf), 1, fp);
#else
		CBasicStream file;
		if(!file.OpenFromFile(szFile))			return false;	// ファイルが開けない
		file.Read(buf, sizeof(buf));
#endif

		if(!png_check_sig((png_bytep)buf, sizeof(buf)))	return false;	// PNGではない

		if(!(png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL)))
			return false;

#ifdef LIBPNG_STDIO
		png_init_io(png_ptr, fp);
#endif

		if(!(info_ptr = png_create_info_struct(png_ptr))){
			png_destroy_read_struct( &png_ptr, NULL, NULL);
#ifdef LIBPNG_STDIO
			fclose(fp);
#endif
			return false;
		}

		ktPngStructReleaser pngreleaser(png_ptr, info_ptr, false);

#ifdef LIBPNG_STDIO
		fseek(fp, 0, SEEK_SET);
#else
		// ファイルチェック用に一度読み込んでいるので、読み込みポインタを元に戻す
		file.SetSeek(0, SEEK_SET);

		// 読み込み用のコールバック関数をlibpngに教える
		png_set_read_fn(png_ptr,(png_voidp)&file,(png_rw_ptr)PngReadFunc);
#endif


		png_read_info(png_ptr, info_ptr);

		png_uint_32 PngWidth;
		png_uint_32 PngHeight;
		int bpp;
		int ColorType;
		png_get_IHDR(png_ptr, info_ptr, &PngWidth, &PngHeight, &bpp, &ColorType, NULL, NULL, NULL);

		int bitcount = png_get_channels(png_ptr, info_ptr) * bpp;
		// αチャンネルが含まれていれば32bits強制
		if(png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)){
			int row = png_get_rowbytes(png_ptr, info_ptr);
			if(bAlpha/* && PngWidth*4==row*/){
				bitcount = 32;
			}
		}

		if(bitcount != img.GetBitCount()){		// 色数が違う
#ifdef LIBPNG_STDIO
			fclose(fp);
#endif
			return false;
		}
		if(PngWidth>65535 || PngHeight>65535){	// 画像が大きすぎる(ないと思うがｗ)
#ifdef LIBPNG_STDIO
			fclose(fp);
#endif
			return false;
		}

		// カラー配列をＲＧＢからＢＧＲに反転
		png_set_bgr(png_ptr);

		if(bAlpha && bitcount==32){
			// アルファチャンネルに関係する処理の設定
			if(png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))		png_set_expand(png_ptr);

			// １６ビットの画像でも８ビットで収納するように設定
			if(bpp > 8)												png_set_strip_16(png_ptr);
			if(ColorType == PNG_COLOR_TYPE_GRAY)					png_set_gray_to_rgb(png_ptr);

			if(sizeof(PIXEL)==4 && !(ColorType & PNG_COLOR_MASK_ALPHA))		png_set_filler(png_ptr, 0, 1);

			// パレット使用画像データの自動展開指定
			if(ColorType == PNG_COLOR_TYPE_PALETTE)					png_set_expand(png_ptr);

			// ８ビット以下のグレースケール画像を８ビット幅まで展開させるよう指定
			if(ColorType == PNG_COLOR_TYPE_GRAY && bpp <= 8)		png_set_expand(png_ptr);
		}
		else{
			if(ColorType & PNG_COLOR_MASK_ALPHA)		png_set_strip_alpha(png_ptr);
		}


		// 変更した設定を反映させる
		png_read_update_info(png_ptr, info_ptr);

		img.Create((unsigned short)PngWidth, (unsigned short)PngHeight);

		// パレット情報の読み込み
		for(i = 0; i < img.GetPalSize(); i++){
			img.SetPalette(i, info_ptr->palette[i].red, info_ptr->palette[i].green, info_ptr->palette[i].blue);
		}

		png_bytep *lines = new png_bytep[img.GetHeight()];
		PIXEL* p = img.Top();
		for(i = 0; i < img.GetHeight(); i++){
			lines[i] = (png_bytep)p;
			p = img.NextLine();
		}
		// 画像データの読み込み
		png_read_image(png_ptr, lines);

		// 読み込み処理の終了
		png_read_end(png_ptr, info_ptr);
		delete[] lines;

#ifdef LIBPNG_STDIO
	fclose(fp);
#endif
		return true;
	}	///< 画像を読み込む


	//=============================================================================
	/**
		画像を書き出す

		@param	szFile			[in] ファイル名
		@param	img				[in] 入力元のCBasicImage
		@param	nCompLv			[in] 保存時の圧縮レベル(0(無圧縮)～9(最高圧縮), -1で標準圧縮)
		@param	nFilter			[in] 保存時のフィルタ

		@retval	true			画像の書き出しに成功
		@retval	false			画像の書き出しに失敗
	*/
	//=============================================================================
	template< typename PIXEL >
	static bool Save(const char* szFile, const CBasicImage< PIXEL >& img, int nCompLv=-1, int nFilter=PNG_ALL_FILTERS)
	{
		unsigned int i;
		png_structp	png_ptr;
		png_infop	info_ptr;

		if(!img.IsInit())						return false;	// 初期化されてない

#ifdef LIBPNG_STDIO
		FILE* fp = fopen(szFile, "wb");
		if(!fp)									return false;
#else
		CBasicStream file;
		if(!file.OpenFromFile(szFile, true))	return false;	// ファイルが開けない
#endif

		if(!(png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL))) return false;
		if(!(info_ptr = png_create_info_struct(png_ptr))){
			png_destroy_write_struct(&png_ptr, png_infopp_NULL);
			return false;
		}
		ktPngStructReleaser pngreleaser(png_ptr, info_ptr, true);

		if(setjmp(png_jmpbuf(png_ptr))){
			// エラーの時ここに来る。
			return false;
		}

#ifdef LIBPNG_STDIO
		png_init_io(png_ptr, fp);                              // libpngにfpを知らせます
#else
		// 書き出し用のコールバック関数をlibpngに教える(flush関数についてはよく分からないので指定しない)
		png_set_write_fn(png_ptr,(png_voidp)&file,(png_rw_ptr)PngWriteFunc, NULL);
#endif

		// 保存時のフィルタと圧縮レベルを設定
		png_set_filter(png_ptr, 0, nFilter);
		png_set_compression_level(png_ptr, nCompLv);

		// パレット情報の書き込み
		if(img.GetPalSize()>0){
			png_color* plt = new png_color[img.GetPalSize()];
			if(!plt)				return false;	// メモリの確保に失敗

			for(i = 0; i < img.GetPalSize(); i++){
				img.GetPalette(i, &plt[i].red, &plt[i].green, &plt[i].blue);
			}
			png_set_PLTE(png_ptr, info_ptr, plt, img.GetPalSize());

			delete[] plt;
		}

		int nColorType;
		if(img.GetPalSize()>0)		nColorType = PNG_COLOR_TYPE_PALETTE;
		else if(sizeof(PIXEL)==4)	nColorType = PNG_COLOR_TYPE_RGB_ALPHA;
		else						nColorType = PNG_COLOR_TYPE_RGB;
		// IHDRチャンク情報を設定します
		png_set_IHDR(png_ptr, info_ptr, img.GetWidth(), img.GetHeight(), 8,
						nColorType, PNG_INTERLACE_NONE,
						PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

		/*
		// PNGに書き込まれるコメント
		png_text text_ptr[1];
		text_ptr[0].key = "Description";
		text_ptr[0].text = "ktcDIB::Save() Data";
		text_ptr[0].compression = PNG_TEXT_COMPRESSION_NONE;
		png_set_text(png_ptr, info_ptr, text_ptr, 1);
		*/

		png_write_info(png_ptr, info_ptr);	// PNGファイルのヘッダを書き込みます

		png_bytep *lines = new png_bytep[img.GetHeight()];
		const PIXEL* p = img.GetPtr(0, 0);
		for(i = 0; i < img.GetHeight(); i++) {
			lines[i] = (png_bytep)p;
			p += img.GetWidth();
		}
		png_set_bgr(png_ptr);
		png_write_image(png_ptr, lines);		// 画像データを書き込みます

		png_write_end(png_ptr, info_ptr);		// 残りの情報を書き込みます

		delete[] lines;

#ifdef LIBPNG_STDIO
		fclose(fp);
#endif
		return true;
	}	///< 画像を書き出す
};

} // end of namespace sueLib


#endif
