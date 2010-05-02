//=============================================================================
// textbuff.cc
//=============================================================================

#include <console/textbuff.hp>
#include <pigsys/pigsys.hp>

TextBuffer::TextBuffer( int width, int height )
{
	_width = width;
	_height = height;

	_screen = (unsigned char*)malloc( _width * _height + 1000 );
	ValidatePtr( _screen );

	clear();
	validate();
}


TextBuffer::~TextBuffer()
{
	ValidatePtr( _screen );
	free( _screen );
}


void
TextBuffer::validate() const
{
}


void
TextBuffer::clear()
{
	memset( _screen, ' ', _width * _height );
	validate();
}


void
TextBuffer::print( int x, int y, const unsigned char* msg, int nChars )
{
	strncpy( (char*)&_screen[ y*_width + x ], (char *)msg, nChars );
}


#undef putchar
void
TextBuffer::flush()
{
	validate();
	for ( int y=0; y<25; ++y )
	{
		for ( int x=0; x<40; ++x )
			putchar( _screen[ y*_width + x ] );
		putchar( '\n' );
	}
}
