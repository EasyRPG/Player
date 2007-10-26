/**
	@file
	@brief		Multidimensional array used in map
	@author		sue445
*/
#ifndef _INC_CMAPTABLE
#define _INC_CMAPTABLE

#include "sueLib_Compile.h"
#include "CDimension.h"
#include <map>
using namespace std;

namespace sueLib {

template< class ValueType, unsigned int DIMENSION >
class CMapTable{
private:
	map< CDimension< DIMENSION >, ValueType >	m_Data;	///< Main data block
	ValueType		m_Default;	///< Initial value without key

public:
	/// Default constructor
	CMapTable(){}

	//=============================================================================
	/**
		Initialization

		@param		def	[in] initial value if no key is found

		@note		値にデフォルトコンストラクタが実装されていれば実行する必要は無い\n
					あくまで値にintなどが指定された時の救済策
	*/
	//=============================================================================
	void Init(ValueType def)
	{
		Release();
		m_Default = def;
	}

	//=============================================================================
	/**
		Release
	*/
	//=============================================================================
	void Release()
	{
		m_Data.clear();
	}

	//=============================================================================
	/**
		Data exists?

		@retval	true		Found
		@retval	false		Not found
	*/
	//=============================================================================
	bool IsExist(CDimension< DIMENSION > dim) const
	{
//		map< CDimension, ValueType >::const_iterator it = m_Data.find(dim);
		return (m_Data.find(dim) != m_Data.end());
	}

	//=============================================================================
	/**
		Setting data

		@param	dim		[in] Coordinates
		@param	bOverWrite	[in] if the key overlaps, true:overwrite / false: not overwrite
	*/
	//=============================================================================
	void Set(ValueType data, CDimension< DIMENSION > dim, bool bOverWrite=true)
	{
		map< CDimension< DIMENSION >, ValueType >::const_iterator it = m_Data.find(dim);
		if(it == m_Data.end()){
			m_Data.insert( pair< CDimension< DIMENSION >, ValueType >(dim, data));
		}
		else if(bOverWrite){
			m_Data[dim] = data;
		}
	}

	//=============================================================================
	/**
		Getting data

		@param	dim			[in] Coordinates
		@return				Got value (if no key found, returns initial value)
	*/
	//=============================================================================
	ValueType Get(CDimension< DIMENSION > dim) const
	{
		map< CDimension< DIMENSION >, ValueType >::const_iterator it = m_Data.find(dim);
		if(it != m_Data.end())		return it->second;
		return m_Default;
	}
};

} // end of namespace sueLib

#endif
