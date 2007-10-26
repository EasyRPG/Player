/**
	@file
	@brief		ツクール仕様の配列(1次元と2次元)を扱うクラス。\n
				配列の中身をバッファとして直接格納する。\n
				ツクールの配列の要素番号は1～だが、内部では自動的に0～で扱う。

	@author		sue445
*/
#ifndef _INC_CRPGARRAY
#define _INC_CRPGARRAY

#include <vector>
#include <string>
#include "sueLib/smart_buffer.h"
#include "sueLib/CMapTable.h"

using std::vector;
using std::string;
using namespace sueLib;

/// 1次元配列
class CRpgArray1 : private CMapTable< smart_buffer, 1 >{
private:
	int		m_nMaxCol;												///< 列の最大数

public:
	CRpgArray1() : m_nMaxCol(0){}									///< デフォルトコンストラクタ
	~CRpgArray1(){}													///< デストラクタ

	void Init(){ Release(); m_nMaxCol = 0; }						///< 配列を初期化
	unsigned int GetMaxSize() const{ return m_nMaxCol; }			///< 配列の最大要素数を返す

	int				GetNumber(int col, int  def=0) const;			///< 整数を取得
	bool			GetFlag(  int col, bool def=false) const;		///< bool型フラグを取得
	string			GetString(int col, string def="") const;		///< 文字列を取得
	smart_buffer	GetData(  int col) const;						///< バッファを取得

	void SetNumber(int col, int  data);								///< 整数をセット
	void SetFlag(  int col, bool data);								///< bool型フラグをセット
	void SetString(int col, const string& data);					///< 文字列をセット
	void SetData(  int col, const smart_buffer& buf);				///< バッファをセット
};

/// 2次元配列
class CRpgArray2 : private CMapTable< smart_buffer, 2 >{
private:
	int		m_nMaxCol;														///< 列の最大数
	int		m_nMaxRow;														///< 行の最大数

public:
	CRpgArray2() : m_nMaxCol(0), m_nMaxRow(0){}								///< デフォルトコンストラクタ
	~CRpgArray2(){}															///< デストラクタ

	void Init(){ Release(); m_nMaxCol = m_nMaxRow = 0; }					///< 初期化
	unsigned int GetMaxCol() const { return m_nMaxCol; }					///< 配列の列数を取得
	unsigned int GetMaxRow() const { return m_nMaxRow; }					///< 配列の行数を取得

	int				GetNumber(int row, int col, int  def=0) const;			///< 整数を取得
	bool			GetFlag(  int row, int col, bool def=false) const;		///< bool型フラグを取得
	string			GetString(int row, int col, string def="") const;		///< 文字列を取得
	smart_buffer	GetData(  int row, int col) const;						///< バッファを取得

	void SetNumber(int row, int col, int  data);							///< 整数をセット
	void SetFlag(  int row, int col, bool data);							///< bool型フラグをセット
	void SetString(int row, int col, const string& data);					///< 文字列をセット
	void SetData(  int row, int col, const smart_buffer& buf);				///< バッファをセット
};

#endif
