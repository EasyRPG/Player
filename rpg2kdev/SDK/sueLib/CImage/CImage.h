/**
	@file
	@brief		画像のピクセルデータを管理するクラス
	@author		sue445
*/
#ifndef _INC_CIMAGE
#define _INC_CIMAGE

#include "../sueLib_Compile.h"
#include <stdio.h>
#include <memory.h>
#include "../smart_array.h"

#ifndef RGBA_MAKE
	#define RGBA_MAKE(r, g, b, a)	((unsigned long) (((a) << 24) | ((r) << 16) | ((g) << 8) | (b))) 
#endif


namespace sueLib {

unsigned char Alpha(unsigned long rgba);	///< Alphaを取り出す
unsigned char Red(  unsigned long rgba);	///< Redを取り出す
unsigned char Green(unsigned long rgba);	///< Greenを取り出す
unsigned char Blue( unsigned long rgba);	///< Blueを取り出す
unsigned char CalcAlpha(unsigned char src, unsigned char dst, unsigned char alpha);	///< α合成
unsigned char CalcAlpha2(unsigned char src, unsigned char dst, unsigned char alpha, unsigned char opacity);	///< α+不透明度合成

#pragma pack(push)
#pragma pack(1) //構造体のメンバ境界を1バイトにする

/// WINカラーテーブル要素
typedef struct tagRGBQUAD {
	unsigned char	rgbBlue;		///< 青
	unsigned char	rgbGreen;		///< 緑
	unsigned char	rgbRed;			///< 赤
	unsigned char	rgbReserved;	///< 予約
} RGBQUAD;

/// OS/2 カラーテーブル要素(CImage24用に若干拡張)
struct RGBTRIPLE {
	unsigned char		rgbBlue;	///< 青
	unsigned char		rgbGreen;	///< 緑
	unsigned char		rgbRed;		///< 赤

	/// デフォルトコンストラクタ
	RGBTRIPLE() : rgbBlue(0), rgbGreen(0), rgbRed(0) {}

	/// コピーコンストラクタ
	RGBTRIPLE(unsigned long color){ operator=(color); }

	/// 代入演算子
	RGBTRIPLE& operator=(unsigned long color)
	{
		rgbRed		= (unsigned char)((color & 0x00FF0000) >> 16);
		rgbGreen	= (unsigned char)((color & 0x0000FF00) >> 8);
		rgbBlue		= (unsigned char)( color & 0x000000FF);
		return *this;
	}

