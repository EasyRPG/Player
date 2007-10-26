/**
	@file
	@brief		バッファからイベントを読むためのストリーム
	@author		sue445
*/
#include "CRpgEventStream.h"


//=============================================================================
/**
	ストリームからイベント1つ分を取得

	@param	event			[out] 取得したイベント

	@retval	true			イベントの取得に成功
	@retval	false			イベントの取得に失敗
*/
//=============================================================================
bool CRpgEventStream::ReadEvent(CRpgEvent& event)
{
	event.m_deqParam.clear();
	event.m_str = "";

	if(IsEof())		return false;

	// イベントコード
	event.m_nEventCode = ReadBerNumber();
	if(event.m_nEventCode==0)	return false;

	// ネスト数
	event.m_nNest = ReadBerNumber();

	// テキスト
	event.m_str = ReadString();

	// 引数の数
	event.m_nParamMax = ReadBerNumber();
	int max = event.m_nParamMax;
	while(max-->0){
		event.m_deqParam.push_back( ReadBerNumber() );
	}

	return true;
}
