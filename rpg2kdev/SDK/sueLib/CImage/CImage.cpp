/**
	@file
	@brief		画像のピクセルデータを管理するクラス
	@author		sue445
*/
#include "CImage.h"

namespace sueLib {

/// Alphaを取り出す
unsigned char Alpha(unsigned long rgba)
{
	return (unsigned char)((rgba & 0xFF000000)>>24);
}
/// Redを取り出す
unsigned char Red(unsigned long rgba)
{
	return (unsigned char)((rgba & 0x00FF0000)>>16);
}
/// Greenを取り出す
unsigned char Green(unsigned long rgba)
{
	return (unsigned char)((rgba & 0x0000FF00)>>8);
}
/// Blueを取り出す
unsigned char Blue(unsigned long rgba)
{
	return (unsigned char)(rgba & 0x000000FF);
}
/// α合成
unsigned char CalcAlpha(unsigned char src, unsigned char dst, unsigned char alpha)
{
	// 参考文献：Windowsプロフェッショナルゲームプログラミング2(やねうらお著) P303
	return ((dst<<8)-dst + (src-dst)*alpha + 255)>>8;
}
/// α合成(不透明度も考慮)
unsigned char CalcAlpha2(unsigned char src, unsigned char dst, unsigned char alpha, unsigned char opacity)
{
	return CalcAlpha(CalcAlpha(src,dst,alpha), dst, opacity);
}

} // end of namespace sueLib
