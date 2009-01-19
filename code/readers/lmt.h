/* lmt.h, LcfMapTree reader class.
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

#ifndef LMT_H
#define LMT_H

#include <string>
#include <vector>

struct music_data
{
    std::string name;
    int fade_in;
    int volume;
    int tempo;
    int balance;
};

struct node_data
{
    int id;
    std::string name;
    int parent_id;
    int depth;
    int type;
    int scrollbar_x;
    int scrollbar_y;
    int expanded;
    int music;
    music_data music_file;
    int backdrop;
    std::string backdrop_file;
    int teleport;
    int escape;
    int save;
    std::vector<int> encounter;
    int encounter_steps;
    int area_start_x;
    int area_start_y;
    int area_end_x;
    int area_end_y;
};

class lmt_data
{
public:
    int total_nodes;
    int party_map_id;
    int party_x;
    int party_y;
    int skiff_map_id;
    int skiff_x;
    int skiff_y;
    int boat_map_id;
    int boat_x;
    int boat_y;
    int airship_map_id;
    int airship_x;
    int airship_y;
    std::vector<node_data> tree_list;
    std::vector<int> tree_order;
    std::vector<int> tree_order_ordered;
    int selected_node;
};


class lmt_reader
{
public:
    //Methods
    bool load(std::string filename,lmt_data * data);
    void print(lmt_data * data);
private:
    void read_tree(FILE *file,lmt_data * data);
    void clear(node_data *node);
};

#endif
