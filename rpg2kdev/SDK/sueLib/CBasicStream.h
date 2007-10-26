/**
	@file
	@brief		ファイル入力とメモリ入力を同じように扱うための基本ストリーム
	@author		sue445
*/
#ifndef _INC_CBASICSTREAM
#define _INC_CBASICSTREAM


#include "sueLib_Compile.h"
#include <stdio.h>
//#include <memory.h>


/// 基本ストリーム
class CBasicStream{
private:
	bool  m_bOpened;		///< ストリームが開かれているかどうか
	bool  m_bFile;			///< ファイル入力モードかどうか
	bool  m_bWrite;			///< 書き出しモードかどうか
	const void* m_pData;		///< メモリデータ
	int   m_nFile;			///< ファイルハンドル
	unsigned int m_nLength;		///< ストリームの長さ
	unsigned int m_nPosition;	///< ストリームの現在位置

public:
	CBasicStream():
		m_bOpened(false),
		m_bFile(false),
		m_bWrite(false),
		m_pData(NULL),
		m_nFile(-1),
		m_nLength(0),
		m_nPosition(0){}	///< コンストラクタ

	~CBasicStream(){ Close(); }	///< デストラクタ

	virtual bool OpenFromMemory(const void* pData, unsigned int nLength, bool bWrite=false);	///< メモリデータからストリームを開く
	virtual bool OpenFromFile(const char* szFile, bool bWrite=false);	///< ファイルからストリームを開く
	virtual void Close();							
///< ストリームを閉じる
	virtual void SetSeek(int nPos, int nOrigin);				///< ストリームポインタを移動する
	virtual unsigned int Read(void* pData, unsigned int nSize);		///< ストリームから読み込む
	virtual unsigned int Write(const void* pData, unsigned int nSize);	///< ストリームに書き出す

	virtual bool IsOpened()          const{ return m_bOpened; }		///< ストリームが開かれているかどうか
	virtual bool IsEof()             const{ return (m_nPosition==m_nLength); }	///< ストリームが終端かどうか
	virtual bool IsWriteMode()       const{ return m_bWrite; }		///< 書き出しモードかどうか
	virtual unsigned int GetLength() const{ return m_nLength; }		///< ストリームの長さを取得
	virtual unsigned int GetSeek()   const{ return m_nPosition; }		///< ストリームの現在位置を取得

private:
	void MemoryCopy(void* pDst, const void* pSrc, unsigned int nSize);	///< メモリ転送
};

#endif
