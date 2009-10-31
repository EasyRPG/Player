#include "tilemap.h"
#include "zobj.h"

Tilemap::Tilemap()
{
	disposed = false;
	id = count;
	add_tilemap(count, this);
	count++;
	
	visible = true;
	ox = 0;
	oy = 0;
}

Tilemap::Tilemap(Viewport *iviewport)
{
	disposed = false;
	id = count;
	add_tilemap(count, this);
	count++;
	
	viewport = iviewport;
	visible = true;
	ox = 0;
	oy = 0;
}

Tilemap::~Tilemap()
{
//	Graphics::remove_tilemap(id);
}

int Tilemap::count = 0;

void Tilemap::dispose()
{
	disposed = true;
}

bool Tilemap::is_disposed()
{
	return disposed;
}

void Tilemap::update()
{
	
}

void Tilemap::draw(SDL_Surface *screen)
{

}

Viewport* Tilemap::get_viewport()
{
	return viewport;
}

Bitmap* Tilemap::get_tileset()
{
	return tileset;
}

Table* Tilemap::get_map_data()
{
	return map_data;
}

Table* Tilemap::get_properties()
{
	return properties;
}

bool Tilemap::get_visible()
{
	return visible;
}

int Tilemap::get_ox()
{
	return ox;
}

int Tilemap::get_oy()
{
	return oy;
}

void Tilemap::get_viewport(Viewport* nviewport)
{
	viewport = nviewport;
}

void Tilemap::get_tileset(Bitmap* nbitmap)
{
//	bitmap = nbitmap;
}

void Tilemap::get_map_data(Table* nmap_data)
{
	map_data = nmap_data;
}

void Tilemap::get_properties(Table* nproperties)
{
	properties = nproperties;
}

void Tilemap::get_visible(bool nvisible)
{
	visible = nvisible;
}

void Tilemap::get_ox(int nox)
{
	ox = nox;
}

void Tilemap::get_oy(int noy)
{
	oy = noy;
}

void Tilemap::add_tilemap(int id, Tilemap *tilemap)
{
	tilemaps[id] = tilemap;
	//ZObj zobj(0, id, TYPE_TILEMAP, frame_count);
//	ZObj::zlist.push_back(zobj);
}

void Tilemap::remove_tilemap(int id)
{
	tilemaps.erase(id);
//	ZObj::zlist.remove_if(remove_zobj_id(id));
}
