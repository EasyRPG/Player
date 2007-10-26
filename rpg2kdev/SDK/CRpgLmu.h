/**
	@file
	@brief		Map????.lmu (LcfMapUnit) class manager
	@author		sue445
*/
#ifndef _INC_CRPGLMU
#define _INC_CRPGLMU

#include "CRpgIOBase.h"
#include "CRpgEvent.h"
#include "CRpgImage.h"
#include "CRpgLdb.h"


const int CHIP_SIZE = 16;	///< Tile size

/// Map????.lmu (LcfMapUnit) class manager
class CRpgLmu : public CRpgIOBase{
private:
	const char* GetHeader(){ return "LcfMapUnit"; }	///< specific file header (overrides CRpgIOBase default empty return value)

	int m_nChipSet;		///< 0x01: Tileset number (initial value: 1)
	int m_nWidth;		///< 0x02: Map width (initial value: 20)
	int m_nHeight;		///< 0x03: Map height (initial value: 15)
	string	m_strPanorama;	///< 0x20: Panorama file name (initial value: "")

	// Map Tileset (Respecting to internal data, it's referring by [y][x])
	smart_array< smart_array< unsigned short > >	m_saUpper;	///< 0x47: Upper layer
	smart_array< smart_array< unsigned short > >	m_saLower;	///< 0x48: Lower layer

	CRpgImage imgChipSet;	///< Tileset picture
	CRpgImage imgPanorama;	///< Panorama picture

public:
	// Each map event
	struct MAPEVENT{
		MAPEVENT(): x(0), y(0){}	///< Default constructor

		string strName;	///< 0x01: Name
		int  x;		///< 0x02: X coordinate
		int  y;		///< 0x03: Y coordinate
		int  data;	///< 0x04: Unknown (O_O)

		/// Map event in a page unit
		struct MAPEVENT_PAGE{
			MAPEVENT_PAGE():
				nWalkPos(0), nWalkMuki(0), nWalkPattern(1), bWalkHalf(false), nStart(0){}	///< Constructor

			struct EVENTAPPEAR{
				EVENTAPPEAR():
					nSw1(1), nSw2(1), nVarNum(1), nVarOver(0), 
					nItem(1), nChara(1), nTimer(0){}	///< Constructor

				/**
					Bit 0: Switch [%04d:%s] ON (1)\n
					Bit 1: Switch [%04d:%s] ON (2)\n
					Bit 2: Variable [%04d:%s] above [%d]\n
					Bit 3: Item [%04d:%s] in possession\n
					Bit 4: Actor [%04d:%s] in party\n
					Bit 5: Timer [%02d:%02d] below
				*/
				int nFlag;	///< 0x01: Checkbox state (6 bits flag)

				int nSw1;	///< 0x02: Switch (1)	Initial value :1
				int nSw2;	///< 0x03: Switch (2)	Initial value :1
				int nVarNum;	///< 0x04: Variable number	Initial value :1
				int nVarOver;	///< 0x05: Variable - above
				int nItem;	///< 0x06: Item - in posession	Initial value :1
				int nChara;	///< 0x07: Actor - in party	Initial value :1
				int nTimer;	///< 0x08: Timer (in seconds)

			} EventAppear;		///< 0x02: Appearance conditions

			// Actor graphic related
			string	 strWalk;	///< 0x15: File name

			/**
				Actor graphic part: 0-7 starting at upper left\n
				Tileset part: Upper layer map position\n
				*Judge whether a walk picture or the upper layer map have a file name
			*/
			int nWalkPos;		///< 0x16: Actor graphic (in upper layer map) orientation

			/**
				0:Up\n
				1:Right\n
				2:Down\n
				3:Left
			*/
			int nWalkMuki;		///< 0x17: Actor graphic walking frame

			/**
				0: Left\n
				1: Middle (initial value?)\n
				2: Right
			*/
			int nWalkPattern;	///< 0x18: Walking pattern

			bool bWalkHalf;		///< 0x19: Transparency status

			/**
				0: On button key press\n
				1: On actor touch\n
				2: On event touch\n
				3: Automatic start\n
				4: Parallel process
			*/
			int nStart;			///< 0x21: Start conditions

//			int nEventDataSize;		///< 0x33: Auto start
			vector< CRpgEvent > vctEvent;	///< 0x34: Event data block

		};	// end of struct MAPEVENT_PAGE
		smart_array< MAPEVENT_PAGE > saPage;	///< 0x05: Map event (per event page)

	};	// end of struct MAPEVENT
	smart_array< MAPEVENT >	saMapEvent;		///< Map event package (name, position, pages)

public:
	CRpgLmu(){}	///< Constructor
	~CRpgLmu(){}	///< Destructor

	bool Init(int nMapNum, const CRpgLdb& ldb, const char* szDir="");	///< Initialization
	bool GetUpperChip(int x, int y, CRpgImage& img) const;			///< Gets the upper layer map
	bool GetLowerChip(int x, int y, int anime, CRpgImage& img) const;	///< Gets the lower layer map

	int GetChipSet()	const{ return m_nChipSet; }	///< Gets tileset ID
	int GetWidth()		const{ return m_nWidth;   }	///< Gets map width
	int GetHeight()		const{ return m_nHeight;  }	///< Gets map height

private:
	void GetMapEvent(smart_buffer& bufSrc);					///< Gets map event
	void GetOceanType(unsigned short chip, bool bOcean[4]) const;		///< Gets sea type by tileset ID
	void GetChipPos(int nChip, bool bUpper, int* lpX, int* lpY) const;	///< Gets tileset position by tileset ID
};

#endif
