/**
	@file
	@brief		ストリームを介せずにバッファから直接データを操作する
	@author		sue445
*/
#include "CRpgUtil.h"
#include "CRpgIOBase.h"


//=============================================================================
/**
	バッファから直接データを取得

	@param	buf			[in] 入力元バッファ

	@return				取得したバッファ
*/
//=============================================================================
smart_buffer CRpgUtil::GetData(const smart_buffer& buf)
{
	CRpgIOBase st;
	smart_buffer temp;

	return (st.OpenFromMemory(buf.GetConstPtr(), buf.GetSize())) ? st.ReadData() : temp;
}

//=============================================================================
/**
	バッファから文字列を取得

	@param	buf			[in] 入力元バッファ

	@return				取得した文字列
*/
//=============================================================================
string CRpgUtil::GetString(const smart_buffer& buf)
{
	CRpgIOBase st;

	return (st.OpenFromMemory(buf.GetConstPtr(), buf.GetSize())) ? st.ReadString() : "";
}

//=============================================================================
/**
	ツクール仕様の1次元配列を読み込む

	@param	buf		[in] 入力元バッファ

	@return			取得した1次元配列
*/
//=============================================================================
CRpgArray1 CRpgUtil::GetArray1(const smart_buffer& buf)
{
	int col;
	CRpgIOBase		st;
	CRpgArray1		array;


	// バッファが初期化されていない
	if(buf.GetSize()==0)		return array;

	// 読み込み用ストリームの初期化
	st.OpenFromMemory(buf.GetConstPtr(), buf.GetSize());

	// データを格納する
	while(!st.IsEof()){
		col = st.ReadBerNumber();
		if(col==0)	break;
		array.SetData(col, st.ReadData());
	}

	return array;
}

//=============================================================================
/**
	ツクール仕様の2次元配列を読み込む

	@param	buf		[in] 入力元バッファ

	@return			取得した2次元配列
*/
//=============================================================================
CRpgArray2 CRpgUtil::GetArray2(const smart_buffer& buf)
{
	int row, col, max;
	CRpgIOBase		st;
	CRpgArray2		array;


	// バッファが初期化されていない
	if(buf.GetSize()==0)		return array;

	// 読み込み用ストリームの初期化
	st.OpenFromMemory(buf.GetConstPtr(), buf.GetSize());

	// 一番最初には配列のサイズが格納されている
	max = st.ReadBerNumber();

	// データを格納する
	while(!st.IsEof()){
		row = st.ReadBerNumber();
		for(;;){
			col = st.ReadBerNumber();
			if(col==0)	break;
			array.SetData(row, col, st.ReadData());
		}
	}

	return array;
}
