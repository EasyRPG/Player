/**
	@file
	@brief		ツクール仕様の配列(1次元と2次元)を扱うクラス。\n
				配列の中身をバッファとして直接格納する。\n
				ツクールの配列の要素番号は1～だが、内部では自動的に0～で扱う。

	@author		sue445
*/
#include "CRpgArray.h"
#include "CRpgUtil.h"


////////////////////////////////////
// 1次元配列
////////////////////////////////////
//=============================================================================
/**
	バッファの中身をBER圧縮整数として取得

	@param	col			[in] 配列の要素番号(1～)
	@param	def			[in] 要素が存在しなかった場合の初期値

	@return				取得した整数
*/
//=============================================================================
int CRpgArray1::GetNumber(int col, int def) const
{
	return (IsExist(CDim1(col))) ? CRpgUtil::GetBerNumber(Get(CDim1(col))) : def;
}

//=============================================================================
/**
	バッファの中身をbool型(0 or 1)として取得

	@param	col			[in] 配列の要素番号(1～)
	@param	def			[in] 要素が存在しなかった場合の初期値

	@return				取得したフラグ
*/
//=============================================================================
bool CRpgArray1::GetFlag(int col, bool def) const
{
	return (IsExist(CDim1(col))) ? (CRpgUtil::GetBerNumber(Get(CDim1(col))) != 0 ) : def;
}

//=============================================================================
/**
	バッファの中身を文字列として取得(string版)

	@param	col			[in] 配列の要素番号(1～)
	@param	def			[in] 要素が存在しなかった場合の初期値

	@return				取得した文字列
*/
//=============================================================================
string CRpgArray1::GetString(int col, string def) const
{
//	return (IsExist(CDim1(col))) ? CRpgUtil::GetString(Get(CDim1(col))) : def;
	if(IsExist(CDim1(col))){
		smart_buffer buf = Get(CDim1(col));
		string str(buf, buf.GetSize());
		return str;
	}
	return def;
}

//=============================================================================
/**
	バッファの中身をそのまま取得

	@param	col			[in] 配列の要素番号(1～)

	@return				取得したバッファ
*/
//=============================================================================
smart_buffer CRpgArray1::GetData(int col) const
{
	smart_buffer buf;
	return (IsExist(CDim1(col))) ? Get(CDim1(col)) : buf;
}

//=============================================================================
/**
	バッファにBER圧縮整数をセット

	@param	col			[in] 配列の要素番号(1～)
	@param	data		[in] 整数
*/
//=============================================================================
void CRpgArray1::SetNumber(int col, int data)
{
	Set(CRpgUtil::SetBerNumber(data), CDim1(col));
	if(col > m_nMaxCol)		m_nMaxCol = col;
}

//=============================================================================
/**
	バッファにbool型(0 or 1)をセット

	@param	col			[in] 配列の要素番号(1～)
	@param	data		[in] フラグ
*/
//=============================================================================
void CRpgArray1::SetFlag(int col, bool data)
{
	Set(CRpgUtil::SetBerNumber(data), CDim1(col));
	if(col > m_nMaxCol)		m_nMaxCol = col;
}

//=============================================================================
/**
	バッファに文字列をセット

	@param	col			[in] 配列の要素番号(1～)
	@param	data		[in] 文字列
*/
//=============================================================================
void CRpgArray1::SetString(int col, const string& data)
{
	smart_buffer buf(data.length());
	memcpy(buf, data.c_str(), data.length());
	Set(buf, CDim1(col));
	if(col > m_nMaxCol)		m_nMaxCol = col;
}

//=============================================================================
/**
	バッファをセット

	@param	col			[in] 配列の要素番号(1～)
	@param	buf			[in] バッファ
*/
//=============================================================================
void CRpgArray1::SetData(int col, const smart_buffer& buf)
{
	Set(buf, CDim1(col));
	if(col > m_nMaxCol)		m_nMaxCol = col;
}


