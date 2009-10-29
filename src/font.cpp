#include "font.h"

Font::Font()
{
	name = default_name;
	size = default_size;
	bold = default_bold;
	italic = default_italic;
	color = default_color;
}

Font::Font(std::string name)
{
	name = name;
	size = default_size;
	bold = default_bold;
	italic = default_italic;
	color = default_color;
}

Font::Font(std::string name, int size)
{
	name = name;
	size = size;
	bold = default_bold;
	italic = default_italic;
	color = default_color;
}

Font::~Font()
{
}

std::string Font::default_name = "Arial";
int Font::default_size = 22;
bool Font::default_bold = false;
bool Font::default_italic = false;
Color Font::default_color = *(new Color(0, 0, 0));

bool Font::does_exist(std::string name)
{
	/* TODO */
	return true;
}

std::string Font::get_name()
{
	return name;
}

int Font::get_size()
{
	return size;
}

bool Font::get_bold()
{
	return bold;
}

bool Font::get_italic()
{
	return italic;
}

Color Font::get_color()
{
	return color;
}

void Font::set_name(std::string nname)
{
	name = nname;
}

void Font::set_size(int nsize)
{
	size = nsize;
}

void Font::set_bold(bool nbold)
{
	bold = nbold;
}

void Font::set_italic(bool nitalic)
{
	italic = nitalic;
}

void Font::set_color(Color ncolor)
{
	color = ncolor;
}

std::string Font::get_default_name()
{
	return default_name;
}

int Font::get_default_size()
{
	return default_size;
}

bool Font::get_default_bold()
{
	return default_bold;
}

bool Font::get_default_italic()
{
	return default_italic;
}

Color Font::get_default_color()
{
	return default_color;
}

void Font::set_default_name(std::string nname)
{
	default_name = nname;
}

void Font::set_default_size(int nsize)
{
	default_size = nsize;
}

void Font::set_default_bold(bool nbold)
{
	default_bold = nbold;
}

void Font::set_default_italic(bool nitalic)
{
	default_italic = nitalic;
}

void Font::set_default_color(Color ncolor)
{
	default_color = ncolor;
}
