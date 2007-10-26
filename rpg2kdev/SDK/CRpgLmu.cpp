/**
	@file
	@brief		Map????.lmu (LcfMapUnit) class manager
	@author		sue445
*/
#include "CRpgLmu.h"
#include "CRpgEventStream.h"
#include "sueLib/CImage/CBmpIO.h"


//=============================================================================
/**
	Initialization

	@param	nMapNum		[in] Map file number (1...)
	@param	szDir		[in] Reading folder

	@retval	true		Successfully reading map file
	@retval	false		Failed reading map file
*/
//=============================================================================
bool CRpgLmu::Init(int nMapNum, const CRpgLdb& ldb, const char* szDir)
{
	int type;
	char file[256];
	smart_buffer buf;
	string strFile;


	// Unexpected number of map
	if(nMapNum<1)			return false;

	// ldb not initialized
	if(!ldb.IsInit())		return false;

	sprintf(file, "Map%04d.lmu", nMapNum);

	bInit = false;
	strFile = "";
	if(strlen(szDir)){
		strFile += szDir;
		strFile += "\\";
	}
	strFile += file;

	// No save data
	if(!OpenFile(strFile.c_str()))		return false;

	// Initial values (These values are not defined in lmu, they're set)
	m_nChipSet = 1;
	m_nWidth   = 20;
	m_nHeight  = 15;
	m_strPanorama = "";

	// Data reading
	while(!IsEof()){
		type = ReadBerNumber();
		buf = ReadData();

		switch(type){
		case 0x01:		// Tileset
			m_nChipSet = CRpgUtil::GetBerNumber(buf);
			break;
		case 0x02:		// Width
			m_nWidth = CRpgUtil::GetBerNumber(buf);
			break;
		case 0x03:		// Height
			m_nHeight = CRpgUtil::GetBerNumber(buf);
			break;
		case 0x20:		// Panorama file name
			m_strPanorama = CRpgUtil::GetString(buf);
			break;
		case 0x47:		// Lower layer map
			BufferToArray2(m_saLower, buf, m_nWidth, m_nHeight);
			break;
		case 0x48:		// Upper layer map
			BufferToArray2(m_saUpper, buf, m_nWidth, m_nHeight);
			break;
		case 0x51:		// Map event block
			GetMapEvent(buf);
			break;
		}
	}

	// Chipset loading
	strFile = "";
	if(strlen(szDir)){
		strFile += szDir;
		strFile += "\\";
	}
	strFile += "ChipSet\\";
	strFile += ldb.saChipSet[m_nChipSet].strFile;
	imgChipSet.Load(strFile.c_str());

	// Panorama loading
	strFile = "";
	if(strlen(szDir)){
		strFile += szDir;
		strFile += "\\";
	}
	strFile += "Panorama\\";
	strFile += m_strPanorama;
	imgPanorama.Load(strFile.c_str());

	bInit = true;
	return true;
}

