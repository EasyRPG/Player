#include "zobj.h"

ZObj::ZObj(int iz, int itype, int iid, int icreation) {
	z = iz;
	type = itype;
	id = iid;
	creation = icreation;
}

ZObj::~ZObj()
{
}

int ZObj::get_z()
{
	return z;
}

int ZObj::get_id()
{
	return id;
}

int ZObj::get_type()
{
	return type;
}

int ZObj::get_creation()
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
