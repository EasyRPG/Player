#ifndef MAP_H
#define MAP_H

// *****************************************************************************
// =============================================================================
    #include <stdlib.h>
    #include <stdio.h>
    #include <string>
    #include <SDL/SDL.h>
    #include "tools.h"
    #include "chipset.h"
    using namespace std;
// =============================================================================
// *****************************************************************************
        
    // === Map structure =======================================================
    struct stMap
    {
        // --- Fields declaration ---------------------------------------------- 
        short MapWidth, MapHeight;      // These determine the size of the map
        
        short ChipsetID;                // This points to the ID of the Chipset
        stChipset Chipset;              // the map uses, and the structure
                                        // contains the tileset data of it
   
        unsigned char TypeOfLoop;       // These flags determines if the map has to
                                        // loop infinitely.
        
        bool ParallaxBackground;
        string BackgroundName;
            
        bool HorizontalPan;
        bool HorizontalAutoPan;
        short HorizontalPanSpeed;
            
        bool VerticalPan;
        bool VerticalAutoPan;
        short VerticalPanSpeed;
            
        unsigned short * LowerLayer;
        unsigned short * UpperLayer;
            
        int TimesSaved;
        int NumEvents;
        
        // --- Methods declaration ---------------------------------------------
        bool Load(string Filename);
        void GetNextChunk(FILE * Stream);
        void ProcessChunk(FILE * Stream, tChunk * Chunk);
        void ShowInformation();   
        
        void Render(SDL_Surface * Destiny, int Layer, int CameraX, int CameraY);         
    };

#endif
