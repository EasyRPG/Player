/* lmt.cpp, LcfMapTree reader class.
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

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <algorithm>
#include "../tools/tools.h"
#include "lmt.h"

bool lmt_reader::load(std::string filename,lmt_data * data)
{
    FILE *file;
    char dummy;
    int vehicle_chunk_id;
    int vehicle_chunk_size;

    file = fopen(filename.c_str(), "rb");
    //Check if the file is valid, if not...
    if (file == NULL)
    {
        printf("Unable to open file %s. make sure the file exists and you have read privileges on it.\n", filename.c_str());
        return 1;
    }
    //Read header
    if (ReadString(file) != "LcfMapTree")
    {
        printf("Reading error: File is not a valid RPG Maker 2000/2003 map tree.\n");
        fclose(file);
        return 2;
    }
    //Read number of nodes
    data->total_nodes = ReadCompressedInteger(file);
    ReadCompressedInteger(file); //00 end of block
    //Read tree
    read_tree(file,data);
    ReadCompressedInteger(file); //00 end of block
    //Read tree order
    data->tree_order.push_back(0); // Add 0 to the tree root
    for (int i = 0; i < (data->total_nodes - 1); i++) //Root zero node is not included in this list, so total_nodes - 1
    {
        data->tree_order.push_back(ReadCompressedInteger(file));
    }
    //Read selected node
    data->selected_node = ReadCompressedInteger(file);
    //Read vehicles (if any)
    data->party_map_id = 0;
    data->party_x = 0;
    data->party_y = 0;
    data->skiff_map_id = 0;
    data->skiff_x = 0;
    data->skiff_y = 0;
    data->boat_map_id = 0;
    data->boat_x = 0;
    data->boat_y = 0;
    data->airship_map_id = 0;
    data->airship_x = 0;
    data->airship_y = 0;
    while (!feof(file))
    {
        vehicle_chunk_id = ReadCompressedInteger(file);
        vehicle_chunk_size = ReadCompressedInteger(file);
        switch (vehicle_chunk_id)
        {
        case 1: //0x01
            data->party_map_id = ReadCompressedInteger(file);
            break;
        case 2: //0x02
            data->party_x = ReadCompressedInteger(file);
            break;
        case 3: //0x03
            data->party_y = ReadCompressedInteger(file);
            break;
        case 11: //0x0B
            data->skiff_map_id = ReadCompressedInteger(file);
            break;
        case 12: //0x0C
            data->skiff_x = ReadCompressedInteger(file);
            break;
        case 13: //0x0D
            data->skiff_y = ReadCompressedInteger(file);
            break;
        case 21: //0x15
            data->boat_map_id = ReadCompressedInteger(file);
            break;
        case 22: //0x16
            data->boat_x = ReadCompressedInteger(file);
            break;
        case 23: //0x17
            data->boat_y = ReadCompressedInteger(file);
            break;
        case 31: //0x1F
            data->airship_map_id = ReadCompressedInteger(file);
            break;
        case 32: //0x20
            data->airship_x = ReadCompressedInteger(file);
            break;
        case 33: //0x21
            data->airship_y = ReadCompressedInteger(file);
            break;
        case 0: //0x00
            if (vehicle_chunk_size != 0)
            {
                printf("WARNING: Unexpected byte after end of block. Size: %d\n", vehicle_chunk_size);
            }
            break;
        default:
            printf("WARNING: Unknown vehicle block ID: %d (size: %d)\n", vehicle_chunk_id, vehicle_chunk_size);
            // skip unknown vehicle_chunk_id case
            for (int i = 1 ; i > vehicle_chunk_size; i++)
            {
                bool return_value;
                return_value = fread(&dummy, 1, 1, file);
                printf("Byte read: %d\n", dummy);
            }
            break;
        }
    }

    fclose(file);
    //Assign tree IDs to the tree list
    //First, create a copy of tree_order
    data->tree_order_ordered = data->tree_order;
    //Next, sort this tree order copy
    std::sort(data->tree_order_ordered.begin(), data->tree_order_ordered.end());
    //Assign 0 to the tree root. This is always the 0. Because 0 is not stored in the tree_order vector.
    data->tree_list[0].id = 0;
    //Next from i = 0 to skip unexisting 0 on the tree_order and assign the IDs for each id.
    for(int i = 1; i < data->total_nodes; i++)
    {
        data->tree_list[i].id = data->tree_order_ordered[i];
    }

    return true;
}

void lmt_reader::clear(node_data *node)
{
    node->id = 0;
    node->name = "";
    node->parent_id = 0;
    node->depth = 0;
    node->type = 0;
    node->scrollbar_x = 0;
    node->scrollbar_y = 0;
    node->expanded = 0;
    node->music = 0;
    node->music_file.name = "";
    node->music_file.fade_in = 0;
    node->music_file.volume = 100;
    node->music_file.tempo = 100;
    node->music_file.balance = 50;
    node->backdrop = 0;
    node->backdrop_file = "";
    node->teleport = 0;
    node->escape = 0;
    node->save = 0;
    node->encounter.clear();
    node->encounter_steps = 0;
}

void lmt_reader::read_tree(FILE *file,lmt_data * data)
{
    int enemygroups = 0;
    int idgroup = 0;
    int enemy = 0;
    int current_node = 0;
    node_data node;
    char dummy;
    int node_chunk_id;
    int node_chunk_size;

    clear(&node);

    // Loop while we haven't reached the end of the file
    while (current_node < data->total_nodes)
    {
        node_chunk_id = ReadCompressedInteger(file); // lectura de tipo del pedazo
        node_chunk_size = ReadCompressedInteger(file);
        switch (node_chunk_id) // segun el tipo
        {
        case 1:
            node.name = ReadString(file, node_chunk_size);
            break;
        case 2:
            node.parent_id = ReadCompressedInteger(file);
            break;
        case 3:
            node.depth = ReadCompressedInteger(file); //redundant data, parent_id can discover this. rm2k editor can't have more than 256 depth nodes in TreeCtrl
            break;
        case 4:
            node.type = ReadCompressedInteger(file);
            break;
        case 5:
            node.scrollbar_x = ReadCompressedInteger(file); //editor data
            break;
        case 6:
            node.scrollbar_y = ReadCompressedInteger(file); //editor data
            break;
        case 7:
            node.expanded = ReadCompressedInteger(file); //editor data
            break;
        case 11: //0x0B
            node.music = ReadCompressedInteger(file);
            break;
        case 12: //0x0C
            while (node_chunk_id != 0)
            {
                node_chunk_id = ReadCompressedInteger(file);
                if (node_chunk_id != 0)
                {
                    node_chunk_size = ReadCompressedInteger(file);
                }
                switch (node_chunk_id)
                {
                case 0x01:
                    node.music_file.name = ReadString(file, node_chunk_size);
                    break;
                case 0x02:
                    node.music_file.fade_in = ReadCompressedInteger(file);
                    break;
                case 0x03:
                    node.music_file.volume = ReadCompressedInteger(file);
                    break;
                case 0x04:
                    node.music_file.tempo = ReadCompressedInteger(file);
                    break;
                case 0x05:
                    node.music_file.balance = ReadCompressedInteger(file);
                    break;
                case 0x00:
                    break;
                default:
                    printf("WARNING: Unknown Music block ID: %d (size: %d)\n", node_chunk_id, node_chunk_size);
                    // skip unknown node_chunk_id case
                    while (node_chunk_size--)
                    {
                        bool return_value;
                        return_value = fread(&dummy, 1, 1, file);
                        printf("Byte read: %d\n", dummy);
                    }
                    break;
                }
            }
            break;
        case 21: //0x15
            node.backdrop = ReadCompressedInteger(file);
            break;
        case 22: //0x16
            node.backdrop_file = ReadString(file, node_chunk_size);
            break;
        case 31: //0x1F
            node.teleport = ReadCompressedInteger(file);
            break;
        case 32: //0x20
            node.escape = ReadCompressedInteger(file);
            break;
        case 33: //0x21
            node.save = ReadCompressedInteger(file);
            break;
        case 41: //0x29
            enemygroups = ReadCompressedInteger(file); // numero de grupos
            idgroup = 0;
            while (idgroup < enemygroups)
            {
                idgroup = ReadCompressedInteger(file); //id de dato
                // no se para que escribieron los dos primeros enteros pero el 3ro es el id de grupo
                // probablemente quedo incompleto el rm2k
                ReadCompressedInteger(file);
                ReadCompressedInteger(file);
                enemy = ReadCompressedInteger(file);
                node.encounter.push_back(enemy);
                ReadCompressedInteger(file); //fin de bloque
            }
            break;
        case 44: //0x2C
            node.encounter_steps = ReadCompressedInteger(file);
            break;
        case 51: //0x33 Area data
            bool return_value;
            return_value = fread(&node.area_start_x, 4, 1, file);
            return_value = fread(&node.area_start_y, 4, 1, file);
            return_value = fread(&node.area_end_x, 4, 1, file);
            return_value = fread(&node.area_end_y, 4, 1, file);
            data->tree_list.push_back(node);
            break;
        case 0: //0x00 End of block
            clear(&node);
            current_node++;
            break;
        default:
            printf("WARNING: Unknown node block ID: %d (size: %d)\n", node_chunk_id, node_chunk_size);
            // skip unknown node_chunk_id case
            while (node_chunk_size--)
            {
                bool return_value;
                return_value = fread(&dummy, 1, 1, file);
                printf("Byte read: %d\n", dummy);
            }
            break;
        }
    }
}

void lmt_reader::print(lmt_data * data) // muestra de informacion del mapa
{
    printf("==========================================================================\n");
    printf("Map tree\n");
    for(int i = 0; i < (data->total_nodes); i++)
    {
        printf("--------------------------------------------------------------------------\n");
        printf("ID:                                                            %d\n", data->tree_list[i].id);
        printf("Name:                                                          %s\n", data->tree_list[i].name.c_str());
        printf("Parent ID:                                                     %d\n", data->tree_list[i].parent_id);
        printf("Depth:                                                         %d\n", data->tree_list[i].depth);
        printf("Type (0=root, 1=map, 2=area):                                  %d\n", data->tree_list[i].type);
        printf("Scrollbars:                                                    %d,%d\n", data->tree_list[i].scrollbar_x, data->tree_list[i].scrollbar_y);
        printf("Is expanded?:                                                  %d\n", data->tree_list[i].expanded);
        printf("Has Music? (0=inherit, 1=don't change, 2=specify):             %d\n", data->tree_list[i].music);
        printf("Music name:                                                    %s\n", data->tree_list[i].music_file.name.c_str());
        printf("Music fade in:                                                 %d\n", data->tree_list[i].music_file.fade_in);
        printf("Music volume:                                                  %d\n", data->tree_list[i].music_file.volume);
        printf("Music tempo:                                                   %d\n", data->tree_list[i].music_file.tempo);
        printf("Music balance:                                                 %d\n", data->tree_list[i].music_file.balance);
        printf("Has battle background? (0=inherit, 1=don't change, 2=specify): %d\n", data->tree_list[i].backdrop);
        printf("Battle baground name:                                          %s\n", data->tree_list[i].backdrop_file.c_str());
        printf("Teleport options (0=inherit, 1=don't change, 2=specify):       %d\n", data->tree_list[i].teleport);
        printf("Escape options (0=inherit, 1=don't change, 2=specify):         %d\n", data->tree_list[i].escape);
        printf("Save options (0=inherit, 1=don't change, 2=specify):           %d\n", data->tree_list[i].save);
        for (unsigned int j = 0; j < data->tree_list[i].encounter.size(); j++)
        {
            printf("Enemy encounter:                                               %d\n", data->tree_list[i].encounter[j]);
        }
        printf("Encounter steps:                                               %d\n", data->tree_list[i].encounter_steps);
        printf("Area coordinates:                                              %d,%d-%d,%d\n", data->tree_list[i].area_start_x, data->tree_list[i].area_start_y, data->tree_list[i].area_end_y, data->tree_list[i].area_end_y);
    }
    printf("==========================================================================\n");
    printf("End of tree. Data after the tree:\n");
    printf("--------------------------------------------------------------------------\n");
    printf("Branch order:\n");
    for(int i = 0; i < (data->total_nodes); i++)
    {
        printf("%d ", data->tree_order[i]);
    }
    printf("(0 is always faked)\n\n");
    printf("Selected node:                                                 %d\n", data->selected_node);
    printf("--------------------------------------------------------------------------\n");
    printf("Party map ID:                                                  %d\n", data->party_map_id);
    printf("Party X:                                                       %d\n", data->party_x);
    printf("Party Y:                                                       %d\n", data->party_y);
    printf("Skiff map ID:                                                  %d\n", data->skiff_map_id);
    printf("Skiff X:                                                       %d\n", data->skiff_x);
    printf("Skiff Y:                                                       %d\n", data->skiff_y);
    printf("Boat map ID:                                                   %d\n", data->boat_map_id);
    printf("Boat X:                                                        %d\n", data->boat_x);
    printf("Boat Y:                                                        %d\n", data->boat_y);
    printf("Airship map ID:                                                %d\n", data->airship_map_id);
    printf("Airship X:                                                     %d\n", data->airship_x);
    printf("Airship Y:                                                     %d\n", data->airship_y);
}
