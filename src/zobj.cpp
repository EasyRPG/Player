#include "zobj.h"

int ZObj::get_z() const
{
	return z;
}

int ZObj::get_id() const
{
	return id;
}

int ZObj::get_type() const
{
	return type;
}

int ZObj::get_creation() const
{
	return creation;
}

void ZObj::set_z(int nz)
{
	z = nz;
}

void ZObj::set_id(int nid)
{
	id = nid;
}

void ZObj::set_type(int ntype)
{
	type = ntype;
}

void ZObj::set_creation(int ncreation)
{
	creation = ncreation;
}
