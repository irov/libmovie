/******************************************************************************
* libMOVIE Software License v1.0
*
* Copyright (c) 2016-2018, Yuriy Levchenko <irov13@mail.ru>
* All rights reserved.
*
* You are granted a perpetual, non-exclusive, non-sublicensable, and
* non-transferable license to use, install, execute, and perform the libMOVIE
* software and derivative works solely for personal or internal
* use. Without the written permission of Yuriy Levchenko, you may not (a) modify, translate,
* adapt, or develop new applications using the libMOVIE or otherwise
* create derivative works or improvements of the libMOVIE or (b) remove,
* delete, alter, or obscure any trademarks or any copyright, trademark, patent,
* or other intellectual property or proprietary rights notices on or in the
* Software, including any copy thereof. Redistributions in binary or source
* form must include this license and terms.
*
* THIS SOFTWARE IS PROVIDED BY YURIY LEVCHENKO "AS IS" AND ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
* EVENT SHALL YURIY LEVCHENKO BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES, BUSINESS INTERRUPTION,
* OR LOSS OF USE, DATA, OR PROFITS) HOWEVER CAUSED AND ON ANY THEORY OF
* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
* OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************/

//#include "cocos2d.h"
//#include "extensions/cocos-ext.h"
//#include "audio/include/AudioEngine.h"
#include "AEMovieData.h"
#include "AEMovie.h"

NS_CC_EXT_BEGIN;

USING_NS_CC;
using namespace cocos2d::experimental;

//===============================================

namespace Detail
{
	static ae_size_t read_file( ae_voidptr_t _data, ae_voidptr_t _buff, ae_size_t _carriage, ae_size_t _size ) {
		(void)_carriage;
		BundleReader * reader = static_cast<BundleReader *>(_data);
		ssize_t rs = reader->read( _buff, 1, _size );

		return (ae_size_t)rs;
	}

	static void memory_copy( ae_voidptr_t _data, ae_constvoidptr_t _src, ae_voidptr_t _dst, ae_size_t _size ) {
		(void)_data;
		memcpy( _dst, _src, _size );
	}
}

//===============================================

void *AEMovieData::callbackResourceProvider( const aeMovieResource * _resource, ae_voidptr_t _data ) {
    //XCODE COMPILE COMMENT: CCLOG( "CALL: resource provider" );

    AEMovieData *data = static_cast<AEMovieData *>(_data);

    switch( _resource->type )
    {
    case AE_MOVIE_RESOURCE_IMAGE:
        {
            const aeMovieResourceImage * r = (const aeMovieResourceImage *)_resource;

            //XCODE COMPILE COMMENT: CCLOG( "Resource type: image." );
            //XCODE COMPILE COMMENT: CCLOG( " path        = '%s'", r->path );
            //XCODE COMPILE COMMENT: CCLOG( " trim_width  = %i", (int)r->trim_width );
            //XCODE COMPILE COMMENT: CCLOG( " trim_height = %i", (int)r->trim_height );

            Ref * ref = data->createImage( r->path, (int)r->trim_width, (int)r->trim_height );
            return ref;
        }
    case AE_MOVIE_RESOURCE_VIDEO:
        {
            //			const aeMovieResourceVideo * r = (const aeMovieResourceVideo *)_resource;

            //XCODE COMPILE COMMENT: CCLOG( "Resource type: video." );

            //			Ref * rr = data->createResourceVideo( r );
            //			return rr;
            break;
        }
    case AE_MOVIE_RESOURCE_SOUND:
        {
            const aeMovieResourceSound * r = (const aeMovieResourceSound *)_resource;

            //XCODE COMPILE COMMENT: CCLOG( "Resource type: sound." );
            //XCODE COMPILE COMMENT: CCLOG( " path        = '%s'", r->path );

            AESound *sound = data->createSound( r->path );
            return sound;
        }
    case AE_MOVIE_RESOURCE_SLOT:
        {
            const aeMovieResourceSlot * r = (const aeMovieResourceSlot *)_resource;
            AE_UNUSED(r);
            
            //XCODE COMPILE COMMENT: CCLOG( "Resource type: slot." );
            //XCODE COMPILE COMMENT: CCLOG( " width  = %i", r->width );
            //XCODE COMPILE COMMENT: CCLOG( " height = %i", r->height );
            break;
        }
    default:
        {
            //XCODE COMPILE COMMENT: CCLOG( "Resource type: other." );
            break;
        }
    }

    return nullptr;
}

void AEMovieData::callbackResourceDeleter( aeMovieResourceTypeEnum _type, ae_voidptr_t _data, ae_voidptr_t _ud )
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
        //XCODE COMPILE COMMENT: CCLOG( "Deleting movie data." );
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
        //XCODE COMPILE COMMENT: CCLOG( "AEMovieData::initWithFile(): blank resource filename." );
        return false;
    }

    // get full path for the .aem file
    _path = path + name + "/";
    std::string relPath = _path + name + ".aem";
    std::string fullPath = FileUtils::getInstance()->fullPathForFilename( relPath );

    //XCODE COMPILE COMMENT: CCLOG( "Initializing with file '%s'.", fullPath.c_str() );

    Data data = FileUtils::getInstance()->getDataFromFile( fullPath );

    if( data.isNull() )
    {
        //XCODE COMPILE COMMENT: CCLOG( "'%s' not found.", fullPath.c_str() );
        return false;
    }

    BundleReader reader;
    reader.init( (char *)(data.getBytes()), data.getSize() );

    _data = ae_create_movie_data( instance, &AEMovieData::callbackResourceProvider, &AEMovieData::callbackResourceDeleter, this );
    aeMovieStream * stream = ae_create_movie_stream( instance, &Detail::read_file, &Detail::memory_copy, &reader );
    int r = ae_load_movie_data( _data, stream );
    ae_delete_movie_stream( stream );

    //	data.clear();

    if( r != AE_RESULT_SUCCESSFUL )
    {
        //XCODE COMPILE COMMENT: CCLOG( "Failed to load movie data." );
        return false;
    }

    return true;
}

Ref *AEMovieData::createImage( const std::string & path, int width, int height ) {
    AE_UNUSED( width );
    AE_UNUSED( height );

    std::string fileName = _path + path;
    std::replace( fileName.begin(), fileName.end(), '\\', '/' );
    //XCODE COMPILE COMMENT: CCLOG( "createImage fileName = %s", fileName.c_str() );

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
