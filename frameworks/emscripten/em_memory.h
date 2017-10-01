#include <malloc.h>
//////////////////////////////////////////////////////////////////////////
#define EM_NEW(T) (T *)malloc(sizeof(T))
#define EM_DELETE(P) free(P)
#define EM_MALLOC(S) malloc(S)
#define EM_FREE(S) free(S)