////////////////////////////////////
// 2次元配列
////////////////////////////////////
//=============================================================================
/**
	バッファの中身をBER圧縮整数として取得

	@param	row			[in] 配列の親要素の番号(1～)
	@param	col			[in] 配列の子要素の番号(1～)
	@param	def			[in] 要素が存在しなかった場合の初期値

	@return				取得した整数
*/
//=============================================================================
int CRpgArray2::GetNumber(int row, int col, int def) const
{
	return (IsExist(CDim2(col,row))) ? CRpgUtil::GetBerNumber(Get(CDim2(col,row))) : def;
}

//=============================================================================
/**
	バッファの中身をbool型(0 or 1)として取得

	@param	row			[in] 配列の親要素の番号(1～)
	@param	col			[in] 配列の子要素の番号(1～)
	@param	def			[in] 要素が存在しなかった場合の初期値

	@return				取得したフラグ
*/
//=============================================================================
bool CRpgArray2::GetFlag(int row, int col, bool def) const
{
	return (IsExist(CDim2(col,row))) ? (CRpgUtil::GetBerNumber(Get(CDim2(col,row))) != 0 ) : def;
}

//=============================================================================
/**
	バッファの中身を文字列として取得(string版)

	@param	row			[in] 配列の親要素の番号(1～)
	@param	col			[in] 配列の子要素の番号(1～)
	@param	def			[in] 要素が存在しなかった場合の初期値

	@return				取得した文字列
*/
//=============================================================================
string CRpgArray2::GetString(int row, int col, string def) const
{
//	return (IsExist(CDim2(col,row))) ? CRpgUtil::GetString(Get(CDim2(col,row))) : def;
	if(IsExist(CDim2(col,row))){
		smart_buffer buf = Get(CDim2(col,row));
		string str(buf, buf.GetSize());
		return str;
	}
	return def;
}

//=============================================================================
/**
	バッファの中身をそのまま取得

	@param	row			[in] 配列の親要素の番号(1～)
	@param	col			[in] 配列の子要素の番号(1～)

	@return				取得したバッファ
*/
//=============================================================================
smart_buffer CRpgArray2::GetData(int row, int col) const
{
	smart_buffer buf;
	return (IsExist(CDim2(col,row))) ? Get(CDim2(col,row)) : buf;
}

//=============================================================================
/**
	バッファにBER圧縮整数をセット

	@param	row			[in] 配列の親要素の番号(1～)
	@param	col			[in] 配列の子要素の番号(1～)
	@param	data		[in] 整数
*/
//=============================================================================
void CRpgArray2::SetNumber(int row, int col, int data)
{
	Set(CRpgUtil::SetBerNumber(data), CDim2(col,row));
	if(col > m_nMaxCol)		m_nMaxCol = col;
	if(row > m_nMaxRow)		m_nMaxRow = row;
}

//=============================================================================
/**
	バッファにbool型(0 or 1)をセット

	@param	row			[in] 配列の親要素の番号(1～)
	@param	col			[in] 配列の子要素の番号(1～)
	@param	data		[in] フラグ
*/
//=============================================================================
void CRpgArray2::SetFlag(int row, int col, bool data)
{
	Set(CRpgUtil::SetBerNumber(data), CDim2(col,row));
	if(col > m_nMaxCol)		m_nMaxCol = col;
	if(row > m_nMaxRow)		m_nMaxRow = row;
}

//=============================================================================
/**
	バッファに文字列をセット

	@param	row			[in] 配列の親要素の番号(1～)
	@param	col			[in] 配列の子要素の番号(1～)
	@param	data		[in] 文字列
*/
//=============================================================================
void CRpgArray2::SetString(int row, int col, const string& data)
{
	smart_buffer buf(data.length());
	memcpy(buf, data.c_str(), data.length());
	Set(buf, CDim2(col,row));
	if(col > m_nMaxCol)		m_nMaxCol = col;
	if(row > m_nMaxRow)		m_nMaxRow = row;
}

//=============================================================================
/**
	バッファをセット

	@param	row			[in] 配列の親要素の番号(1～)
	@param	col			[in] 配列の子要素の番号(1～)
	@param	buf			[in] バッファ
*/
//=============================================================================
void CRpgArray2::SetData(int row, int col, const smart_buffer& buf)
{
	Set(buf, CDim2(col,row));
	if(col > m_nMaxCol)		m_nMaxCol = col;
	if(row > m_nMaxRow)		m_nMaxRow = row;
}
