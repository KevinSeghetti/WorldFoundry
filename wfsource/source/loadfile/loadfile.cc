// loadfile.cc

#include <loadfile/loadfile.hp>

#include <cstdlib>
#include <cstdio>
#include <pigsys/pigsys.hp>
#include <cstring>

#if 0
static long
ffilesize( FILE *fp )
{
	ValidatePtr( fp );

	long lSavedPos = ftell( fp );
	fseek( fp, 0, SEEK_END );
	long lFileSize = ftell( fp );
	fseek( fp, lSavedPos, SEEK_SET );

	return( lFileSize );
}


static bool
fileExists( const char* szFilename )
{
	FILE* fp = fopen( szFilename, "r" );
	if ( fp )
	{
		fclose( fp );
		return true;
	}
	else
		return false;
}
#endif


void*
LoadBinaryFile( const char* _szFilename, int32& nSize )
{
	FILE* fp;
	void* ptr;
	const char* szFilename = _szFilename;

	fp = fopen( (char*)szFilename, "rb" );
	if ( !fp )
		return NULL;

	fseek( fp, 0L, SEEK_END );
	assert( !ferror( fp ) );
	nSize = ftell( fp );
	fseek( fp, 0L, SEEK_SET );
	assert( !ferror( fp ) );

	ptr = malloc( nSize );
	ValidatePtr( ptr );
	if ( ptr )
	{
		int cbRead = fread( ptr, 1, nSize, fp );
		assert( !ferror( fp ) );
		assert( cbRead == nSize );
	}

	AssertMsg( !ferror( fp ), "ferror(fp) = " << ferror( fp) );
	fclose( fp );
    // kts Duh!, the error is it is closed
	//AssertMsg( !ferror( fp ), "ferror(fp) = " << ferror( fp) );

	return ptr;
}


char*
LoadTextFile( const char* _szFilename, int32& nSize )
{
	FILE* fp;
	char* ptr;
	const char* szFilename = _szFilename;

	fp = fopen( (char*)szFilename, "rt" );
	if ( !fp )
		return NULL;

	fseek( fp, 0L, SEEK_END );
	AssertMsg( !ferror( fp ), "ferror(fp) = " << ferror( fp) );
	nSize = ftell( fp );
	fseek( fp, 0L, SEEK_SET );
	assert( !ferror( fp ) );

	ptr = (char*)malloc( nSize+1 );
	ValidatePtr( ptr );
	if ( ptr )
		{
		int cbRead = fread( ptr, 1, nSize, fp );
		assert( !ferror( fp ) );
		assert( cbRead <= nSize );
		*( ptr + cbRead ) = '\0';
		nSize = cbRead;
		}

	assert( !ferror( fp ) );
	fclose( fp );
    // kts Duh!, the error is it is closed
	//assert( !ferror( fp ) );

	return ptr;
}
