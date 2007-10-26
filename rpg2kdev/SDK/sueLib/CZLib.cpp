/**
	@file
	@brief		ZLib compression class
	@note		<Reference URL> http://oku.edu.mie-u.ac.jp/~okumura/compression/zlib.html
	@author		sue445
*/
#include "CZLib.h"
#include <zlib.h>

/*
zlib Ver1.23：Copyright (C) 1995-2005 Jean-loup Gailly and Mark Adler
<URL> http://www.gzip.org/zlib/
*/
// リリースコンパイル時
#ifndef _DEBUG
	#pragma comment( lib, "zlib.lib" )
// デバッグコンパイル時
#else
	#pragma comment( lib, "zlibd.lib" )
#endif

//#define ERROR_PRINT			// printfによるエラー出力を行うかどうか


namespace sueLib {

//static const int nInBufSize		= 1024*10;		// 入力バッファサイズ（任意）
//static const int nOutBufSize	= 1040*10;		// 出力バッファサイズ（≧上×1.001＋12）
static const int nInBufSize		= 1024*32;		// 入力バッファサイズ（任意）
static const int nOutBufSize	= (int)(nInBufSize * 1.001) + 12;		// 出力バッファサイズ（≧上×1.001＋12）

//=============================================================================
/**
	zlibで展開

	@param	pstInput		[in] 入力ストリーム
	@param	pstOutput		[out] 出力ストリーム(NULLを指定すれば書き出しを行わず、必要サイズの計算のみを行う)
	@param	lpSize			[out] 展開後サイズの取得先(NULLで取得しない)

	@retval	true			成功
	@retval	false			失敗
*/
//=============================================================================
bool CZLib::Decompress(CBasicStream* pstInput, CBasicStream* pstOutput, unsigned int* lpSize)
{
	z_stream z; 							// ライブラリとやりとりするための構造体
	unsigned char inbuf[nInBufSize];		// 入力バッファ
	unsigned char outbuf[nOutBufSize]; 		// 出力バッファ
	int count, status;
	unsigned int outsize = 0;				// 書き出したサイズ(戻り値用)

	if(!pstInput || !pstInput->IsOpened() || pstInput->IsWriteMode())		return false;	// 入力ストリームが初期化されていない
	if(pstOutput && (!pstOutput->IsOpened() || !pstOutput->IsWriteMode()))	return false;	// 出力ストリームが不正


	// すべてのメモリ管理をライブラリに任せる
	z.zalloc = Z_NULL;	// 内部のメモリ確保に使われます
	z.zfree  = Z_NULL;	// 内部のメモリ解放に使われます
	z.opaque = Z_NULL;	// zalloc と zfree に渡される プライベートデータ・オブジェクト

	// 初期化
	z.next_in = Z_NULL;
	z.avail_in = 0;

	if(inflateInit(&z) != Z_OK){
#ifdef ERROR_PRINT
		printf("inflateInit: %s\n", (z.msg) ? z.msg : "???");
#endif
		return false;
	}

	z.next_out  = outbuf;		// 出力ポインタ
	z.avail_out = nOutBufSize;	// 出力バッファ残量
	status = Z_OK;

	while(status != Z_STREAM_END){
		if(z.avail_in == 0){	// 入力残量がゼロになれば 
			z.next_in  = inbuf;	// 入力ポインタを元に戻す
			z.avail_in = pstInput->Read(inbuf, nInBufSize);
		}
		status = inflate(&z, Z_NO_FLUSH);	// 展開
		if(status == Z_STREAM_END){			// 完了
			break;
		}
		else if(status != Z_OK){			// エラー
#ifdef ERROR_PRINT
			printf("inflate: %s\n", (z.msg) ? z.msg : "???");
#endif
			return false;
		}
		if(z.avail_out == 0){				// 出力バッファが尽きれば
			// まとめて書き出す
			if(pstOutput)	outsize += pstOutput->Write(outbuf, nOutBufSize);
			else			outsize += nOutBufSize;

			z.next_out  = outbuf;			// 出力ポインタを元に戻す
			z.avail_out = nOutBufSize;		// 出力バッファ残量を元に戻す
		}
	}

	// 残りを吐き出す
	if((count = nOutBufSize - z.avail_out) != 0){
		if(pstOutput)	outsize += pstOutput->Write(outbuf, count);
		else			outsize += count;
	}

	// 後始末
	if(inflateEnd(&z) != Z_OK){
#ifdef ERROR_PRINT
		printf("inflateEnd: %s\n", (z.msg) ? z.msg : "???");
#endif
		return false;
	}

	if(lpSize)		*lpSize = outsize;
	return true;
}

//=============================================================================
/**
	zlibで圧縮

	@param	pstInput		[in] 入力ストリーム
	@param	pstOutput		[out] 出力ストリーム(NULLを指定すれば書き出しを行わず、必要サイズの計算のみを行う)
	@param	lpSize			[out] 圧縮後サイズの取得先(NULLで取得しない)
	@param	nCompLv			[in] 保存時の圧縮レベル(0(無圧縮)～9(最高圧縮), -1で標準圧縮)

	@retval	true			成功
	@retval	false			失敗
*/
//=============================================================================
bool CZLib::Compress(CBasicStream* pstInput, CBasicStream* pstOutput, int nCompLv, unsigned int* lpSize)
{
	z_stream z; 							// ライブラリとやりとりするための構造体
	unsigned char inbuf[nInBufSize];		// 入力バッファ
	unsigned char outbuf[nOutBufSize]; 		// 出力バッファ
	int count, flush, status;
	unsigned int outsize = 0;				// 書き出したサイズ(戻り値用)

	if(!pstInput || !pstInput->IsOpened() || pstInput->IsWriteMode())		return false;	// 入力ストリームが初期化されていない
	if(pstOutput && (!pstOutput->IsOpened() || !pstOutput->IsWriteMode()))	return false;	// 出力ストリームが不正
	if(nCompLv<-1 || nCompLv>9)		return false;	// 圧縮レベルがおかしい


	// すべてのメモリ管理をライブラリに任せる
	z.zalloc = Z_NULL;	// 内部のメモリ確保に使われます
	z.zfree  = Z_NULL;	// 内部のメモリ解放に使われます
	z.opaque = Z_NULL;	// zalloc と zfree に渡される プライベートデータ・オブジェクト


	// 初期化
	// 第2引数は圧縮の度合。0～9 の範囲の整数で，0 は無圧縮
	// Z_DEFAULT_COMPRESSION (= 6) が標準
//	/*
	if(deflateInit(&z, nCompLv) != Z_OK){
#ifdef ERROR_PRINT
		printf("deflateInit: %s\n", (z.msg) ? z.msg : "???");
#endif
		return false;
	}
//	*/


	/*
//	z.next_in = Z_NULL;	// 入力ポインタを入力バッファの先頭に
	if(deflateInit2(&z, nCompLv, Z_DEFLATED, 15, 9, Z_DEFAULT_STRATEGY) != Z_OK){
#ifdef ERROR_PRINT
		printf("deflateInit2: %s\n", (z.msg) ? z.msg : "???");
#endif
		return false;
	}
	*/

	z.avail_in = 0; 			// 入力バッファ中のデータのバイト数
	z.next_out = outbuf;		// 出力ポインタ
	z.avail_out = nOutBufSize;	// 出力バッファのサイズ

	// 通常は deflate() の第2引数は Z_NO_FLUSH にして呼び出す
	flush = Z_NO_FLUSH;

	while(1){
		if(z.avail_in == 0){	// 入力が尽きれば
			z.next_in = inbuf;	// 入力ポインタを入力バッファの先頭に
			z.avail_in = pstInput->Read(inbuf, nInBufSize);

			// 入力が最後になったら deflate() の第2引数は Z_FINISH にする
			if(z.avail_in < nInBufSize)		flush = Z_FINISH;
		}
		status = deflate(&z, flush);	// 圧縮する
		if(status == Z_STREAM_END){		// 完了
			break;
		}
		else if(status != Z_OK){		// エラー
#ifdef ERROR_PRINT
			printf("deflate: %s\n", (z.msg) ? z.msg : "???");
#endif
			return false;
		}
		if(z.avail_out == 0){			// 出力バッファが尽きれば
			// まとめて書き出す
			if(pstOutput)	outsize += pstOutput->Write(outbuf, nOutBufSize);
			else			outsize += nOutBufSize;

			z.next_out = outbuf;		// 出力バッファ残量を元に戻す
			z.avail_out = nOutBufSize;	// 出力ポインタを元に戻す
		}
	}

	// 残りを吐き出す
	if((count = nOutBufSize - z.avail_out) != 0){
		if(pstOutput)	outsize += pstOutput->Write(outbuf, count);
		else			outsize += count;
	}

	// 後始末
	if(deflateEnd(&z) != Z_OK){
#ifdef ERROR_PRINT
		printf("deflateEnd: %s\n", (z.msg) ? z.msg : "???");
#endif
		return false;
	}

	if(lpSize)	*lpSize = outsize;
	return true;
}

//=============================================================================
/**
	メモリ展開

	@param	bufSrc		[in] 入力元バッファ(圧縮データ)
	@param	bufDst		[out] 出力先バッファ(オリジナルデータ)

	@retval	true		成功
	@retval	false		失敗
*/
//=============================================================================
bool CZLib::MemDecompress(const smart_buffer& bufSrc, smart_buffer& bufDst)
{
	CBasicStream stSrc, stDst;
	unsigned int nSize;

	// 入力ストリームの初期化に失敗
	if(!stSrc.OpenFromMemory(bufSrc.GetConstPtr(), bufSrc.GetSize(), false))	return false;

	// 展開後のサイズの取得に失敗
	if(!Decompress(&stSrc, NULL, &nSize))			return false;

	if(nSize>0){
		bufDst.Resize(nSize);

		// 出力ストリームの初期化に失敗
		if(!stDst.OpenFromMemory(bufDst.GetPtr(), bufDst.GetSize(), true))	return false;

		// 入力ストリームを巻き戻す
		stSrc.SetSeek(0, SEEK_SET);

		// 展開に失敗
		if(!Decompress(&stSrc, &stDst))					return false;
	}
	else{
		bufDst.Release();
	}

	return true;
}

//=============================================================================
/**
	メモリ圧縮

	@param	bufSrc		[in] 入力元バッファ(オリジナルデータ)
	@param	bufDst		[out] 出力先バッファ(圧縮データ)
	@param	nCompLv		[in] 保存時の圧縮レベル(0(無圧縮)～9(最高圧縮), -1で標準圧縮)

	@retval	true		成功
	@retval	false		失敗
*/
//=============================================================================
bool CZLib::MemCompress(const smart_buffer& bufSrc, smart_buffer& bufDst, int nCompLv)
{
	CBasicStream stSrc, stDst;
	unsigned int nSize;

	// 入力ストリームの初期化に失敗
	if(!stSrc.OpenFromMemory(bufSrc.GetConstPtr(), bufSrc.GetSize(), false))	return false;

	// 圧縮後のサイズの取得に失敗
	if(!Compress(&stSrc, NULL, nCompLv, &nSize))		return false;

	bufDst.Resize(nSize);

	// 出力ストリームの初期化に失敗
	if(!stDst.OpenFromMemory(bufDst.GetPtr(), bufDst.GetSize(), true))	return false;

	// 入力ストリームを巻き戻す
	stSrc.SetSeek(0, SEEK_SET);

	// 圧縮に失敗
	if(!Compress(&stSrc, &stDst, nCompLv, NULL))	return false;

	return true;
}

} // end of namespace sueLib

