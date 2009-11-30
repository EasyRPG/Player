#ifndef __font__
#define __font__

#include <string>
#include "SDL_ttf.h"
#include "color.h"

class Font {

public:
	Font();
	Font(std::string name);
	Font(std::string name, int size);
	~Font();
	
	std::string get_name();
	int get_size();
	bool get_bold();
	bool get_italic();
	Color get_color();
	
	void set_name(std::string nname);
	void set_size(int nsize);
	void set_bold(bool nbold);
	void set_italic(bool nitalic);
	void set_color(Color ncolor);
	
	static std::string get_default_name();
	static int get_default_size();
	static bool get_default_bold();
	static bool get_default_italic();
	static Color get_default_color();
	
	static void set_default_name(std::string nname);
	static void set_default_size(int nsize);
	static void set_default_bold(bool nbold);
	static void set_default_italic(bool nitalic);
	static void set_default_color(Color ncolor);
	
	static bool does_exist(std::string name);
	
private:
	TTF_Font *font;
	
	std::string name;
	int size;
	bool bold;
	bool italic;
	Color color;

	static std::string default_name;
	static int default_size;
	static bool default_bold;
	static bool default_italic;
	static Color default_color;
};
#endif // __font__
