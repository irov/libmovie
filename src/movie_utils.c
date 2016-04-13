#	include "movie_utils.h"

//////////////////////////////////////////////////////////////////////////
int32_t ae_strcmp( const char * _src, const char * _dst )
{
	int32_t cmp = 0;

	while( !(cmp = *(unsigned char *)_src - *(unsigned char *)_dst) && *_dst )
	{
		++_src;
		++_dst;
	}
	
	return cmp;
}
//////////////////////////////////////////////////////////////////////////
int32_t ae_strncmp( const char * _src, const char * _dst, uint32_t _count )
{
	if( _count == 0 )
	{
		return 0;
	}

	int32_t cmp = 0;

	while( !(cmp = *(unsigned char *)_src - *(unsigned char *)_dst) && *_dst && _count-- )
	{
		++_src;
		++_dst;
	}

	return cmp;
}
//////////////////////////////////////////////////////////////////////////