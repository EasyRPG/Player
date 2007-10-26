//#include <windows.h>
#include <stdio.h>
#include "./SDK/index.h"
#include "sueLib/CImage/CImgUtil.h"
#include "sueLib/CImage/CBmpIO.h"


int main(int argc, char *argv[])
{
	int num;
	CRpgLmu lmu;
	CRpgLdb ldb;


	if(!ldb.Init()){
		printf("Unable to open RPG_RT.ldb\n");
		return 0;
	}

	printf("Map number? (1-...):");
	scanf("%d", &num);

	if(!lmu.Init(num, ldb)){
		printf("Unable to open Map%04d.lmu\n", num);
		return 0;
	}

	CRpgImage	imgUpper, imgLower;		// Upper and lower tiles
	CImage24	imgChip;	// Place to hold generated tiles(24bits)
	CImage24	imgMap;		// Map picture
	int x, y;

	// Initialization of map picture to save
	imgMap.Create(lmu.GetWidth()*16, lmu.GetHeight()*16);
	for(x = 0; x < lmu.GetWidth(); x++){
		for(y = 0; y < lmu.GetHeight(); y++){
			// Get upper and lower tiles
			if(lmu.GetUpperChip(x, y, imgUpper) && lmu.GetLowerChip(x, y, 0, imgLower)){
				// Generate upper and lower tiles
				// (Color 0 is ignored, removing in 8 bit mode)
				imgLower.Blt(imgUpper);

				// 8bits -> 24bits
				CImgUtil::ConvertColorNum(imgChip, imgLower);

				// Drawing to map picture
				imgMap.Blt(imgChip, x*16, y*16);
			}
		}
	}

	// PNG saving
	char file[256];
	sprintf(file, "Map%04d.bmp", num);
	CBmpIO::Save(file, imgMap);

	return 0;
}
