#ifndef __table__
#define __table__

#include <vector>

class Table {

public:
	Table(int ixsize);
	Table(int ixsize, int iysize);
	Table(int ixsize, int iysize, int izsize);
	~Table();

	void resize(int nxsize);
	void resize(int nxsize, int nysize);
	void resize(int nxsize, int nysize, int nzsize);
	int get_xsize();
	int get_ysize();
	int get_zsize();
	
	int get_val(int x);
	int get_val(int x, int y);
	int get_val(int x, int y, int z);
	
private:
	std::vector<int> values;
	int xsize;
	int ysize;
	int zsize;
};
#endif // __table__