	/// RGBを合成したカラーコードを返す
	operator unsigned long() const { return (rgbRed<<16) | (rgbGreen<<8) | rgbBlue; }
};

#pragma pack(pop)

/// CBasicImageやCImgUtil内で使用するα値などの型
typedef double Float;	
//typedef float Float;

/// 画像サイズ
typedef unsigned short ImgLength;

template< typename PIXEL > struct AlphaFunc;

/// 8bits時(カラーキー処理)
template<>
struct AlphaFunc< unsigned char >{
	AlphaFunc(unsigned char* pDst, const unsigned char* pSrc, int nDstWidth, int nSrcWidth, int x, int y,int left, int top, int width, int height)
	{
		// 参考文献)やねうらお著「Windowsプロフェッショナルゲームプログラミング2」
	//	unsigned long dwRgbColorKey = rgbColorKey | (rgbColorKey<<8) | (rgbColorKey<<16) | (rgbColorKey<<24);
		unsigned long dwRgbColorKey = 0;	// どうせ抜き色は0で固定だからこれでＯＫｗ
		while(height-- > 0){
			unsigned long* pcSrc = (unsigned long*)pSrc;
			unsigned long* pcDst = (unsigned long*)pDst;
			while((unsigned char*)pcSrc+4<=pSrc+width){
				unsigned long mask = (*pcSrc ^ dwRgbColorKey);
				// ↑xorをとれば、完全に一致している場合、そのunsigned charは0になるはず
				mask = mask | ((mask  & 0x7F7F7F7F) + 0x7F7F7F7F);
				// ↑このmaskのMSBが1のバイトは、一致していないので転送する必要がある
				mask = (mask & 0x80808080) >> 7;	// LSBに移動
				mask = (mask + 0x7F7F7F7F) ^ 0x7F7F7F7F;
				*pcDst = ((*pcSrc ^ *pcDst) & mask) ^ *pcDst;
				pcDst++;
				pcSrc++;
			}

			// 端数の補正(これくらいforで回してもいいよねｗ)
			if((width & 0x03) != 0){
				int max = (width & 0x03);
				unsigned char* psrc = (unsigned char*)pcSrc;
				unsigned char* pdst = (unsigned char*)pcDst;
				while(max-- > 0){
					if(*psrc)	*pdst = *psrc;
					psrc++;
					pdst++;
				}
			}
			pDst += nDstWidth;
			pSrc += nSrcWidth;
		}
	}
};

/// 32bits時(アルファチャンネル処理)
template<>
struct AlphaFunc< unsigned long >{
	AlphaFunc(unsigned long* pDst, const unsigned long* pSrc, int nDstWidth, int nSrcWidth, int x, int y,int left, int top, int width, int height)
	{
		unsigned char cSrcR, cSrcG, cSrcB, cAlpha;
		unsigned char cDstR, cDstG, cDstB;

		while(height-->0){
			int w = width;
			const PIXEL*	_pSrc = pSrc;
			PIXEL*			_pDst = pDst;

			while(w-->0){
				cAlpha = Alpha(*_pSrc);
				cSrcR  = Red(*_pSrc);
				cSrcG  = Green(*_pSrc);
				cSrcB  = Blue(*_pSrc);
				cDstR  = Red(*_pDst);
				cDstG  = Green(*_pDst);
				cDstB  = Blue(*_pDst);

			//	/*
				// 1)αを全部計算する
				*_pDst = (*_pDst & 0xFF000000) |
					CalcAlpha(cSrcR, cDstR, cAlpha)<<16 |
					CalcAlpha(cSrcG, cDstG, cAlpha)<<8 |
					CalcAlpha(cSrcB, cDstB, cAlpha);
			//	*/
				/*
				// 2)α値によってif分岐
				if(cAlpha==0x00){
					*_pDst = (*_pDst & 0xFF000000) | cDstR<<16 | cDstG<<8 | cDstB;
				}
				else if(cAlpha==0xFF){
					*_pDst = (*_pDst & 0xFF000000) | cSrcR<<16 | cSrcG<<8 | cSrcB;
				}
			//	else if(cAlpha==0x7F || cAlpha==0x80){
			//		*_pDst = (*_pDst & 0xFF000000) |
			//			((unsigned char)((cSrcR>>1)+(cDstR>>1)))<<16 |
			//			((unsigned char)((cSrcG>>1)+(cDstG>>1)))<<8 |
			//			( unsigned char)((cSrcB>>1)+(cDstB>>1));
			//	}
				else{
					*_pDst = (*_pDst & 0xFF000000) |
						CalcAlpha(cSrcR, cDstR, cAlpha)<<16 |
						CalcAlpha(cSrcG, cDstG, cAlpha)<<8 |
						CalcAlpha(cSrcB, cDstB, cAlpha);
				}
				*/

				_pSrc++;
				_pDst++;
			}

			pSrc += nSrcWidth;
			pDst += nDstWidth;
		}
	}
};

/// 8bits,32bits以外(通常描画)
template< typename PIXEL >
struct AlphaFunc{
	AlphaFunc(PIXEL* pDst, const PIXEL* pSrc, int nDstWidth, int nSrcWidth, int x, int y,int left, int top, int width, int height)
	{
		unsigned int linesize = sizeof(PIXEL) * width;
		while(height-->0){
			memcpy(pDst, pSrc, linesize);
			pSrc += nSrcWidth;
			pDst += nDstWidth;
		}
	}
};

/// 基本クラス(テンプレートには、1ピクセル辺りに使用する型を指定します)
template< typename PIXEL >
class CBasicImage {
private:
//protected:

//#pragma pack(push)
//#pragma pack(1) //構造体のメンバ境界を1バイトにする
	/// RGBの順で並んだ構造体(CBasicImage内部ではこれで管理されます)
	typedef struct tagRGBSTRUCT{
		unsigned char	red;			///< 赤
		unsigned char	green;			///< 緑
		unsigned char	blue;			///< 青
	} RGBSTRUCT;
//#pragma pack(pop)