//=============================================================================
/**
	Getting map data

	@param	bufSrc		[in] raw data
*/
//=============================================================================
void CRpgLmu::GetMapEvent(smart_buffer& bufSrc)
{
	CRpgArray2	array;
	smart_buffer buf;


	array = CRpgUtil::GetArray2(bufSrc);
	int max_event = array.GetMaxRow();


	// 開始番号が1なので配列要素数も1増やしておいた方がやりやすい
	saMapEvent.Resize(max_event+1);
	for(int i = 1; i <= max_event; i++){
		saMapEvent[i].strName	= array.GetString(i, 0x01);
		saMapEvent[i].x			= array.GetNumber(i, 0x02);
		saMapEvent[i].y			= array.GetNumber(i, 0x03);
		saMapEvent[i].data		= array.GetNumber(i, 0x04);

		// Divides every page of map events
		CRpgArray2	array2a = CRpgUtil::GetArray2(array.GetData(i, 0x05));
		int max_page = array2a.GetMaxRow();
		saMapEvent[i].saPage.Resize(max_page+1);

		for(int j = 1; j <= max_page; j++){
			// Event appearance conditions
			CRpgArray1 array1a = CRpgUtil::GetArray1(array2a.GetData(j, 0x02));
			saMapEvent[i].saPage[j].EventAppear.nFlag	= array1a.GetNumber(0x01);
			saMapEvent[i].saPage[j].EventAppear.nSw1	= array1a.GetNumber(0x02, 1);
			saMapEvent[i].saPage[j].EventAppear.nSw2	= array1a.GetNumber(0x03, 1);
			saMapEvent[i].saPage[j].EventAppear.nVarNum	= array1a.GetNumber(0x04, 1);
			saMapEvent[i].saPage[j].EventAppear.nVarOver	= array1a.GetNumber(0x05);
			saMapEvent[i].saPage[j].EventAppear.nItem	= array1a.GetNumber(0x06, 1);
			saMapEvent[i].saPage[j].EventAppear.nChara	= array1a.GetNumber(0x07, 1);
			saMapEvent[i].saPage[j].EventAppear.nTimer	= array1a.GetNumber(0x08);

			array2a.GetString(j, 0x15, saMapEvent[i].saPage[j].strWalk);
			saMapEvent[i].saPage[j].nWalkPos		= array2a.GetNumber(j, 0x16);
			saMapEvent[i].saPage[j].nWalkMuki		= array2a.GetNumber(j, 0x17);
			saMapEvent[i].saPage[j].nWalkPattern		= array2a.GetNumber(j, 0x18, 1);
			saMapEvent[i].saPage[j].bWalkHalf		= array2a.GetFlag(  j, 0x19, 1);
			saMapEvent[i].saPage[j].nStart			= array2a.GetNumber(j, 0x21, 1);

			// イベントデータを1行ずつvectorに入れる
			buf = array2a.GetData(j, 0x34);
			CRpgEventStream st;
			if(st.OpenFromMemory(buf, buf.GetSize())){
				CRpgEvent event;
				while(st.ReadEvent(event)){
					saMapEvent[i].saPage[j].vctEvent.push_back(event);
				}
			}
		}
	}
}

//=============================================================================
/**
	Gets upper layer map chip coordinates (x,y)

	@param	x				[in] x coordinate
	@param	y				[in] y coordinate
	@param	img				[out] got picture

	@retval	true			Successfully reading map chip
	@retval	false			Failed reading map chip
*/
//=============================================================================
bool CRpgLmu::GetUpperChip(int x, int y, CRpgImage& img) const
{
	if(x<0 || x >= m_nWidth || y<0 || y>= m_nHeight)	return false;

	unsigned short chip = m_saUpper[y][x];
	int cx, cy;

	img.Create(CHIP_SIZE, CHIP_SIZE);
	img.CopyPalette(imgChipSet);
	if(chip>=10000 && chip<10144){
		GetChipPos(chip, true, &cx, &cy);
		img.Blt(imgChipSet, 0, 0, cx<<4, cy<<4, CHIP_SIZE, CHIP_SIZE, false);
		return true;
	}

	return false;
}

