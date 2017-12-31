//#include "cocos2d.h"
#include "AEMovie.h"

NS_CC_EXT_BEGIN;

USING_NS_CC;

AEMovieCache * AEMovieCache::s_sharedInstance = nullptr;

//===============================================

static ae_voidptr_t stdlib_movie_alloc( ae_voidptr_t _data, ae_size_t _size ) {
    (void)_data;
    return malloc( _size );
}

static ae_voidptr_t stdlib_movie_alloc_n( ae_voidptr_t _data, ae_size_t _size, ae_size_t _count ) {
    (void)_data;
    ae_size_t total = _size * _count;
    return malloc( total );
}

static void stdlib_movie_free( ae_voidptr_t _data, ae_constvoidptr_t _ptr ) {
    (void)_data;
    free( (void *)_ptr );
}

static void stdlib_movie_free_n( ae_voidptr_t _data, ae_constvoidptr_t _ptr ) {
    (void)_data;
    free( (void *)_ptr );
}

static void stdlib_movie_logerror( ae_voidptr_t _data, aeMovieErrorCode _code, const ae_char_t * _format, ... ) {
    char dst[2048];
    (void)_data;
    (void)_code;

    va_list argList;

    va_start( argList, _format );
    //	vprintf( _format, argList );
    _vsnprintf( (char *)dst, sizeof( dst ), _format, argList );
    va_end( argList );

    CCLOG( dst );
}

//===============================================

AEMovieCache::AEMovieCache() {
    _instance = ae_create_movie_instance( "f86464bbdebf0fe3e684b03ec263d049d079e6f1"
        , &stdlib_movie_alloc
        , &stdlib_movie_alloc_n
        , &stdlib_movie_free
        , &stdlib_movie_free_n
        , (ae_movie_strncmp_t)AE_NULL
        , &stdlib_movie_logerror
        , AE_NULL );
}

AEMovieCache::~AEMovieCache() {
    if( _instance != nullptr ) {
        CCLOG( "Deleting movie instance." );
        ae_delete_movie_instance( _instance );
    }
}

AEMovieData * AEMovieCache::addMovie( const std::string & path, const std::string & name ) {
    std::string folder = path + name + "/";
    std::string relPath = folder + name + ".aem";

    auto it = _movies.find( relPath );

    if( it != _movies.end() ) {
        return it->second;
    }

    AEMovieData * data = new (std::nothrow) AEMovieData();

    if( data && data->initWithFile( _instance, path, name ) ) {
        _movies.insert( relPath, data );
        data->release();
        return data;
    }

    return nullptr;
}

void AEMovieCache::removeUnusedMovies() {
    for( auto it = _movies.cbegin(); it != _movies.cend(); ) {
        AEMovieData *data = it->second;

        if( data->getReferenceCount() == 1 ) {
            CCLOG( "AEMovieCache::removeUnusedMovies(): '%s'", it->first.c_str() );
            it = _movies.erase( it );
        }
        else {
            it++;
        }
    }
}

AEMovieCache * AEMovieCache::getInstance() {
    if( !s_sharedInstance ) {
        s_sharedInstance = new (std::nothrow) AEMovieCache();
        CCASSERT( s_sharedInstance, "FATAL: Not enough memory to create movie cache." );
    }

    return s_sharedInstance;
}

void AEMovieCache::destroyInstance() {
    CC_SAFE_RELEASE_NULL( s_sharedInstance );
}

NS_CC_EXT_END;