	ImgLength		m_shWidth;					///< 幅
	ImgLength		m_shHeight;					///< 高さ
	smart_array< PIXEL >		m_saImage;		///< ピクセルデータ
	smart_array< RGBSTRUCT >	m_saPalette;	///< パレットデータ
	PIXEL*			m_pIndex;					///< 画像操作用のポインタ

public:

	friend class CBasicImage;

	//=============================================================================
	/**
		コンストラクタ

		@param	width		[in] 画像の幅
		@param	height		[in] 画像の高さ
	*/
	//=============================================================================
	CBasicImage(ImgLength width=0, ImgLength height=0):
		m_shWidth(0),
		m_shHeight(0),
		m_pIndex(NULL)
	{
		Create(width, height);
	}	///< コンストラクタ


	~CBasicImage(){ Destroy(); }	///< デストラクタ

	/// コピーコンストラクタ
	CBasicImage(const CBasicImage& obj){ operator=(obj); }

	// 代入コンストラクタ
	CBasicImage& operator=(const CBasicImage& obj)
	{
		m_shWidth	= obj.m_shWidth;
		m_shHeight	= obj.m_shHeight;
		m_pIndex	= obj.m_pIndex;

		if(obj.m_saImage!=NULL)		m_saImage.Copy(obj.m_saImage);
		else						m_saImage.Release();
		if(obj.m_saPalette!=NULL)	m_saPalette.Copy(obj.m_saPalette);
		else						m_saPalette.Release();

		return *this;
	}

	//=============================================================================
	/**
		画像を作成

		@note				nWidthかnHeightのどちらかに0を指定すると解放だけされます

		@param	width		[in] 幅
		@param	height		[in] 高さ
	*/
	//=============================================================================
	void Create(ImgLength width, ImgLength height)
	{
		Destroy();

		if(width && height){
			m_shWidth	= width;
			m_shHeight	= height;
			m_saImage.Resize(m_shWidth * m_shHeight);

			// 8ビット以下ならパレットの確保
			if(GetBitCount()<=8){
				m_saPalette.Resize( 1<<GetBitCount() );
			}
			Clear(true);
			Top();
		}
	}	///< 画像を作成

	/// 画像データの解放
	void Destroy()
	{
		/*
		if(m_saImage!=NULL){
			m_saImage.Release();
			m_pIndex = NULL;
			m_shWidth = m_shHeight = 0;
		}
		if(m_saPalette!=NULL){
			m_saPalette.Release();
		}
		*/
		m_saImage.Release();
		m_pIndex = NULL;
		m_shWidth = m_shHeight = 0;
		m_saPalette.Release();
	}	///< 画像データの解放

	//=============================================================================
	/**
		画像データを0で初期化する

		@param	bPalInit		[in] パレットデータも初期化するかどうか
	*/
	//=============================================================================
	void Clear(bool bPalInit=false)
	{
		if(IsInit()){
			memset(m_saImage.GetPtr(), 0, sizeof(PIXEL)*m_saImage.GetSize());
		}

		if(m_saPalette!=NULL && bPalInit){
			memset(m_saPalette.GetPtr(), 0, sizeof(RGBSTRUCT)*m_saPalette.GetSize());
		}
	}	///< 画像データを0で初期化する

