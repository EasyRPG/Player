/**
	@file
	@brief		イベントデータ1つ分
	@author		sue445
*/
#include "CRpgEvent.h"


//=============================================================================
/**
	イベント内の引数を取得

	@param	lpData			[out] 取得した引数

	@retval	true			引数の取得に成功
	@retval	false			引数の取得に失敗／引数スタックが空
*/
//=============================================================================
bool CRpgEvent::GetParam(int* lpData)
{
	if(m_deqParam.empty())	return false;

	if(lpData)	*lpData = m_deqParam.front();
	m_deqParam.pop_front();

	return true;
}
