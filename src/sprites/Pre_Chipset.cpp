  #include "Pre_Chipset.h"


   bool Pre_Chipset::GenerateFromSurface(SDL_Surface * Surface)
    {
        // Set base surface, used for generating the tileset
        BaseSurface     = Surface;
        ChipsetSurface  = CreateSurface(32*16, 45*16);

        int CurrentTile = 0;

        // Generate water A
        for (int j=0; j<3; j++)
            for (int i=0; i<47; i++, CurrentTile++)
                RenderWaterTile(ChipsetSurface, (CurrentTile%32)*16, (CurrentTile/32)*16, j, 0, 0, i);

        // Generate water B
        for (int j=0; j<3; j++)
            for (int i=0; i<47; i++, CurrentTile++)
                RenderWaterTile(ChipsetSurface, (CurrentTile%32)*16, (CurrentTile/32)*16, j, 1, 0, i);

        // Generate water C
        for (int j=0; j<3; j++)
            for (int i=0; i<47; i++, CurrentTile++)
                RenderWaterTile(ChipsetSurface, (CurrentTile%32)*16, (CurrentTile/32)*16, j, 0, 3, i);

        // Generate water depth tiles
        for (int i=0; i<48; i++, CurrentTile++)
            RenderDepthTile(ChipsetSurface, (CurrentTile%32)*16, (CurrentTile/32)*16, i/16, 2, i%16);

        for (int i=0; i<48; i++, CurrentTile++)
            RenderDepthTile(ChipsetSurface, (CurrentTile%32)*16, (CurrentTile/32)*16, i/16, 1, i%16);

        // Generate animated tiles
        for (int j=0; j<3; j++)
            for (int i=0; i<4; i++, CurrentTile++)
                DrawSurface(ChipsetSurface, (CurrentTile%32)*16, (CurrentTile/32)*16, BaseSurface, 48+j*16, 64+i*16, 16, 16);

        // Generate terrain tiles
        for (int j=0; j<12; j++)
            for (int i=0; i<50; i++, CurrentTile++)
                RenderTerrainTile(ChipsetSurface, (CurrentTile%32)*16, (CurrentTile/32)*16, j, i);

        // Generate common tiles
        for (int i=0; i<288; i++, CurrentTile++)
            DrawSurface(ChipsetSurface, (CurrentTile%32)*16, (CurrentTile/32)*16, BaseSurface, 192+((i%6)*16)+(i/96)*96, ((i/6)%16)*16, 16, 16);

        // Done
        return true;
    }

    bool Pre_Chipset::GenerateFromFile(string Filename)
    {
        SDL_Surface * FileSurface = LoadSurface(Filename);
        return GenerateFromSurface(FileSurface);
    }
    void Pre_Chipset::dispose()
    {
         SDL_FreeSurface(BaseSurface);
         SDL_FreeSurface(ChipsetSurface);
    }

    void Pre_Chipset::RenderWaterTile(SDL_Surface * Destiny, int x, int y, int Frame, int Border, int Water, int Combination)
    {
        int SFrame       = Frame*16, SBorder = Border*48;
        //int SFrameBorder = SFrame+SBorder;
        Combination &= 0x3F;

        // Since this function isn't meant to be used in realtime, we can allow
        // use nasty code here. First off, draw the water tile, for the background.
        DrawSurface(Destiny, x, y, BaseSurface, SFrame, 64+(Water*16), 16, 16);

        // Now, get the combination from the tile and draw it using this stupidly
        // hard coded routine. I've found out that this was easier than just find
        // out a damn algorithm.
        if (Combination & 0x20)
        {
            // This is where it gets nasty :S
            if (Combination > 0x29)
            {
                // Multiple edge possibilities
                switch(Combination)
                {
                    case 0x2A:
                        DrawSurface(Destiny, x, y, BaseSurface, SFrame+SBorder, 0, 16, 8);
                        DrawSurface(Destiny, x, y+8, BaseSurface, SFrame+SBorder, 24, 16, 8);
                        break;
                    case 0x2B:
                        DrawSurface(Destiny, x, y, BaseSurface, SFrame+SBorder, 0, 8, 16);
                        DrawSurface(Destiny, x+8, y, BaseSurface, SFrame+SBorder+8, 32, 8, 16);
                        break;
                    case 0x2C:
                        DrawSurface(Destiny, x, y+8, BaseSurface, SFrame+SBorder, 8, 16, 8);
                        DrawSurface(Destiny, x, y, BaseSurface, SFrame+SBorder, 16, 16, 8);
                        break;
                    case 0x2D:
                        DrawSurface(Destiny, x+8, y, BaseSurface, SFrame+SBorder+8, 0, 8, 16);
                        DrawSurface(Destiny, x, y, BaseSurface, SFrame+SBorder, 32, 8, 16);
                        break;
                    case 0x2E:
                        DrawSurface(Destiny, x, y, BaseSurface, SFrame+SBorder, 0, 16, 16);
                        break;
                }
            } else {
                // Wall + inner edges
                switch((Combination>>1)&0x07)
                {
                    case 0x00:
                        if (Combination&0x01) DrawSurface(Destiny, x, y, BaseSurface, SFrame+SBorder, 32, 16, 16);
                        else                  DrawSurface(Destiny, x, y, BaseSurface, SFrame+SBorder, 16, 16, 16);
                        break;
                    case 0x01:
                        DrawSurface(Destiny, x, y, BaseSurface, SFrame+SBorder, 0, 8, 8);           // Corner
                        DrawSurface(Destiny, x, y+8, BaseSurface, SFrame+SBorder, 24, 8, 8);        // Left/Right frame
                        DrawSurface(Destiny, x+8, y, BaseSurface, SFrame+SBorder+8, 32, 8, 8);      // Top/Bottom frame

                        if (Combination&0x01) DrawSurface(Destiny, x+8, y+8, BaseSurface, SFrame+SBorder+8, 48+8, 8, 8);
                        break;
                    case 0x02:
                        DrawSurface(Destiny, x+8, y, BaseSurface, SFrame+SBorder+8, 0, 8, 8);       // Corner
                        DrawSurface(Destiny, x+8, y+8, BaseSurface, SFrame+SBorder+8, 24, 8, 8);    // Left/Right frame
                        DrawSurface(Destiny, x, y, BaseSurface, SFrame+SBorder, 32, 8, 8);          // Top/Bottom frame

                        if (Combination&0x01) DrawSurface(Destiny, x, y+8, BaseSurface, SFrame+SBorder, 48+8, 8, 8);
                        break;
                    case 0x03:
                        DrawSurface(Destiny, x+8, y+8, BaseSurface, SFrame+SBorder+8, 8, 8, 8);     // Corner
                        DrawSurface(Destiny, x+8, y, BaseSurface, SFrame+SBorder+8, 16, 8, 8);      // Left/Right frame
                        DrawSurface(Destiny, x, y+8, BaseSurface, SFrame+SBorder, 40, 8, 8);        // Top/Bottom frame

                        if (Combination&0x01) DrawSurface(Destiny, x, y, BaseSurface, SFrame+SBorder, 48, 8, 8);
                        break;
                    case 0x04:
                        DrawSurface(Destiny, x, y+8, BaseSurface, SFrame+SBorder, 8, 8, 8);         // Corner
                        DrawSurface(Destiny, x, y, BaseSurface, SFrame+SBorder, 16, 8, 8);          // Left/Right frame
                        DrawSurface(Destiny, x+8, y+8, BaseSurface, SFrame+SBorder+8, 40, 8, 8);    // Top/Bottom frame

                        if (Combination&0x01) DrawSurface(Destiny, x+8, y, BaseSurface, SFrame+SBorder+8, 48, 8, 8);
                        break;
                }
            }
        } else if (Combination & 0x10)
        {
            // Wall + inner edge cases. They're also easier to find out the
            // values here too
            switch((Combination>>2)&0x03)
            {
                case 0x00:
                    // Render left wall
                    DrawSurface(Destiny, x, y, BaseSurface, SFrame+SBorder, 16, 8, 16);

                    // Render top right corner and bottom right corner
                    if (Combination&0x01) DrawSurface(Destiny, x+8, y,   BaseSurface, SFrame+SBorder+8, 48, 8, 8);
                    if (Combination&0x02) DrawSurface(Destiny, x+8, y+8, BaseSurface, SFrame+SBorder+8, 48+8, 8, 8);
                    break;
                case 0x01:
                    // Render top wall
                    DrawSurface(Destiny, x, y, BaseSurface, SFrame+SBorder, 32, 16, 8);

                    // Render bottom right corner and bottom left corner
                    if (Combination&0x01) DrawSurface(Destiny, x+8, y+8, BaseSurface, SFrame+SBorder+8, 48+8, 8, 8);
                    if (Combination&0x02) DrawSurface(Destiny, x, y+8,   BaseSurface, SFrame+SBorder, 48+8, 8, 8);
                    break;
                case 0x02:
                    // Right wall
                    DrawSurface(Destiny, x+8, y, BaseSurface, SFrame+SBorder+8, 16, 8, 16);

                    // Render bottom left corner and top left corner
                    if (Combination&0x01) DrawSurface(Destiny, x, y+8, BaseSurface, SFrame+SBorder, 48+8, 8, 8);
                    if (Combination&0x02) DrawSurface(Destiny, x, y,   BaseSurface, SFrame+SBorder, 48, 8, 8);
                    break;
                case 0x03:
                    // Bottom wall
                    DrawSurface(Destiny, x, y+8, BaseSurface, SFrame+SBorder, 32+8, 16, 8);

                    // Render top left corner and top right corner
                    if (Combination&0x01) DrawSurface(Destiny, x, y,   BaseSurface, SFrame+SBorder, 48, 8, 8);
                    if (Combination&0x02) DrawSurface(Destiny, x+8, y, BaseSurface, SFrame+SBorder+8, 48, 8, 8);
                    break;
            }
        } else {
            // Single inner edge cases. They're easier to find out the values
            // this way.
            if (Combination&0x01) DrawSurface(Destiny, x, y, BaseSurface, SFrame+SBorder, 48, 8, 8);
            if (Combination&0x02) DrawSurface(Destiny, x+8, y, BaseSurface, SFrame+SBorder+8, 48, 8, 8);
            if (Combination&0x04) DrawSurface(Destiny, x+8, y+8, BaseSurface, SFrame+SBorder+8, 48+8, 8, 8);
            if (Combination&0x08) DrawSurface(Destiny, x, y+8, BaseSurface, SFrame+SBorder, 48+8, 8, 8);
        }
    }


    void Pre_Chipset::RenderTerrainTile(SDL_Surface * Destiny, int x, int y, int Terrain, int Combination)
    {
        Terrain += 4;
        int XTerrain = ((Terrain%2)*48)+(Terrain/8)*96, YTerrain = ((Terrain/2)%4)*64;


        // Since this function isn't meant to be used in realtime, we can allow
        // use nasty code here. First off, draw the water tile, for the background.
        DrawSurface(Destiny, x, y, BaseSurface, XTerrain+16, YTerrain+32, 16, 16);

        // Now, get the combination from the tile and draw it using this stupidly
        // hard coded routine. I've found out that this was easier than just find
        // out a damn algorithm.
        if (Combination & 0x20)
        {
            // This is where it gets nasty :S
            if (Combination > 0x29)
            {
                // Multiple edge possibilities
                switch(Combination)
                {
                    case 0x2A:
                        DrawSurface(Destiny, x, y, BaseSurface,     XTerrain,    YTerrain+16, 8, 16);
                        DrawSurface(Destiny, x+8, y, BaseSurface,   XTerrain+40, YTerrain+16, 8, 16);
                        break;
                    case 0x2B:
                        DrawSurface(Destiny, x, y, BaseSurface,     XTerrain,    YTerrain+16, 16, 8);
                        DrawSurface(Destiny, x, y+8, BaseSurface,   XTerrain,    YTerrain+56, 16, 8);
                        break;
                    case 0x2C:
                        DrawSurface(Destiny, x, y, BaseSurface,     XTerrain,    YTerrain+48, 8, 16);
                        DrawSurface(Destiny, x+8, y, BaseSurface,   XTerrain+40, YTerrain+48, 8, 16);
                        break;
                    case 0x2D:
                        DrawSurface(Destiny, x, y, BaseSurface,     XTerrain+32, YTerrain+16, 16, 8);
                        DrawSurface(Destiny, x, y+8, BaseSurface,   XTerrain+32, YTerrain+56, 16, 8);
                        break;
                    case 0x2E:
                        DrawSurface(Destiny, x, y, BaseSurface,     XTerrain,    YTerrain+16, 8, 8);
                        DrawSurface(Destiny, x+8, y, BaseSurface,   XTerrain+40, YTerrain+16, 8, 8);
                        DrawSurface(Destiny, x, y+8, BaseSurface,   XTerrain,    YTerrain+56, 8, 8);
                        DrawSurface(Destiny, x+8, y+8, BaseSurface, XTerrain+40, YTerrain+56, 8, 8);
                        break;
                    case 0x31:
                        DrawSurface(Destiny, x, y, BaseSurface, XTerrain, YTerrain, 16, 16);
                        break;
                }
            } else {
                // Wall + inner edges
                switch((Combination>>1)&0x07)
                {
                    case 0x00:
                        if (Combination&0x01)
                        {
                            DrawSurface(Destiny, x, y, BaseSurface, XTerrain+16, YTerrain+16, 16, 8);
                            DrawSurface(Destiny, x, y+8, BaseSurface, XTerrain+16, YTerrain+56, 16, 8);
                        } else {
                            DrawSurface(Destiny, x, y, BaseSurface, XTerrain, YTerrain+32, 8, 16);
                            DrawSurface(Destiny, x+8, y, BaseSurface, XTerrain+40, YTerrain+32, 8, 16);
                        }
                        break;
                    case 0x01:
                        DrawSurface(Destiny, x, y, BaseSurface, XTerrain, YTerrain+16, 16, 16);
                        if (Combination&0x01) DrawSurface(Destiny, x+8, y+8, BaseSurface, XTerrain+40, YTerrain+8, 8, 8);
                        break;
                    case 0x02:
                        DrawSurface(Destiny, x, y, BaseSurface, XTerrain+32, YTerrain+16, 16, 16);
                        if (Combination&0x01) DrawSurface(Destiny, x, y+8, BaseSurface, XTerrain+32, YTerrain+8, 8, 8);
                        break;
                    case 0x03:
                        DrawSurface(Destiny, x, y, BaseSurface, XTerrain+32, YTerrain+48, 16, 16);
                        if (Combination&0x01) DrawSurface(Destiny, x, y, BaseSurface, XTerrain+32, YTerrain, 8, 8);
                        break;
                    case 0x04:
                        DrawSurface(Destiny, x, y, BaseSurface, XTerrain, YTerrain+48, 16, 16);
                        if (Combination&0x01) DrawSurface(Destiny, x+8, y, BaseSurface, XTerrain+40, YTerrain, 8, 8);
                        break;
                }
            }
        } else if (Combination & 0x10)
        {
            // Wall + inner edge cases. They're also easier to find out the
            // values here too
            switch((Combination>>2)&0x03)
            {
                case 0x00:
                    // Render left wall
                    DrawSurface(Destiny, x, y, BaseSurface, XTerrain, YTerrain+32, 16, 16);

                    // Render top right corner and bottom right corner
                    if (Combination&0x01) DrawSurface(Destiny, x+8, y,   BaseSurface, XTerrain+40, YTerrain, 8, 8);
                    if (Combination&0x02) DrawSurface(Destiny, x+8, y+8, BaseSurface, XTerrain+40, YTerrain+8, 8, 8);
                    break;
                case 0x01:
                    // Render top wall
                    DrawSurface(Destiny, x, y, BaseSurface, XTerrain+16, YTerrain+16, 16, 16);

                    // Render bottom right corner and bottom left corner
                    if (Combination&0x01) DrawSurface(Destiny, x+8, y+8, BaseSurface, XTerrain+40, YTerrain+8, 8, 8);
                    if (Combination&0x02) DrawSurface(Destiny, x, y+8,   BaseSurface, XTerrain+32, YTerrain+8, 8, 8);
                    break;
                case 0x02:
                    // Right wall
                    DrawSurface(Destiny, x, y, BaseSurface, XTerrain+32, YTerrain+32, 16, 16);

                    // Render bottom left corner and top left corner
                    if (Combination&0x01) DrawSurface(Destiny, x, y+8,   BaseSurface, XTerrain+32, YTerrain+8, 8, 8);
                    if (Combination&0x02) DrawSurface(Destiny, x, y,     BaseSurface, XTerrain+32, YTerrain, 8, 8);
                    break;
                case 0x03:
                    // Bottom wall
                    DrawSurface(Destiny, x, y, BaseSurface, XTerrain+16, YTerrain+48, 16, 16);

                    // Render top left corner and top right corner
                    if (Combination&0x01) DrawSurface(Destiny, x, y,     BaseSurface, XTerrain+32, YTerrain, 8, 8);
                    if (Combination&0x02) DrawSurface(Destiny, x+8, y,   BaseSurface, XTerrain+40, YTerrain, 8, 8);
                    break;
            }
        } else {
            // Single inner edge cases. They're easier to find out the values
            // this way.
            if (Combination&0x01) DrawSurface(Destiny, x, y,     BaseSurface, XTerrain+32, YTerrain, 8, 8);
            if (Combination&0x02) DrawSurface(Destiny, x+8, y,   BaseSurface, XTerrain+40, YTerrain, 8, 8);
            if (Combination&0x04) DrawSurface(Destiny, x+8, y+8, BaseSurface, XTerrain+40, YTerrain+8, 8, 8);
            if (Combination&0x08) DrawSurface(Destiny, x, y+8,   BaseSurface, XTerrain+32, YTerrain+8, 8, 8);
        }
    }

    void Pre_Chipset::RenderDepthTile(SDL_Surface * Destiny, int x, int y, int Frame, int Depth, int DepthCombination)
    {

        int SFrame = Frame*16;

        // Now render the depth part
        if (DepthCombination&0x01)
        DrawSurface(Destiny, x,   y,   BaseSurface, SFrame,   64+(Depth*16),   8, 8);

        if (DepthCombination&0x02)
        DrawSurface(Destiny, x+8, y,   BaseSurface, SFrame+8, 64+(Depth*16),   8, 8);

        if (DepthCombination&0x04)
        DrawSurface(Destiny, x,   y+8, BaseSurface, SFrame,   64+(Depth*16)+8, 8, 8);

        if (DepthCombination&0x08)
        DrawSurface(Destiny, x+8, y+8, BaseSurface, SFrame+8, 64+(Depth*16)+8, 8, 8);

    }