//=============================================================================
/**
	Gets upper layer map chip coordinates (x,y)

	@param	x				[in] x coordinate
	@param	y				[in] y coordinate
	@param	anime			[in] Animation number
	@param	img				[out] acquired picture


	@retval	true			Successfully reading map chip
	@retval	false			Failed reading map chip
*/
//=============================================================================
bool CRpgLmu::GetLowerChip(int x, int y, int anime, CRpgImage& img) const
{
	if(x<0 || x >= m_nWidth || y<0 || y>= m_nHeight)	return false;

	int i, cx = 18, cy = 8, base_cx, base_cy;
	const unsigned short chip = m_saLower[y][x];
	const int nOffset[4][2] = {
		{0, 0},		// Up Left
		{8, 0},		// Up Right
		{0, 8},		// Down Left
		{8, 8},		// Down Right
	};
	img.Create(CHIP_SIZE, CHIP_SIZE);
	img.CopyPalette(imgChipSet);

	if(x==2 && y==6){
		printf("%02X\n", chip);
	}

	if(chip<3000){
		bool ocean_flag[4];

		/*
			0: Shoal (sea)
			1: The shoal which has deep sea and a boundary in four corners
			2: The deep sea which has the shoal and boundary in the four corners
			3: Deep sea
		*/
		int  ocean_type[4];

		GetOceanType(chip, ocean_flag);

		// Sea type analysis
		// 0...
		if(chip<1000){		// Coastline A1 (Normal land)
			base_cx = 0;
			base_cy = 0;
			for(i = 0; i < 4; i++){
				if(!ocean_flag[i])	ocean_type[i] = 0;
				else				ocean_type[i] = 1;
			}
		}
		// 1000...
		else if(chip<2000){	// Coastline A2 (Snow)
			base_cx = 3;
			base_cy = 0;
			for(i = 0; i < 4; i++){
				if(!ocean_flag[i])	ocean_type[i] = 0;
				else				ocean_type[i] = 1;
			}
		}
		// 2000...
		else if(chip<3000){	// Coastline A1 (Normal land)
			base_cx = 0;
			base_cy = 0;
			for(i = 0; i < 4; i++){
				if(!ocean_flag[i])	ocean_type[i] = 3;
				else				ocean_type[i] = 2;
			}
		}

		// Setting the sea as base
		for(i = 0; i < 4; i++){
			int x = ((anime % 3)      <<4) + nOffset[i][0];
			int y = ((ocean_type[i]+4)<<4) + nOffset[i][1];
			img.Blt(imgChipSet, nOffset[i][0], nOffset[i][1], x, y, CHIP_SIZE>>1, CHIP_SIZE>>1, false);
		}

		/*
			00: Sea, without coastline

			01: Coastline,   Up  Left
			02: Coastline,   Up Right
			04: Coastline, Down Right
			08: Coastline, Down  Left

			10: Coastline,  Left
			11: Coastline,  Left +   Up Right
			12: Coastline,  Left              + Down Right
			13: Coastline,  Left +   Up Right + Down Right

			14: Coastline,    Up
			15: Coastline,    Up + Down Right
			16: Coastline,    Up              + Down Left
			17: Coastline,    Up + Down Right + Down Left

			18: Coastline, Right
			19: Coastline, Right + Down  Left
			1A: Coastline, Right              +   Up Left
			1B: Coastline, Right + Down  Left +   Up Left

			1C: Coastline,  Down
			1D: Coastline,  Down +   Up  Left
			1E: Coastline,  Down              +   Up Right
			1F: Coastline,  Down +   Up  Left +   Up Right

			20: Coastline, Left + Right
			21: Coastline,   Up + Down

			A B
			C D
			22: Coastline, A B C
			23: Coastline, A B C   + Down Right
			24: Coastline, A B   D
			25: Coastline, A B   D + Down Left
			26: Coastline,   B C D
			27: Coastline,   B C D +   Up Left
			28: Coastline, A   C D
			29: Coastline, A   C D +   Up Right
			2A: Coastline, C A B D
			2B: Coastline, B A C D
			2C: Coastline, A C D B
			2D: Coastline, A B D C
			2E: Coastline, circle (A B C D)
		*/
		int chip2 = chip%50;

		enum{
			A,	// Four edges (neighbors)
			B,	// Up Down
			C,	// Left Right
			D,	// Four corners

			OCEAN,	// Without coastline (sea lower map chip is set in this form)
		};
		int nChip[4] = {OCEAN, OCEAN, OCEAN, OCEAN};

		if(chip2<0x10){
			bool flag[8];
			ByteToBool((unsigned char)chip2, flag);
			if(flag[0])		nChip[0] = D;	//   Up Left
			if(flag[1])		nChip[1] = D;	//   Up Right
			if(flag[2])		nChip[3] = D;	// Down Right
			if(flag[3])		nChip[2] = D;	// Down Left
		}
		// 0x10...
		else if(chip2<0x14){
			nChip[0] = nChip[2] = B;
			if(chip2 & 0x01)	nChip[1] = D;
			if(chip2 & 0x02)	nChip[3] = D;
		}
		// 0x14...
		else if(chip2<0x18){
			nChip[0] = nChip[1] = C;
			if(chip2 & 0x01)	nChip[3] = D;
			if(chip2 & 0x02)	nChip[2] = D;
		}
		// 0x18...
		else if(chip2<0x1C){
			nChip[1] = nChip[3] = B;
			if(chip2 & 0x01)	nChip[2] = D;
			if(chip2 & 0x02)	nChip[0] = D;
		}
		// 0x1C...
		else if(chip2<0x20){
			nChip[2] = nChip[3] = C;
			if(chip2 & 0x01)	nChip[0] = D;
			if(chip2 & 0x02)	nChip[1] = D;
		}
		else if(chip2==0x20){
			nChip[0] = nChip[1] = nChip[2] = nChip[3] = B;
		}
		else if(chip2==0x21){
			nChip[0] = nChip[1] = nChip[2] = nChip[3] = C;
		}
		else if(chip2<=0x23){
			nChip[0] = A;
			nChip[2] = B;
			nChip[1] = C;
			if(chip2 & 0x01)	nChip[3] = D;
		}
		else if(chip2<=0x25){
			nChip[1] = A;
			nChip[3] = B;
			nChip[0] = C;
			if(chip2 & 0x01)	nChip[2] = D;
		}
		else if(chip2<=0x27){
			nChip[3] = A;
			nChip[1] = B;
			nChip[2] = C;
			if(chip2 & 0x01)	nChip[0] = D;
		}
		else if(chip2<=0x29){
			nChip[2] = A;
			nChip[0] = B;
			nChip[3] = C;
			if(chip2 & 0x01)	nChip[1] = D;
		}
		else if(chip2==0x2A){
			nChip[0] = nChip[1] = A;
			nChip[2] = nChip[3] = B;
		}
		else if(chip2==0x2B){
			nChip[0] = nChip[2] = A;
			nChip[1] = nChip[3] = C;
		}
		else if(chip2==0x2C){
			nChip[2] = nChip[3] = A;
			nChip[0] = nChip[1] = B;
		}
		else if(chip2==0x2D){
			nChip[1] = nChip[3] = A;
			nChip[0] = nChip[2] = C;
		}
	//	else if(chip2==0x2E){
		else{
			nChip[0] = nChip[1] = nChip[2] = nChip[3] = A;
		}

		// Draws coastline
		for(i = 0; i < 4; i++){
			if(nChip[i] != OCEAN){
				int x = ((base_cx + anime % 3)<<4) + nOffset[i][0];
				int y = ((base_cy + nChip[i]) <<4) + nOffset[i][1];
				img.Blt(imgChipSet, nOffset[i][0], nOffset[i][1], x, y, CHIP_SIZE>>1, CHIP_SIZE>>1, false);
			}
		}

		return true;
	}
	// 3000...(3028)
	else if(chip<3050){
		// C1
		cx = 3;
		cy = 4;
		cy += anime % 4;
	}
	// 3050...(3078)
	else if(chip<3100){
		// C2
		cx = 4;
		cy = 4;
		cy += anime % 4;
	}
	// 3100...(3128)
	else if(chip<3150){
		// C3
		cx = 5;
		cy = 4;
		cy += anime % 4;
	}
	// 3150...
	else if(chip<3200){
		// D1
		cx = 0;
		cy = 8;
	}
	// 3200...
	else if(chip<3250){
		// D2
		cx = 3;
		cy = 8;
	}
	// 3250...
	else if(chip<3300){
		// D3
		cx = 0;
		cy = 12;
	}
	// 3300...
	else if(chip<3350){
		// D4
		cx = 3;
		cy = 12;
	}
	// 3350...
	else if(chip<3400){
		// D5
		cx = 6;
		cy = 0;
	}
	// 3400...
	else if(chip<3450){
		// D6
		cx = 9;
		cy = 0;
	}
	// 3450...
	else if(chip<3500){
		// D7
		cx = 6;
		cy = 4;
	}
	// 3500...
	else if(chip<3550){
		// D8
		cx = 9;
		cy = 4;
	}
	// 3550...
	else if(chip<3600){
		// D9
		cx = 6;
		cy = 8;
	}
	// 3600...
	else if(chip<3650){
		// D10
		cx = 9;
		cy = 8;
	}
	// 3650...
	else if(chip<3700){
		// D11
		cx = 6;
		cy = 12;
	}
	// 3700...
	else if(chip<3750){
		// D12
		cx = 9;
		cy = 12;
	}
	// 3750...
	else if(chip<4000);	// Probably it does not exist
	// 4000...
	else if(chip<5000){
		// Relative position (A...D)
		enum{
			A,
			B,
			C,
			D7,	// D enumeration is equivalent to numeric keypad positions
			D8,
			D9,
			D4,
			D5,
			D6,
			D1,
			D2,
			D3,
		};
		// Relative position -> relative coordinate
		static const int nOffset2[12][2] = {
			{0, 0},
			{1, 0},
			{2, 0},
			{0, 1},
			{1, 1},
			{2, 1},
			{0, 2},
			{1, 2},
			{2, 2},
			{0, 3},
			{1, 3},
			{2, 3},
		};

		// 4000...
		if(chip<4050){
			// D1
			base_cx = 0;
			base_cy = 8;
		}
		// 4050...
		else if(chip<4100){
			// D2
			base_cx = 3;
			base_cy = 8;
		}
		// 4100...
		else if(chip<4150){
			// D3
			base_cx = 0;
			base_cy = 12;
		}
		// 4150...
		else if(chip<4200){
			// D4
			base_cx = 3;
			base_cy = 12;
		}
		// 4200...
		else if(chip<4250){
			// D5
			base_cx = 6;
			base_cy = 0;
		}
		// 4250...
		else if(chip<4300){
			// D6
			base_cx = 9;
			base_cy = 0;
		}
		// 4300...
		else if(chip<4350){
			// D7
			base_cx = 6;
			base_cy = 4;
		}
		// 4350...
		else if(chip<4400){
			// D8
			base_cx = 9;
			base_cy = 4;
		}
		// 4400...
		else if(chip<4450){
			// D9
			base_cx = 6;
			base_cy = 8;
		}
		// 4450...
		else if(chip<4500){
			// D10
			base_cx = 9;
			base_cy = 8;
		}
		// 4500...
		else if(chip<4550){
			// D11
			base_cx = 6;
			base_cy = 12;
		}
		// 4550...
		else if(chip<4600){
			// D12
			base_cx = 9;
			base_cy = 12;
		}

		/*
			00: Center

			01: Four corners,   Up  Left
			02: Four corners,   Up Right
			04: Four corners, Down Right
			08: Four corners, Down  Left

			10: Set, Left
			11: Set, Left + Up Right
			12: Set, Left            + Down Right
			13: Set, Left + Up Right + Down Right

			14: Set, Up
			15: Set, Up + Down Right
			16: Set, Up +            + Down Left
			17: Set, Up + Down Right + Down Left

			18: Set, Right
			19: Set, Right + Down Left
			1A: Set, Right +           + Up Left
			1B: Set, Right + Down Left + Up Left

			1C: Set, Down
			1D: Set, Down + Up Left
			1E: Set, Down +         + Up Right
			1F: Set, Down + Up Left + Up Right

			20: Set, Left + Right
			21: Set,   Up +  Down

			// Now, standard type terrain chip ^||^
			22: Set, Up Left
			23: Set, Up Left + Four corners, Down Right

			24: Set, Up Right
			25: Set, Up Right + Four corners, Down Left

			26: Set, Down Right
			27: Set, Down Right + Four corners, Up Left

			28: Set, Down Left
			29: Set, Down Left + Four corners, Up Right

			2A: Set (  Up  Left +   Up Right)
			2B: Set (  Up  Left + Down  Left)
			2C: Set (Down  Left + Down Right)
			2D: Set (  Up Right + Down Right)
			2E: Set (Up Left + Up Right + Down Left + Down Right)
		*/
		int nChip[4] = {D5, D5, D5, D5};

		int chip2 = chip % 50;
		// 0x00...
		if(chip2<0x10){
			bool flag[8];
			ByteToBool((unsigned char)chip2, flag);
			for(i = 0; i < 4; i++){
				if(flag[i])		nChip[i] = C;
			}
		}
		// 0x10...
		else if(chip2<0x14){
			nChip[0] = nChip[2] = D4;
			if(chip2 & 0x01)	nChip[1] = C;
			if(chip2 & 0x02)	nChip[3] = C;
		}
		// 0x14...
		else if(chip2<0x18){
			nChip[0] = nChip[1] = D8;
			if(chip2 & 0x01)	nChip[3] = C;
			if(chip2 & 0x02)	nChip[2] = C;
		}
		// 0x18...
		else if(chip2<0x1C){
			nChip[1] = nChip[3] = D8;
			if(chip2 & 0x01)	nChip[2] = C;
			if(chip2 & 0x02)	nChip[0] = C;
		}
		// 0x1C...
		else if(chip2<0x20){
			nChip[2] = nChip[3] = D2;
			if(chip2 & 0x01)	nChip[0] = C;
			if(chip2 & 0x02)	nChip[1] = C;
		}
		else if(chip2==0x20){
			nChip[0] = nChip[2] = D4;
			nChip[1] = nChip[3] = D6;
		}
		else if(chip2==0x21){
			nChip[0] = nChip[1] = D8;
			nChip[2] = nChip[3] = D2;
		}
		else if(chip2<=0x23){
			nChip[0] = nChip[1] = nChip[2] = nChip[3] = D7;
			if(chip2 & 0x01)	nChip[3] = C;
		}
		else if(chip2<=0x25){
			nChip[0] = nChip[1] = nChip[2] = nChip[3] = D9;
			if(chip2 & 0x01)	nChip[2] = C;
		}
		else if(chip2<=0x27){
			nChip[0] = nChip[1] = nChip[2] = nChip[3] = D3;
			if(chip2 & 0x01)	nChip[0] = C;
		}
		else if(chip2<=0x29){
			nChip[0] = nChip[1] = nChip[2] = nChip[3] = D1;
			if(chip2 & 0x01)	nChip[1] = C;
		}
		else if(chip2==0x2A){
			nChip[0] = nChip[2] = D7;
			nChip[1] = nChip[3] = D9;
		}
		else if(chip2==0x2B){
			nChip[0] = nChip[1] = D7;
			nChip[2] = nChip[3] = D1;
		}
		else if(chip2==0x2C){
			nChip[0] = nChip[2] = D1;
			nChip[1] = nChip[3] = D3;
		}
		else if(chip2==0x2D){
			nChip[0] = nChip[1] = D9;
			nChip[2] = nChip[3] = D3;
		}
	//	else if(chip2==0x2E){
		else{
			nChip[0] = D7;
			nChip[1] = D9;
			nChip[2] = D1;
			nChip[3] = D3;
		}

		// Draws the chip
		for(i = 0; i < 4; i++){
			int x = ( (base_cx+nOffset2[nChip[i]][0])<<4 ) + nOffset[i][0];
			int y = ( (base_cy+nOffset2[nChip[i]][1])<<4 ) + nOffset[i][1];
			img.Blt(imgChipSet, nOffset[i][0], nOffset[i][1], x, y, CHIP_SIZE>>1, CHIP_SIZE>>1, false);
		}

		return true;
	}
	// 5000...
	else{	// E Block, standard chip
		GetChipPos(chip, false, &cx, &cy);
	}


	// sets coordinates cx,cy (These chips are not divided by four)
	img.Blt(imgChipSet, 0, 0, cx<<4, cy<<4, CHIP_SIZE, CHIP_SIZE, false);

	return true;
}

