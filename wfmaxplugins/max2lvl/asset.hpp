//============================================================================
// asset.hp:
// Copyright(c) 1996 Cave Logic Studios / PF.Magic
//============================================================================
/* Documentation:

	Abstract:
			class containing array of assets, plua s class for packing asset id's
	History:
			Created	6/19/96 11:51AM Kevin T. Seghetti

	Class Hierarchy:
			none

	Dependancies:
		PIGS, STL, iostream
	Restrictions:

	Example:
*/
//============================================================================

// use only once insurance
#ifndef _aSSET_HP
#define _aSSET_HP

//============================================================================

#include "global.hpp"
#include "pigtool.h"
#include <stl/bstring.h>
#include <stl/vector.h>

//============================================================================

class AssetExtensions
{
	vector<string> extensions;
public:
	AssetExtensions() {}
	~AssetExtensions() {}
	void Load(istream& in);
	void Save(ostream& out);

	int Entries();
	const string& operator[](int index);
	int Lookup(const string& extension);
};

//============================================================================

class packedAssetID
{
	enum
	 {
		INDEX_BITLENGTH = 12,
		ROOM_BITLENGTH = 12,
		TYPE_BITLENGTH = 8,

		INDEX_B = 0,
		ROOM_B = INDEX_B+INDEX_BITLENGTH,
		TYPE_B = ROOM_B+ROOM_BITLENGTH,

		INDEX_LENGTH = 1 << INDEX_BITLENGTH,
		ROOM_LENGTH = 1 << ROOM_BITLENGTH,
		TYPE_LENGTH = 1 << TYPE_BITLENGTH,

		INDEX_M = 0XFFF,
		ROOM_M = 0XFFF << 	ROOM_B,
		TYPE_M = 0XFF << TYPE_B
	 };

public:
	enum { PERMANENT_ROOM = ROOM_LENGTH-1 }; 						// set to 0xfff
	packedAssetID() { packedID = 0; }
	packedAssetID(int32 newPackedID) { packedID = newPackedID; }
	int32 Room(void) const { return((ROOM_M & packedID) >> ROOM_B); }
	int32 Type(void) const { return((TYPE_M & packedID) >> TYPE_B); }
	int32 Index(void) const { return((INDEX_M & packedID) >> INDEX_B); }

	int32 Room(int32 newRoom) { assert(newRoom >= 0); assert(newRoom < ROOM_LENGTH); packedID = (packedID & ~ROOM_M) | ((newRoom << ROOM_B) & ROOM_M); return(Room()); }
	int32 Type(int32 newType) { assert(newType >= 0); assert(newType < TYPE_LENGTH); packedID = (packedID & ~TYPE_M) | ((newType << TYPE_B) & TYPE_M); return(Type()); }
	int32 Index(int32 newIndex) { assert(newIndex >= 0); assert(newIndex < INDEX_LENGTH); packedID = (packedID & ~INDEX_M) | ((newIndex << INDEX_B) & INDEX_M); return(Index()); }

	int32 ID(void) const { return(packedID); }
	int32 ID(int32 newID) { assert(newID); packedID = newID; return(packedID); }

	int operator!=(const packedAssetID& left) const { return (packedID != left.packedID); }
	int operator==(const packedAssetID& left) const { return (packedID == left.packedID); }
	packedAssetID& operator=(const packedAssetID& left) { packedID = left.packedID; return *this; }
	friend ostream& operator<<(ostream& s, const packedAssetID &a) { return s << "Type: " << a.Type() << "; Room: " << a.Room() << "; Index: " << a.Index(); }
private:
	int32 packedID;
};

//============================================================================
// asset numbering scheme: 32 bit #: type:8, room:12, id:12 bits


class asset
{
public:
	asset() : _id() {  }
	asset(const char* name, packedAssetID id) { _name = string(name); _id = id; }
	const string& Name(void) { return _name; }
	const string& Name(const string& name) { _name = name; return _name; }
//	int32 ID(void) const { return _id.ID(); }
	packedAssetID ID(void) const { return _id; }
	int32 ID(int32 newID) { _id = packedAssetID(newID); return newID; }

	friend ostream& operator<<(ostream& s, const asset &a) { return s << a._name << ',' << hex << a._id.ID() << dec; }

	bool operator==(const asset &c) const { return ( (_name == c._name) && (_id == c._id) ); }
	asset& operator=(const asset& left) { _name = left._name; _id = left._id; return *this; }

private:
	string _name;
	packedAssetID _id;
};

//============================================================================
#endif
//============================================================================