	//=============================================================================
	/**
		パレットをセットする

		@param	index		[in] パレット番号
		@param	red			[in] 赤
		@param	green		[in] 緑
		@param	blue		[in] 青

		@retval	true		パレットのセットに成功
		@retval	false		パレットのセットに失敗
	*/
	//=============================================================================
	bool SetPalette(unsigned short index, unsigned char red, unsigned char green, unsigned char blue)
	{
		if(m_saPalette!=NULL && index<m_saPalette.GetSize()){
			m_saPalette[index].red		= red;
			m_saPalette[index].green	= green;
			m_saPalette[index].blue		= blue;

			return true;
		}
		return false;
	}

	//=============================================================================
	/**
		パレットを取得する

		@param	index		[in] パレット番号
		@param	red			[out] 赤
		@param	green		[out] 緑
		@param	blue		[out] 青

		@retval	true		パレットの取得に成功
		@retval	false		パレットの取得に失敗
	*/
	//=============================================================================
	bool GetPalette(unsigned short index, unsigned char* red, unsigned char* green, unsigned char* blue) const
	{
		if(m_saPalette!=NULL && index<m_saPalette.GetSize()){
			*red	= m_saPalette[index].red;
			*green	= m_saPalette[index].green;
			*blue	= m_saPalette[index].blue;

			return true;
		}

		return false;
	}

	//=============================================================================
	/**
		別の画像のパレットをコピーする

		@param	img			[in] コピー元画像
	*/
	//=============================================================================
	void CopyPalette(const CBasicImage< PIXEL >& img)
	{
		unsigned char r, g, b;
		unsigned int i;

		if(img.GetPalSize() >0){
			for(i = 0; i < img.GetPalSize(); i++){
				img.GetPalette(i, &r, &g, &b);
				SetPalette(i, r, g, b);
			}
		}
	}

	bool IsInit() const { return (m_saImage!=NULL); }					///< 初期化されているかどうか
	bool operator!() const{ return !IsInit(); }							///< 初期化されていなければtrue

	ImgLength		GetWidth()    const { return m_shWidth; }			///< 画像の幅を取得
	ImgLength		GetHeight()   const { return m_shHeight; }			///< 画像の高さを取得
	unsigned char	GetBitCount() const { return sizeof(PIXEL)*8; }		///< 画像の色数を取得
	unsigned short	GetPalSize()  const { return (unsigned short)m_saPalette.GetSize(); }///< パレットサイズを取得

	bool IsEof() { return (m_pIndex==m_saImage.GetEndPtr()); }	///< 画像の終端かどうか
	bool IsTop() { return (m_pIndex==m_saImage.GetPtr()); }		///< 画像の先頭かどうか

	///////////////// ピクセル操作用の関数
	//=============================================================================
	/**
		ピクセルデータをセットする

		@param	x			[in] x位置
		@param	y			[in] y位置
		@param	pixel		[in] ピクセルデータ
	*/
	//=============================================================================
	void SetPixel(ImgLength x, ImgLength y, PIXEL pixel)
	{
		if(x<m_shWidth && y<m_shHeight){
			m_saImage[y*m_shWidth+x] = pixel;
		}
	}

	//=============================================================================
	/**
		ピクセルデータを取得する

		@param	x			[in] x位置
		@param	y			[in] y位置

		@return				ピクセルデータ
	*/
	//=============================================================================
	PIXEL GetPixel(ImgLength x, ImgLength y) const
	{
		if(x<m_shWidth && y<m_shHeight){
			return m_saImage[y*m_shWidth+x];
		}

		return 0;
	}

	PIXEL* Index(){  return m_pIndex; }									///< 現在のピクセル

	PIXEL* Top(){ m_pIndex = m_saImage.GetPtr(); return m_pIndex; }		///< 画像の先頭部(左上)に移動

	/// 次のピクセルに移動
	PIXEL* Next()
	{
		if(!IsEof())	m_pIndex++;
		return m_pIndex;
	}

	/// 前のピクセルに移動
	PIXEL* Prev()
	{
		if(!IsTop())	m_pIndex--;
		return m_pIndex;
	}

	/// 次の行に移動
	PIXEL* NextLine()
	{
		m_pIndex += m_shWidth;
		if(m_pIndex>m_saImage.GetEndPtr())	m_pIndex = m_saImage.GetEndPtr();
		return m_pIndex;
	}

