#ifndef MOVIE_TYPE_H_
#define MOVIE_TYPE_H_

#	include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

	typedef int8_t result_t;

	typedef enum
	{
		MOVIE_SUCCESSFUL = 0,
		MOVIE_FAILED = -1
	} aeResult;
	
#ifdef __cplusplus
}
#endif

#endif