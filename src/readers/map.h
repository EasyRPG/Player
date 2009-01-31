/* map.h, main map routines.
   Copyright (C) 2007 EasyRPG Project <http://easyrpg.sourceforge.net/>.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#ifndef MAP_DATA_H
#define MAP_DATA_H

// *****************************************************************************
// =============================================================================
    #include <stdlib.h>
    #include <stdio.h>
    #include <string>
    #include "../tools/tools.h"
    #include "eventchunks.h"
    #include "mapchunks.h"
    #include "strmap.h"
    using namespace std;
    #include <vector>
// =============================================================================
// *****************************************************************************

    class map_data
    {
        public:
        short MapWidth, MapHeight;      // These determine the size of the map
        short ChipsetID;                // This points to the ID of the Chipset
        unsigned char TypeOfLoop;       // These flags determines if the map has to
                                        // loop infinitely.
        bool ParallaxBackground;        // si se usa un fondo paralelo
        string BackgroundName;          // nombre de la imagen del fondo
        bool HorizontalPan;             //si hay mobimiento orisontal
        bool HorizontalAutoPan;         // si es automatico
        short HorizontalPanSpeed;       // la velocidad del movimiento
        bool VerticalPan;               //si hay movimiento vertical
        bool VerticalAutoPan;           // si es automatico
        short VerticalPanSpeed;         // la velocidad del movimiento
        unsigned short * LowerLayer;// patadores a las cpas
        unsigned short * UpperLayer;
        int TimesSaved;
        int NumEvents;

        int use_genertor;
        int gen_mode;

        int gen_Num_titles;
        int gen_width;
        int gen_height;
        int gen_surround_map;
        int gen_use_upper_wall;
        int gen_use_floor_b;
        int gen_use_floor_c;
        int gen_use_extra_b;
        int gen_use_extra_c;

        int gen_roof_X;
        int gen_down_wall_X;
        int gen_upper_wall_X;
        int gen_floor_a_X;
        int gen_floor_b_X;
        int gen_floor_c_X;
        int gen_extra_a_X;
        int gen_extra_b_X;
        int gen_extra_c_X;

        int gen_roof_Y;
        int gen_down_wall_Y;
        int gen_upper_wall_Y;
        int gen_floor_a_Y;
        int gen_floor_b_Y;
        int gen_floor_c_Y;
        int gen_extra_a_Y;
        int gen_extra_b_Y;
        int gen_extra_c_Y;
        unsigned short * gen_chipset_ids;
        std:: vector <stEventMap> vcEvents;
        void clear_events();

    };

    class map_reader
    {
        unsigned char Void;
        tChunk ChunkInfo; // informacion del pedazo leido
        // --- Methods declaration ---------------------------------------------
public:
        bool Load(string Filename,map_data * data); // carga de mapa
        void ShowInformation(map_data * data);  // info del mapa
private:
        void GetNextChunk(FILE * Stream,map_data * data); //lectra de pedasos
        std:: vector <stEventMap> eventChunk(FILE * Stream);//eventos de mapa
        vector <stPageEventMap> pageChunk(FILE * Stream);//paginas de evento
        stPageConditionEventMap conditionChunk(FILE * Stream);//condiciones de pagina
        stPageMovesEventMap PageMovesChunk(FILE * Stream);//movimiento de pagina

    };
#endif
