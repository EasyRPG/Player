/**
	@file
	@brief		イベントデータ1つ分
	@author		sue445
*/
#ifndef _INC_CRPGEVENT
#define _INC_CRPGEVENT

#include "CRpgIOBase.h"
#include <deque>

using std::deque;

// イベントコード(ツクール内で使用)
// 操作系
#define CODE_OPERATE_SWITCH		0x27e2	///< スイッチ
#define CODE_OPERATE_VAR		0x27ec	///< 変数
#define CODE_OPERATE_ITEM		0x2850	///< アイテム
#define CODE_OPERATE_KEY		0x2d5a	///< キー
#define CODE_OPERATE_INPUT		0x27A6	///< 数値入力
#define CODE_OPERATE_TIMER		0x27F6	///< タイマー
#define CODE_OPERATE_MONEY		0x2846	///< 所持金
// パーティ
#define CODE_PARTY_CHANGE		0x285a	///< パーティ変更
#define CODE_PARTY_HP			0x28dc	///< HP
#define CODE_PARTY_MP			0x28E6	///< MP
#define CODE_PARTY_STATE		0x28f0	///< 状態変更
#define CODE_PARTY_SKILL		0x28c8	///< 特技
#define CODE_PARTY_REFRESH		0x28fa	///< 回復
#define CODE_PARTY_EXP			0x28AA	///< 経験値
#define CODE_PARTY_LV			0x28B4	///< レベル
#define CODE_PARTY_POWER		0x28BE	///< 能力値
#define CODE_PARTY_SOUBI		0x28D2	///< 装備
#define CODE_PARTY_DAMAGE		0x2904	///< ダメージ処理
#define CODE_PARTY_NAME			0x2972	///< 名前
#define CODE_PARTY_TITLE		0x297C	///< 肩書き
#define CODE_PARTY_WALK			0x2986	///< 歩行絵
#define CODE_PARTY_FACE			0x2990	///< 顔絵
// システム
#define CODE_SYSTEM_VEHICLE		0x299A	///< 乗り物絵変更
#define CODE_SYSTEM_BGM			0x29a4	///< システムBGM
#define CODE_SYSTEM_SOUND		0x29AE	///< システム効果音
#define CODE_SYSTEM_GRAPHIC		0x29B8	///< システムグラフィック
#define CODE_SYSTEM_SCREEN		0x29C2	///< 画面切り替え方式
// 乗り物
#define CODE_VEH_RIDE			0x2A58	///< 乗降
#define CODE_VEH_LOCATE			0x2A62	///< 位置設定
// 構文
#define CODE_BLOCK_END			0x000a	///< Block
#define CODE_IF_START			0x2eea	///< if
#define CODE_IF_ELSE			0x55fa	///< else
#define CODE_IF_END				0x55fb	///< end if
#define CODE_LOOP_START			0x2fb2	///< loop
#define CODE_LOOP_BREAK			0x2fbc	///< break
#define CODE_LOOP_END			0x56c2	///< loop end
#define CODE_GOTO_MOVE			0x2f58	///< goto
#define CODE_GOTO_LABEL			0x2f4e	///< goto label
#define CODE_SELECT_START		0x279c	///< select case
#define CODE_SELECT_CASE		0x4eac	///< case n
#define CODE_SELECT_END			0x4ead	///< end select
#define CODE_SHOP				0x29E0	///< お店
#define CODE_SHOP_IF_START		0x50F0	///< お店(売買した)
#define CODE_SHOP_IF_ELSE		0x50F1	///< お店(売買しない)
#define CODE_SHOP_IF_END		0x50F2	///< お店(End If)
#define CODE_INN				0x29EA	///< 宿屋
#define CODE_INN_IF_START		0x50FA	///< 宿屋(宿泊した)
#define CODE_INN_IF_ELSE		0x50FB	///< 宿屋(宿泊しない)
#define CODE_INN_IF_END			0x50FC	///< 宿屋(End IF)
// イベント
#define CODE_EVENT_BREAK		0x3016	///< イベント中断
#define CODE_EVENT_CLEAR		0x3020	///< 一時消去
#define CODE_EVENT_GOSUB		0x302a	///< サブルーチン(イベントのよびだし)
// マルチメディア
#define CODE_MM_SOUND			0x2d1e	///< 効果音
#define CODE_MM_BGM_PLAY		0x2cf6	///< BGM再生
#define CODE_MM_BGM_FADEOUT		0x2d00	///< BGMフェードアウト
#define CODE_MM_BGM_SAVE		0x2D0A	///< BGM記憶
#define CODE_MM_BGM_LOAD		0x2D14	///< 記憶したBGMを再生
#define CODE_MM_MOVIE			0x2D28	///< ムービー
// 画面
#define CODE_SCREEN_CLEAR		0x2b02	///< 消去
#define CODE_SCREEN_SHOW		0x2b0c	///< 表示
#define CODE_SCREEN_COLOR		0x2b16	///< 色調変更
#define CODE_SCREEN_SHAKE		0x2b2a	///< シェイク
#define CODE_SCREEN_FLASH		0x2b20	///< フラッシュ
#define CODE_SCREEN_SCROLL		0x2B34	///< スクロール
#define CODE_SCREEN_WEATHER		0x2B3E	///< 天気
// ピクチャ
#define CODE_PICT_SHOW			0x2b66	///< 表示
#define CODE_PICT_MOVE			0x2b70	///< 移動
#define CODE_PICT_CLEAR			0x2b7a	///< 消去
// キャラ
#define CODE_CHARA_TRANS		0x2c2e	///< 透明状態変更
#define CODE_CHARA_MOVE			0x2c42	///< 移動
#define CODE_CHARA_FLASH		0x2C38	///< フラッシュ
#define CODE_MOVEALL_START		0x2C4C	///< 指定動作の全実行
#define CODE_MOVEALL_CANSEL		0x2C56	///< 指定動作の全実行をキャンセル
// 位置
#define CODE_LOCATE_MOVE		0x2a3a	///< 場所移動(固定値)
#define CODE_LOCATE_SAVE		0x2a44	///< 現在地保存
#define CODE_LOCATE_LOAD		0x2a4e	///< 記憶した場所に移動(変数)
// テキスト・注釈
#define CODE_TXT_REM			0x307a	///< 注釈
#define CODE_TXT_REM_ADD		0x578a	///< 注釈追加
#define CODE_TXT_SHOW			0x277e	///< 本文
#define CODE_TXT_SHOW_ADD		0x4e8e	///< 本文追加
#define CODE_TXT_OPTION			0x2788	///< 文章表示オプション
#define CODE_TXT_FACE			0x2792	///< 顔CG変更
// その他
#define CODE_NAME_INPUT			0x29F4	///< 名前入力
#define CODE_EVENT_LOCATE		0x2A6C	///< イベント位置設定
#define CODE_EVENT_SWAP			0x2A76	///< イベント位置交換
#define CODE_LAND_ID			0x2A9E	///< 地形ID取得
#define CODE_EVENT_ID			0x2AA8	///< イベントID取得
#define CODE_WAIT				0x2c92	///< ウェイト
#define CODE_CHIPSET			0x2DBE	///< チップセット
#define CODE_PANORAMA			0x2DC8	///< 遠景
#define CODE_ENCOUNT			0x2DDC	///< エンカウント
#define CODE_CHIP_CONVERT		0x2DE6	///< チップ置換
#define CODE_TELEPORT			0x2E22	///< テレポート増減
#define CODE_TELEPORT_PERM		0x2E2C	///< テレポート禁止
#define CODE_ESCAPE				0x2E36	///< エスケープ位置
#define CODE_ESCAPE_PERM		0x2E40	///< エスケープ禁止
#define CODE_SAVE_SHOW			0x2E86	///< セーブ画面
#define CODE_SAVE_PERM			0x2E9A	///< セーブ禁止
#define CODE_MENU_SHOW			0x2eae	///< メニュー表示
#define CODE_MENU_PERM			0x2EB8	///< メニュー禁止
#define CODE_LABEL				0x2F4E	///< ラベルの設定
#define CODE_GAMEOVER			0x3084	///< ゲームオーバー
#define CODE_TITLE				0x30DE	///< タイトルに戻る
#define CODE_BTLANIME			0x2BCA	///< 戦闘アニメ(非戦闘)
// 戦闘系
#define CODE_BTL_GO_START		0x29d6	///< 戦闘分岐
#define CODE_BTL_GO_WIN			0x50e6	///< 勝った時
#define CODE_BTL_GO_ESCAPE		0x50e7	///< 逃げた時
#define CODE_BTL_GO_LOSE		0x50e8	///< 負けた時
#define CODE_BTL_GO_END			0x50e9	///< 分岐終了
#define CODE_BTL_ANIME			0x33cc	///< 戦闘アニメ
// ツクール2003追加分
#define CODE_2003_JOB			0x03F0	///< 職業変更
#define CODE_2003_BTL_CMD		0x03F1	///< 戦闘コマンド
#define CODE_2003_ATK_REPEAT	0x03EF	///< 連続攻撃
#define CODE_2003_ESCAPE100		0x03EE	///< 100%脱出
#define CODE_2003_BTL_COMMON	0x03ED	///< バトルイベントからコモンよびだし

