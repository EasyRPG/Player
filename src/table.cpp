#include "table.h"

Table::Table(int ixsize)
{
	resize(ixsize);
}

Table::Table(int ixsize, int iysize)
{
	resize(ixsize, iysize);
}

Table::Table(int ixsize, int iysize, int izsize)
{
	resize(ixsize, iysize, izsize);
}

Table::~Table()
{
	
}

void Table::resize(int nxsize)
{
	xsize = nxsize;
	ysize = 0;
	zsize = 0;
	values.resize(xsize);
}

void Table::resize(int nxsize, int nysize)
{
	xsize = nxsize;
	ysize = nysize;
	zsize = 0;
	values.resize(xsize * ysize);
}

void Table::resize(int nxsize, int nysize, int nzsize)
{
	xsize = nxsize;
	ysize = nysize;
	zsize = nzsize;
	values.resize(xsize * ysize * zsize);
}

int Table::get_xsize()
{
	return xsize;
}

int Table::get_ysize()
{
	return ysize;
}

int Table::get_zsize()
{
	return zsize;
}

int Table::get_val(int x)
{
	return values[x];
}

int Table::get_val(int x, int y)
{
	return values[x + y * xsize];
}

int Table::get_val(int x, int y, int z)
{
	return values[x + y * xsize + z * xsize * ysize];
}
