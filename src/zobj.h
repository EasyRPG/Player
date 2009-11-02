#ifndef __zobj__
#define __zobj__

#define TYPE_VIEWPORT 0
#define TYPE_SPRITE 1
#define TYPE_PLANE 2
#define TYPE_WINDOW 3
#define TYPE_TILEMAP 4

class ZObj {

public:
	ZObj(int iz, int itype, int iid, int icreation);
	~ZObj();
	
	int get_z();
	int get_id();
	int get_type();
	int get_creation();
	
	void set_z(int nz);
	void set_id(int nid);
	void set_type(int ntype);
	void set_creation(int ncreation);
private:
	int z;
	int id;
	int type;
	int creation;
};
/* Non compillable func. FIXME
struct remove_zobj_id : public std::binary_function<ZObj, ZObj, bool> {
    remove_zobj_id(int val) : id(val) {}
    bool operator () (ZObj &obj) const {return obj.get_id() == id;}
    int id;
}; */

#endif // __zobj__
