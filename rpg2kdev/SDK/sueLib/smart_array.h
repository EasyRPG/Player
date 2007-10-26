/**
	@file
	@brief		参照カウンタつきスマートポインタ
				・・・の配列版

	@author		trino
*/
#ifndef __SMART_ARRAY__
#define __SMART_ARRAY__

#include "sueLib_Compile.h"

/// 参照カウンタつきスマートポインタ
template< class P >
class smart_array
{
public:
	//=============================================================================
	/**
		デフォルトコンストラクタ。
		・・・・からっぽである。
	*/
	//=============================================================================
	smart_array():m_pnCounter( NULL ),m_aObject( NULL ),m_nSize( 0 ){}	///< デフォルトコンストラクタ


	//=============================================================================
	/**
		コピーコンストラクタ。
		アップキャストもOK

		@param	p		[in] smart_arrayの配列データ
	*/
	//=============================================================================
	template< class D >
	smart_array( const smart_array< D >& p ){
		if( p != NULL ){
			//ポインタコピー
			m_aObject = p.m_aObject;
			m_pnCounter = p.m_pnCounter;
			//参照カウンタ＋＋
			(*m_pnCounter)++;
			//サイズ
			m_nSize = p.m_nSize;
		}else{
			m_aObject = NULL;
			m_pnCounter = NULL;
			m_nSize = 0;
		}
	}	///< コピーコンストラクタ
	smart_array( const smart_array& p ){
		if( p != NULL ){
			m_aObject = p.m_aObject;
			m_pnCounter = p.m_pnCounter;
			(*m_pnCounter)++;
			m_nSize = p.m_nSize;
		}else{
			m_aObject = NULL;
			m_pnCounter = NULL;
			m_nSize = 0;
		}
	}

	//=============================================================================
	/**
		生ポインタコンストラクタ(何)
		サイズを渡すといろいろ嬉しいことがあるかも。

		@param	p		[in] 配列ポインタ
		@param	nSize	[in] 配列のサイズ
	*/
	//=============================================================================
	smart_array( P* p, unsigned int nSize = 0xFFFFFFFF ){
		//オブジェクト保管
		m_aObject = p;
		//制御パラ確保
		m_pnCounter = new unsigned int;
		//参照カウンタセット
		*m_pnCounter = 1;
		//サイズ
		m_nSize = nSize;
	}	///< 生ポインタコンストラクタ

//	smart_array( unsigned int nSize ){ smart_array(new P[nSize], nSize); }	///< サイズ指定コンストラクタ


	//=============================================================================
	/**
		代入演算子

		@param	p		[in] smart_arrayの配列データ

		@return			自分自身のオブジェクト
	*/
	//=============================================================================
	template< class D >
	smart_array< P >& operator =( const smart_array< D >& p ){
		//中身をやさしく削除
		Release();
		if( p != NULL ){
			//メンバコピー
			m_pnCounter = p.m_pnCounter;
			m_aObject = p.m_aObject;
			m_nSize = p.m_nSize;
			//参照カウンタUP!!
			(*m_pnCounter)++;
		}
		return *this;
	}	///< 代入演算子
	smart_array& operator =( const smart_array& p ){
		Release();
		if( p != NULL ){
			m_pnCounter = p.m_pnCounter;
			m_aObject = p.m_aObject;
			m_nSize = p.m_nSize;
			(*m_pnCounter)++;
		}
		return *this;
	}
	template< class D >
	smart_array< P >& operator =( D* p ){
		//サイズがわからないので非推奨

		//中身をやんわり削除
		Release();
		//ぬるぽ！！
		if( p != NULL ){
			//制御パラ確保
			m_pnCounter = new unsigned int;
			//代入
			m_aObject = p;
			*m_pnCounter = 1;
			m_nSize = 0xFFFFFFFF;
		}

		return *this;
	}

	//=============================================================================
	/**
		再利用

		@param	p		[in] smart_arrayの配列データ
		@param	nSize	[in] 配列のサイズ

		@return			自分自身のオブジェクト
	*/
	//=============================================================================
	template< class D >
	smart_array< P >& Reset( D* p, unsigned int nSize ){
		Release();
		if( p != NULL ){
			//制御パラ確保
			m_pnCounter = new unsigned int;
			//代入
			m_aObject = p;
			*m_pnCounter = 1;
			m_nSize = nSize;
		}
		return *this;
	}	///< 再利用

