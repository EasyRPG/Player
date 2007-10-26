/**
	@file
	@brief		バッファからイベントを読むためのストリーム
	@author		sue445
*/
#ifndef _INC_CRPGEVENTSTREAM
#define _INC_CRPGEVENTSTREAM

#include "CRpgIOBase.h"
#include "CRpgEvent.h"


/// バッファからイベントを読むためのストリーム
class CRpgEventStream : public CRpgIOBase{
public:
	CRpgEventStream(){}					///< コンストラクタ
	~CRpgEventStream(){}				///< デストラクタ

	bool ReadEvent(CRpgEvent& event);	///< イベントを1つ読み込む
};

#endif
