#ifndef __AEMovieCacheEx_H__
#define __AEMovieCacheEx_H__

#include "movie/movie.hpp"
#include "extensions/ExtensionMacros.h"
#include "AEMovieData.h"

NS_CC_EXT_BEGIN;

//
// FIXME: this must be owned by Director similar to TextureCache
//

class CC_DLL AEMovieCache : public cocos2d::Ref
{
public:
    AEMovieData * addMovie( const std::string & path, const std::string & name );

    void removeUnusedMovies();

    static AEMovieCache * getInstance();
    static void destroyInstance();

protected:
    aeMovieInstance * _instance;

    cocos2d::Map<std::string, AEMovieData *> _movies;

    static AEMovieCache * s_sharedInstance;

private:
    AEMovieCache();
    virtual ~AEMovieCache();
};

NS_CC_EXT_END;

#endif
