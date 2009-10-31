#ifndef __tilemap__
#define __tilemap__

#include <map>
#include "SDL.h"
#include "viewport.h"
#include "bitmap.h"
#include "rect.h"
#include "table.h"
class Viewport;
class Tilemap {

public:
	Tilemap();
	Tilemap(Viewport *iviewport);
	~Tilemap();
	
	void dispose();
	bool is_disposed();
	void update();

	void draw(SDL_Surface *screen);

	Viewport* get_viewport();
	Bitmap* get_tileset();
	Table* get_map_data();
	Table* get_properties();
	bool get_visible();
	int get_ox();
	int get_oy();
	
	void get_viewport(Viewport* nviewport);
	void get_tileset(Bitmap* nbitmap);
	void get_map_data(Table* nmap_data);
	void get_properties(Table* nproperties);
	void get_visible(bool nvisible);
	void get_ox(int nox);
	void get_oy(int noy);
	
	static std::map<int, Tilemap*> tilemaps;
	static void add_tilemap(int id, Tilemap* tilemap);
	static void remove_tilemap(int id);
	
private:
	Viewport* viewport;
	Bitmap* tileset;
	Table* map_data;
	Table* properties;
	bool visible;
	int ox;
	int oy;
	
	bool disposed;
	
	int id;
	static int count;
};
#endif // __tilemap__