	/// 前の行に移動
	PIXEL* PrevLine()
	{
		m_pIndex -= m_shWidth;
		if(m_pIndex<m_saImage.GetPtr())	m_pIndex = m_saImage.GetPtr();
		return m_pIndex;
	}

	//=============================================================================
	/**
		座標を指定して移動

		@param	x			[in] x位置
		@param	y			[in] y位置

		@return				ピクセルデータへのポインタ
	*/
	//=============================================================================
	PIXEL* Locate(ImgLength x, ImgLength y)
	{
		if(x<m_shWidth && y<m_shHeight)	m_pIndex = &m_saImage[y*m_shWidth+x];
		return m_pIndex;
	}	///< 座標を指定して移動

	//=============================================================================
	/**
		その座標のポインタを取得

		@param	x			[in] x位置
		@param	y			[in] y位置

		@return				ピクセルデータへのポインタ
	*/
	//=============================================================================
	const PIXEL* GetPtr(ImgLength x, ImgLength y) const
	{
		if(x<m_shWidth && y<m_shHeight)	return &m_saImage[y*m_shWidth+x];
		return &m_saImage[0];
	}	///< 座標を指定して移動

	//=============================================================================
	/**
		短径を塗りつぶす

		@param	color		[in] 色
		@param	left		[in] 左上x座標
		@param	top			[in] 左上y座標
		@param	width		[in] 幅(省略時:自分自身の幅)
		@param	height		[in] 高さ(省略時:自分自身の高さ)

		@return				ピクセルデータへのポインタ
	*/
	//=============================================================================
	void FillRect(PIXEL color, ImgLength left=0, ImgLength top=0, ImgLength width=0, ImgLength height=0)
	{
		if(left > GetWidth())	left = GetWidth();
		if(top > GetHeight())	top  = GetHeight();
		if(width == 0)			width  = GetWidth()  - left;
		if(height == 0)			height = GetHeight() - top;

		PIXEL* pDst = Locate(left, top);

		while(height-->0){
			int w = width;
			PIXEL* p = pDst;

			while(w-->0){
				*p++ = color;
			}

			pDst = NextLine();
		}
	}

	//=============================================================================
	/**
		αチャンネルを一定の値で埋める(32bits only)

		@param	alpha		[in] α値
	*/
	//=============================================================================
	void FillAlpha(unsigned char alpha)
	{
		if(sizeof(PIXEL)==4 && IsInit()){
			PIXEL* p		= m_saImage.GetPtr();
			PIXEL* p_end	= m_saImage.GetEndPtr();
			while(p < p_end){
				*p = (*p & 0x00FFFFFF) | (alpha << 24);
				p++;
			}
		}
	}	///< αチャンネルを一定の値で埋める(32bits only)

	//=============================================================================
	/**
		描画範囲のチェックをし、必要に応じて数値補正もすえう

		@param	imgSrc		[in,out] コピー元画像
		@param	x			[in,out] コピー先x座標
		@param	y			[in,out] コピー先y座標
		@param	left		[in,out] コピー元x座標
		@param	top			[in,out] コピー元y座標
		@param	width		[in,out] コピー元幅
		@param	height		[in,out] コピー元高さ
	*/
	//=============================================================================
	bool RectCheck(const CBasicImage< PIXEL >& imgSrc, int& x, int& y,
		int& left, int& top, int& width, int& height) const
	{
		if(!IsInit() || !imgSrc.IsInit())		return false;

		if(left < 0)						left = 0;
		if(top < 0)							top  = 0;
		else if(left >= imgSrc.GetWidth())	left = imgSrc.GetWidth() - 1;
		else if(top >= imgSrc.GetHeight())	top  = imgSrc.GetHeight() - 1;

		if(width == 0)		width  = imgSrc.GetWidth()  - left;
		if(height == 0)		height = imgSrc.GetHeight() - top;

		if(x < 0){
			width += x;		// xは負なので結果的に減算
			left  -= x;		// xは負なので結果的に加算
			x = 0;
		}
		if(y < 0){
			height += y;	// yは負なので結果的に減算
			top    -= y;	// yは負なので結果的に加算
			y = 0;
		}

		if(width > GetWidth()-x)	width  = GetWidth()  - x;
		if(height > GetHeight()-y)	height = GetHeight() - y;

		if(width<=0 || height<=0)	return false;

		return true;
	}