//=============================================================================
/**
	Gets sea type (Order: Up Left, Up Right, Down Left, Down Right)

	@note					
							

	@param	chip	[in]	Chip number
	@param	bOcean	[out]	Shoal (sea) case (false: Shoal / true: The shoal which has the deep sea and boundary in the four corners)\n
				Deep sea case (false: Deep sea / true: The deep sea which has the shoal and boundary in the four corners)
*/
//=============================================================================
void CRpgLmu::GetOceanType(unsigned short chip, bool bOcean[4]) const
{
	unsigned int mask = 1;
	const unsigned short chip2 = chip % 800 / 50;

	for(int i = 0; i < 4; i++){
		bOcean[i] = ((chip2 & mask)!=0);
		mask <<= 1;
	}
}

//=============================================================================
/**
	Gets chip number coordinates

	@param	nChip			[in] Chip number
	@param	bUpper;			[in] true: Upper layer chip / false: Lower layer chip
	@param	lpX				[out] x coordinate
	@param	lpY				[out] y coordinate
*/
//=============================================================================
void CRpgLmu::GetChipPos(int nChip, bool bUpper, int* lpX, int* lpY) const
{
	// Chip ID is set to relative ID in ChipSet (?).
	int chip;
	if(bUpper)	chip = nChip - 10000 + 336;		// 48*7
	else		chip = nChip - 5000  + 192;		// 48*4;

	// 横6マスのチップに換算した時の座標
	int pos_x = chip % 6;
	int pos_y = chip / 6;

	// Coordinates of the block when dividing every block of 6x8
	// 算出しようとするとどうしても乗算が入ってきて遅くなりそうなのでifで分岐(汗
	int block_x, block_y;
	if     (pos_y<8){	block_x = 0;  block_y = 0; }
	else if(pos_y<16){	block_x = 0;  block_y = 8; }
	else if(pos_y<24){	block_x = 6;  block_y = 0; }
	else if(pos_y<32){	block_x = 6;  block_y = 8; }
	else if(pos_y<40){	block_x = 12; block_y = 0; }
	else if(pos_y<48){	block_x = 12; block_y = 8; }
	else if(pos_y<56){	block_x = 18; block_y = 0; }
	else if(pos_y<64){	block_x = 18; block_y = 8; }
	else if(pos_y<72){	block_x = 24; block_y = 0; }
	else			{	block_x = 24; block_y = 8; }

	*lpX = block_x +  pos_x;
	*lpY = block_y + (pos_y & 0x07);
}
