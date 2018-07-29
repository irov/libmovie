#include "movie/movie.h"

#include <stdlib.h>

static const ae_char_t * ex_example_license_hash = "52ad6f051099762d0a0787b4eb2d07c8a0ee4491";

int main( int argc, char *argv[] )
{
    AE_UNUSED( argc );
    AE_UNUSED( argv );

    const aeMovieInstance * instance = ae_create_movie_instance( ex_example_license_hash
        , (ae_movie_alloc_t)AE_NULL
        , (ae_movie_alloc_n_t)AE_NULL
        , (ae_movie_free_t)AE_NULL
        , (ae_movie_free_n_t)AE_NULL
        , (ae_movie_strncmp_t)AE_NULL
        , (ae_movie_logger_t)AE_NULL
        , AE_NULL );

    if( instance == AE_NULL )
    {
        return EXIT_FAILURE;
    }

    ae_delete_movie_instance( instance );

    return EXIT_SUCCESS;
}
