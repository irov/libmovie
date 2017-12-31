#ifndef __AEMovieDataEx_H__
#define __AEMovieDataEx_H__

#include "extensions/ExtensionMacros.h"
#include "AESound.h"

NS_CC_EXT_BEGIN;

class AEMovieData : public cocos2d::Ref
{
public:
    AEMovieData();
    virtual ~AEMovieData();

    // path must end with '/'
    virtual bool initWithFile( aeMovieInstance * instance, const std::string & path, const std::string & name );

    const aeMovieData * getData() const { return _data; }

protected:
    static void * callbackResourceProvider( const aeMovieResource * _resource, ae_voidptr_t _data );
    static void callbackResourceDeleter( aeMovieResourceTypeEnum _type, ae_voidptr_t _data, ae_voidptr_t _ud );

    cocos2d::Ref * createImage( const std::string & path, int width, int height );

    AESound * createSound( const std::string & path );


    // relative path to folder with movie data ending with '/' (eg. 'AEM/example/', where the .aem file & its referred resources are contained)
    std::string _path;

    // data received from the movie library
    aeMovieData * _data;

    typedef std::vector<AESound *> TVectorSound;
    TVectorSound _sounds;

private:
    CC_DISALLOW_COPY_AND_ASSIGN( AEMovieData );
};

NS_CC_EXT_END;

#endif
