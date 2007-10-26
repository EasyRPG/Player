/**
	@file
	@brief		Multidimensional structure
	@author		sue445
*/
#ifndef _INC_CDIMENTION
#define _INC_CDIMENTION

namespace sueLib {

typedef int DimType;

template< unsigned int DIMENSION > class CDimension;

//=============================================================================
/**
	Multidimensional structure (for 1 dimension)

	@note	Part definition of CDimension template
*/
//=============================================================================
template<>
class CDimension< 1 >{
private:
	DimType		x;			///< Its own dimension coordinates

public:
	CDimension(DimType _x=0) : x(_x){}	/// Default constructor

	operator DimType() const{ return x; }
	friend CDimension;
};

//=============================================================================
/**
	Multidimensional structure (for n dimensions)

	@note	General definition of CDimension template
*/
//=============================================================================
template< unsigned int DIMENSION >
class CDimension{
private:
	DimType				x;	///< Its own dimension coordinates
	CDimension< DIMENSION - 1 >	y;	///< 1 dimension lower

public:
	CDimension(DimType _x=0, ...) : x(_x), y(*((&_x)+1)){}	/// Default constructor

	// Comparison operators (The first 2 are implemented and the 4 remaining are copypasted)
	bool operator==(const CDimension& obj) const{ return (x==obj.x && y==obj.y); }
	bool operator< (const CDimension& obj) const{
		return (x==obj.x) ? (y<obj.y) : (x<obj.x);
	}
	bool operator!=(const CDimension& obj) const{ return !(*this==obj); }
	bool operator<=(const CDimension& obj) const{ return (*this==obj || *this<obj); }
	bool operator>=(const CDimension& obj) const{ return !(*this<obj); }
	bool operator> (const CDimension& obj) const{ return !(*this<=obj); }

	friend CDimension;
};

typedef CDimension< 1 >	CDim1;		///< 1 dimension
typedef CDimension< 2 >	CDim2;		///< 2 dimensions
typedef CDimension< 3 >	CDim3;		///< 3 dimensions

} // end of namespace sueLib

#endif
