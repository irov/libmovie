#include "cocos2d.h"
#include "3d/CCBundleReader.h"
#include "extensions/cocos-ext.h"
#include "audio/include/AudioEngine.h"
#include "AEMovie.h"

NS_CC_EXT_BEGIN;

USING_NS_CC;
using namespace cocos2d::experimental;

//===============================================

static size_t read_file( void * _data, void * _buff, size_t _carriage, uint32_t _size ) {
    (void)_carriage;
    BundleReader * reader = static_cast<BundleReader *>(_data);
    return reader->read( _buff, 1, _size );
}

static void memory_copy( void * _data, const void * _src, void * _dst, size_t _size ) {
    (void)_data;
    memcpy( _dst, _src, _size );
}

//===============================================

void *AEMovieData::callbackResourceProvider( const aeMovieResource * _resource, void * _data ) {
    CCLOG( "CALL: resource provider" );

    AEMovieData *data = static_cast<AEMovieData *>(_data);

    switch( _resource->type )
    {
    case AE_MOVIE_RESOURCE_IMAGE:
        {
            const aeMovieResourceImage * r = (const aeMovieResourceImage *)_resource;

            CCLOG( "Resource type: image." );
            CCLOG( " path        = '%s'", r->path );
            CCLOG( " trim_width  = %i", (int)r->trim_width );
            CCLOG( " trim_height = %i", (int)r->trim_height );

            Ref * ref = data->createImage( r->path, (int)r->trim_width, (int)r->trim_height );
            return ref;
        }
    case AE_MOVIE_RESOURCE_VIDEO:
        {
            //			const aeMovieResourceVideo * r = (const aeMovieResourceVideo *)_resource;

            CCLOG( "Resource type: video." );

            //			Ref * rr = data->createResourceVideo( r );
            //			return rr;
            break;
        }
    case AE_MOVIE_RESOURCE_SOUND:
        {
            const aeMovieResourceSound * r = (const aeMovieResourceSound *)_resource;

            CCLOG( "Resource type: sound." );
            CCLOG( " path        = '%s'", r->path );

            AESound *sound = data->createSound( r->path );
            return sound;
        }
    case AE_MOVIE_RESOURCE_SLOT:
        {
            const aeMovieResourceSlot * r = (const aeMovieResourceSlot *)_resource;

            CCLOG( "Resource type: slot." );
            CCLOG( " width  = %i", r->width );
            CCLOG( " height = %i", r->height );
            break;
        }
    default:
        {
            CCLOG( "Resource type: other." );
            break;
        }
    }

    return nullptr;
}

void AEMovieData::callbackResourceDeleter( aeMovieResourceTypeEnum _type, void * _data, void * _ud )
{
    (void)_type;
    (void)_data;
    (void)_ud;
}

//===============================================

AEMovieData::AEMovieData()
    : _data( nullptr )
{
}

AEMovieData::~AEMovieData()
{
    if( _data ) {
        CCLOG( "Deleting movie data." );
        ae_delete_movie_data( _data );
        //		_data = nullptr;
    }

    //	_path.clear();

    for( TVectorSound::iterator soundIt = _sounds.begin(); soundIt != _sounds.end(); soundIt++ )
        CC_SAFE_RELEASE( *soundIt );
}

bool AEMovieData::initWithFile( aeMovieInstance * instance, const std::string & path, const std::string & name )
{
    if( path.empty() || name.empty() )
    {
        CCLOG( "AEMovieData::initWithFile(): blank resource filename." );
        return false;
    }

    // get full path for the .aem file
    _path = path + name + "/";
    std::string relPath = _path + name + ".aem";
    std::string fullPath = FileUtils::getInstance()->fullPathForFilename( relPath );

    CCLOG( "Initializing with file '%s'.", fullPath.c_str() );

    Data data = FileUtils::getInstance()->getDataFromFile( fullPath );

    if( data.isNull() )
    {
        CCLOG( "'%s' not found.", fullPath.c_str() );
        return nullptr;
    }

    BundleReader reader;
    reader.init( (char *)(data.getBytes()), data.getSize() );

    _data = ae_create_movie_data( instance, &callbackResourceProvider, &callbackResourceDeleter, this );
    aeMovieStream * stream = ae_create_movie_stream( instance, &read_file, &memory_copy, &reader );
    int r = ae_load_movie_data( _data, stream );
    ae_delete_movie_stream( stream );

    //	data.clear();

    if( r == AE_MOVIE_FAILED )
    {
        CCLOG( "Failed to load movie data." );
        return false;
    }

    return true;
}

Ref *AEMovieData::createImage( const std::string & path, int width, int height ) {
    std::string fileName = _path + path;
    std::replace( fileName.begin(), fileName.end(), '\\', '/' );
    CCLOG( "createImage fileName = %s", fileName.c_str() );

    Texture2D *texture = Director::getInstance()->getTextureCache()->addImage( fileName );

    Texture2D::TexParams tp;
    tp.magFilter = GL_LINEAR;
    tp.minFilter = GL_LINEAR;
    tp.wrapS = GL_CLAMP_TO_EDGE;
    tp.wrapT = GL_CLAMP_TO_EDGE;

    texture->setTexParameters( tp );

    return texture;
}

AESound *AEMovieData::createSound( const std::string & path ) {
    std::string fileName = _path + path;
    std::replace( fileName.begin(), fileName.end(), '\\', '/' );

    //	AudioEngine::preload(path);

    auto sound = AESound::create( fileName );
    sound->retain();

    _sounds.push_back( sound );

    return sound;
}

NS_CC_EXT_END;