	//=============================================================================
	/**
		大きさだけ指定して作成する(型は自分自身の型で固定)

		@param	nSize	[in] 配列のサイズ

		@return			自分自身のオブジェクト
	*/
	//=============================================================================
	smart_array< P >& Resize(unsigned int nSize ){
#ifndef _DEBUG
		return Reset(new P[nSize], nSize);
#else
		P* p = new P[nSize];
		if(p==NULL)		throw "Buffer Error";
		return Reset(p, nSize);
#endif
	}	///< 配列の大きさだけ指定して作成

	//=============================================================================
	/**
		自分自身の型のサイズを取得

		@return			自分自身の型のサイズ
	*/
	//=============================================================================
	unsigned int SizeOf() const { return sizeof(P); }	///< 自分自身の型のサイズを取得


	//=============================================================================
	/**
		他からコピーしてくる

		@param		p		コピー元
	*/
	//=============================================================================
	void Copy(const smart_array p){
		/*
		Resize(p.GetSize());
		for(unsigned int i = 0; i < GetSize(); i++){
			*(GetPtr()+i) = p[i];
		}
		*/
		if(p!=NULL){
			unsigned int i = p.GetSize();
			Resize(i);
			P* p1		= m_aObject;
			const P* p2	= p.m_aObject;
			while(i-->0){
				*p1++ = *p2++;
			}
		}
	}


	//=============================================================================
	/**
		ディストラクタ。
	*/
	//=============================================================================
	virtual ~smart_array(){
		Release();
	}	///< ディストラクタ

	//=============================================================================
	/**
		配列のふりをする。

		@param	i		[in] 配列の要素番号

		@return			配列の要素
	*/
	//=============================================================================
#ifndef _DEBUG
	P& operator[]( unsigned int i ) const { return m_aObject[i]; }	///< 配列のふりをする
#else
	//=============================================================================
	/**
		配列のふりをする。

		@note			デバッグ用。配列を超えると例外を投げてくる。
		@param	i		[in] 配列の要素番号

		@return			配列の要素
	*/
	//=============================================================================
	P& operator[]( unsigned int i ) const {
		if( i < m_nSize ){
			return  m_aObject[i];
		}else{
			throw "Buffer Overflow";
		}
	}	//< 配列のふりをする
#endif

	//=============================================================================
	/**
		比較演算子。
		NULLチェックにも使えます。

		@param	p		[in] ポインタやsmart_arrayのオブジェクト

		@retval	true	2つは同じもの
		@retval	false	2つは違うもの
	*/
	//=============================================================================
	bool operator==( const P* p ) const{
		return m_aObject == p;
	}	///< 比較演算子
	bool operator==( const smart_array< P >& p ) const{
		return m_aObject == p.m_aObject;
	}
	bool operator!=( const P* p ) const{
		return m_aObject != p;
	}
	bool operator!=( const smart_array< P >& p ) const{
		return m_aObject != p.m_aObject;
	}

	//=============================================================================
	/**
		コンテナ的なこと

		@return			配列の要素数
	*/
	//=============================================================================
#ifndef _DEBUG
	unsigned int GetSize() const { return m_nSize; }	///< 配列のサイズを取得
#endif
#ifdef _DEBUG
	unsigned int GetSize() const {
		if( m_nSize == 0xFFFFFFFF ){
			throw "undefined size";
		}else{
			return m_nSize;
		}
	}	///< コンテナ的なこと
#endif
	P* GetPtr(){ return m_aObject; }				///< 配列の先頭のポインタを取得
	P* GetEndPtr(){ return m_aObject + m_nSize; }	///< 配列の終端のポインタを取得
	const P* GetConstPtr() const{ return m_aObject; }	///< 配列の先頭のポインタを取得(const版)


#if (_MSC_VER<=1200)	// VC++6.0以前
	friend class smart_array;
#else					// VC++.NET以降
	template< class Y > friend class smart_array;
#endif

//protected:
	//=============================================================================
	/**
		オブジェクトを解放
	*/
	//=============================================================================
	void Release(){
		//ぬるぽ中
		if( m_aObject == NULL ){
			return;
		}
		//参照カウンタが０なら制御DATAも削除
		if( --(*m_pnCounter) == 0 ){
			delete m_pnCounter;
			delete[] m_aObject;
		}
		//初物のふりをする。
		m_pnCounter = NULL;
		m_aObject = NULL;
		m_nSize = 0;
		return;
	}	///< オブジェクトを解放


//private:
protected:
	unsigned int* m_pnCounter;	///< 参照カウンタ
	P* m_aObject;				///< 配列データ本体

	unsigned int m_nSize;		///< 配列の要素数


};


#endif