////////////////////////////////////////// バトルイベント
#define CODE_BTL_BACKGROUND		0x339a	///< 背景変更
#define CODE_BTL_STOP			0x3462	///< 戦闘中断
// 敵
#define CODE_BTL_ENEMY_HP		0x3336	///< HP操作
#define CODE_BTL_ENEMY_MP		0x3340	///< MP操作
#define CODE_BTL_ENEMY_STATE	0x334a	///< 状態変更
#define CODE_BTL_ENEMY_APPEAR	0x335e	///< 出現
// 戦闘構文
#define CODE_BTL_IF_START		0x33fe	///< if
#define CODE_BTL_IF_ELSE		0x5b0e	///< ※一緒
#define CODE_BTL_IF_END			0x5b0f	///< ※一緒


/// イベントデータ1つ分を管理するクラス
class CRpgEvent{
public:
	int m_nEventCode;			///< 命令の種類(ツクール内コード)
	int m_nNest;				///< ネスト数
	int	m_nParamMax;			///< 引数の最大数
	deque<int>	m_deqParam;		///< 引数(先入れ先出しなので本当はqueueで十分なんだけどqueueだとclearがないのでdecueを使うｗ)
	string		m_str;			///< イベントに割り当てられた文字列

public:
	CRpgEvent():
		m_nEventCode(0),
		m_nNest(0),
		m_nParamMax(0){}							///< コンストラクタ

	~CRpgEvent(){}									///< デストラクタ

	bool GetParam(int* lpData=NULL);				///< 引数を1つ取得

	int GetEventCode(){		return m_nEventCode; }	///< イベントコードを取得
	int GetNest(){			return m_nNest; }		///< イベントのネスト数を取得
	const char* GetText(){	return m_str.c_str(); }	///< イベントに割り当てられた文字列を取得
};

#endif
