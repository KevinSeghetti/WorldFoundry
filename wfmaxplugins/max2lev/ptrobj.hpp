//============================================================================
// Ptrobj.hp:
// Copyright(c) 1996 Cave Logic Studios / PF.Magic
//============================================================================
/* Documentation:

	Abstract:
			A PtrObj encapsulates a pointer to some other class, for use with
			STL containers.
	History:
			Created	08/12/96 16:18 by Phil Torre

	Class Hierarchy:
			none

	Dependancies:
	
	Restrictions:

	Example:
*/
//============================================================================
// use only once insurance
#ifndef _PTROBJ_HP
#define _PTROBJ_HP

//============================================================================

#include "global.hpp"



template <class T>
class PtrObj
{
public:
	PtrObj(T* thePointer) { _thePointer = thePointer; }
	PtrObj() { _thePointer = NULL; }
	~PtrObj() { }

	PtrObj& operator=(const PtrObj& other)
	{
		_thePointer = other._thePointer;
		return (*this);
	}

	bool operator==(const PtrObj& other) { return (*_thePointer == *other._thePointer); }
	bool operator>(const PtrObj& other) { return (*_thePointer > *other._thePointer); }
	operator T*(void) const { return(_thePointer); }
	operator T&(void) const { return(*_thePointer); }
	T* operator->() const { return(_thePointer); }

private:
	T* _thePointer;
};





#endif	/* _PTROBJ_HP */

