#include "table.h"

Table(int ixsize)
{
	resize(ixsize);
}

Table(int ixsize, int iysize)
{
	resize(ixsize, iysize);
}

Table(int ixsize, int iysize, int izsize)
{
	resize(ixsize, iysize, izsize);
}

~Table()
{
	
}

void resize(int nxsize)
{
	xsize = nxsize;
	ysize = 0;
	zsize = 0;
	values.resize(xsize);
}

void resize(int nxsize, int nysize)
{
	xsize = nxsize;
	ysize = nysize;
	zsize = 0;
	values.resize(xsize * ysize);
}

void resize(int nxsize, int nysize, int nzsize)
{
	xsize = nxsize;
	ysize = nysize;
	zsize = nzsize;
	values.resize(xsize * ysize * zsize);
}

int get_xsize()
{
	return xsize;
}

int get_ysize()
{
	return ysize;
}

int get_zsize()
{
	return zsize;
}

int get_val(int x)
{
	return values[x];
}

int get_val(int x, int y)
{
	return values[x + y * xsize];
}

int get_val(int x, int y, int z)
{
	return values[x + y * xsize + z * xsize * ysize];
}
