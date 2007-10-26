/**
	@file
	@brief		Basic RPG Maker 2000/2003 IO class file
	@note		Even this class is inherited from various places, you can use it as a single unit.

	@author		sue445
*/
#ifndef _INC_CRPGIOBASE
#define _INC_CRPGIOBASE

#include "CBerStream.h"
#include "CRpgArray.h"
#include "CRpgUtil.h"
#include <stdio.h>
#include <string>

using std::string;
/*
#ifndef FNAME_MAX32
#define FNAME_MAX32		512
#endif

#ifndef ZeroMemory
#define ZeroMemory(buf,size)	memset(buf, 0, size)
#endif

#define MAX_RPGNAME		21	///< Full width of ten characters (Translation note: for japanese SHIFT_JIS, double byte and \0)
*/

/*
#pragma pack(push)
#pragma pack(1) // Member boundary of the structure is designated as 1 byte

#ifndef POINT
/// Coordinates structure
struct POINT{
	int x;	///< X position
	int y;	///< Y position
};
#endif

#pragma pack(pop)
*/

/// Basic RPG Maker IO class file
class CRpgIOBase : public CBerStream{
private:
	virtual const char* GetHeader(){ return ""; }		///< ファイル識別用の文字列(継承先で指定します)

protected:
	bool OpenFile(const char* szFile);			///< ファイルを開く(識別チェック有り)
	bool bInit;						///< 初期化されているかどうか

public:
	CRpgIOBase() : bInit(false){}				///< Constructor
	~CRpgIOBase(){}						///< Destructor

	smart_buffer	ReadData();				///< データを読み込む
	string			ReadString();			///< 文字列を読み込む
	bool			IsInit() const{ return bInit; }	///< 初期化されているかどうか

	void ByteToBool(unsigned char num, bool flag[8]) const;	///< unsigned char -> bool配列

	//=============================================================================
	/**
		2次元smart_arrayを[y][x]配列とみなして初期化する

		@param	array		[out] smart_arrayの二次元配列
		@param	x			[in] X方向の要素数
		@param	y			[in] Y方向の要素数
	*/
	//=============================================================================
	template< class T >
	void InitArray2(smart_array< smart_array< T > >& array, int x, int y) const
	{
		array.Resize(y);
		for(int j = 0; j < y; j++){
			array[j].Resize(x);
		}
	}	///< 2次元smart_arrayを[y][x]配列とみなして初期化する

	//=============================================================================
	/**
		バッファの中身を2次元smart_arrayにぶち込む

		@param	array		[out] Two-dimensional array of smart_array
		@param	buf			[in] Source input buffer
		@param	x			[in] Number of elements of X direction
		@param	y			[in] Number of elements of Y direction

		@retval	true		Succesfully array storing
		@retval	false		Failed array storing
	*/
	//=============================================================================
	template< class T >
	bool BufferToArray2(smart_array< smart_array< T > >& array, smart_buffer& buf, int x, int y) const
	{
		CBasicStream st;
		T data;

		// Initialization failure without knowing why.
		if(!st.OpenFromMemory(buf, buf.GetSize()))	return false;

		// Initializing smart_array first
		InitArray2(array, x, y);

		for(int j = 0; j < y; j++){
			for(int i = 0; i < x; i++){
				st.Read(&data, sizeof(T));
				array[j][i] = data;
			}
		}

		return true;
	}	///< The contents of the buffer are put into two-dimensional smart_array.
};

#endif