	//=============================================================================
	/**
		画像を描画する

		@param	img			[in] コピー元画像
		@param	x			[in] コピー先x座標
		@param	y			[in] コピー先y座標
		@param	left		[in] コピー元x座標
		@param	top			[in] コピー元y座標
		@param	width		[in] コピー元幅(省略時:元画像の幅)
		@param	height		[in] コピー元高さ(省略時:元画像の高さ)
		@param	bAlpha		[in]	32bits:αチャンネル計算をするかどうか(falseでコピー元画像を短径コピーする)
									8bits:0番を抜き色として処理
									それ以外:無視

		@retval	true		描画成功
		@retval	false		描画失敗
	*/
	//=============================================================================
	bool Blt(const CBasicImage< PIXEL >& img, int x=0, int y=0,
		int left=0, int top=0, int width=0, int height=0, bool bAlpha=true)
	{
		if(!RectCheck(img, x, y, left, top, width, height))		return false;

		const PIXEL*	pSrc = img.GetPtr(left, top);
		PIXEL*			pDst = Locate(x, y);

		if(bAlpha){
			AlphaFunc< PIXEL >(pDst, pSrc, GetWidth(), img.GetWidth(), x, y, left, top, width, height);
		}
		// 普通の処理(ピクセルをそのまま上書きする)
		else{
			unsigned int linesize = sizeof(PIXEL) * width;
			while(height-->0){
				memcpy(pDst, pSrc, linesize);
				pSrc += img.GetWidth();
				pDst += GetWidth();
			}
			/*
			for(unsigned int j = 0; j < height; j++){
				for(unsigned int i = 0; i < width; i++){
					SetPixel(x+i, y+j, img.GetPixel(left+i, top+j));
				}
			}
			*/
		}

		return true;
	}

	//=============================================================================
	/**
		拡大/縮小した画像を取得する

		@param	nZoom		[in] 拡大率(％指定)

		@return				拡大/縮小した後の画像
	*/
	//=============================================================================
	CBasicImage< PIXEL > Zoom(int nZoom) const
	{
		// 同じ
		if(nZoom==100)		return *this;

		return Zoom((Float)nZoom/100.0);
	}
	//=============================================================================
	/**
		拡大/縮小した画像を取得する

		@param	fZoom		[in] 拡大率(実数指定)

		@return				拡大/縮小した後の画像
	*/
	//=============================================================================
	CBasicImage< PIXEL > Zoom(Float fZoom) const
	{
		CBasicImage< PIXEL > img;
		ImgLength x, y;

		// 同じ
		if(fZoom==1.0)		return *this;

		if(IsInit()){
			img.Create((ImgLength)(GetWidth()*fZoom), (ImgLength)(GetHeight()*fZoom));
			img.CopyPalette(*this);
			for(y = 0; y < img.GetHeight(); y++){
				for(x = 0; x < img.GetWidth(); x++){
					img.SetPixel(x, y, GetPixel((ImgLength)(x/fZoom),(ImgLength)(y/fZoom)));
				}
			}
		}

		return img;
	}
};

typedef CBasicImage< unsigned char  >		CImage8;	///< 8bits  color only
typedef CBasicImage< unsigned short >		CImage16;	///< 16bits color only
typedef CBasicImage< sueLib::RGBTRIPLE >	CImage24;	///< 24bits color only
//typedef CBasicImage< RGBTRIPLE >			CImage24;	///< 24bits color only
typedef CBasicImage< unsigned long  >		CImage32;	///< 32bits color only

} // end of namespace sueLib

#endif
