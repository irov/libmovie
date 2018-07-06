#include "Movie.h"

#include "ResourcesManager.h"
#include "Composition.h"

#include "Logger.h"

#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#include <algorithm>

//////////////////////////////////////////////////////////////////////////
AE_CALLBACK ae_voidptr_t my_alloc( ae_voidptr_t, ae_size_t _size ) 
{
    return malloc( _size );
}
//////////////////////////////////////////////////////////////////////////
AE_CALLBACK ae_voidptr_t my_alloc_n( ae_voidptr_t, ae_size_t _size, ae_size_t _count ) 
{
    ae_size_t total = _size * _count;

    return malloc( total );
}
//////////////////////////////////////////////////////////////////////////
AE_CALLBACK ae_void_t my_free( ae_voidptr_t, ae_constvoidptr_t _ptr ) 
{
    free( (ae_voidptr_t)_ptr );
}
//////////////////////////////////////////////////////////////////////////
AE_CALLBACK ae_void_t my_free_n( ae_voidptr_t, ae_constvoidptr_t _ptr ) 
{
    free( (ae_voidptr_t)_ptr );
}
//////////////////////////////////////////////////////////////////////////
AE_CALLBACK ae_int32_t my_strncmp( ae_voidptr_t, const ae_char_t * _src, const ae_char_t * _dst, ae_size_t _count ) 
{
    return (ae_int32_t)strncmp( _src, _dst, _count );
}
//////////////////////////////////////////////////////////////////////////
AE_CALLBACK ae_void_t my_logerror( ae_voidptr_t, aeMovieErrorCode, const ae_char_t* _format, ... ) 
{
    va_list argList;
    va_start( argList, _format );
    vprintf( _format, argList );
    va_end( argList );
}
//////////////////////////////////////////////////////////////////////////
struct MemoryIO 
{
    const uint8_t * data;
    ae_size_t dataLength;
    ae_size_t cursor;
};
//////////////////////////////////////////////////////////////////////////
AE_CALLBACK ae_size_t my_io_read( ae_voidptr_t _data, ae_voidptr_t _buff, ae_size_t, ae_size_t _size ) 
{
    MemoryIO* io = reinterpret_cast<MemoryIO*>(_data);

    if( io == nullptr )
    {
        return 0;
    }

    if( io->cursor >= io->dataLength )
    {
        return 0;
    }
     
    const ae_size_t dataAvailable = io->dataLength - io->cursor;
    const ae_size_t toRead = (_size > dataAvailable) ? dataAvailable : _size;
    memcpy( _buff, io->data + io->cursor, toRead );
    io->cursor += toRead;

    return toRead;
}
//////////////////////////////////////////////////////////////////////////
AE_CALLBACK ae_void_t my_memory_copy( ae_voidptr_t, ae_constvoidptr_t _src, ae_voidptr_t _dst, ae_size_t _size ) 
{
    memcpy( _dst, _src, _size );
}
//////////////////////////////////////////////////////////////////////////
AE_CALLBACK ae_bool_t my_resource_provider( const aeMovieResource * _resource, ae_voidptrptr_t _rd, ae_voidptr_t _ud ) 
{
    AE_UNUSED( _resource );
    AE_UNUSED( _rd );
    AE_UNUSED( _ud );
}
//////////////////////////////////////////////////////////////////////////
AE_CALLBACK ae_void_t my_resource_deleter( aeMovieResourceTypeEnum _type, ae_voidptr_t _data, ae_voidptr_t _ud ) 
{
    AE_UNUSED( _type );
    AE_UNUSED( _data );
    AE_UNUSED( _ud );
}
//////////////////////////////////////////////////////////////////////////
Movie::Movie()
    : mMovieInstance( nullptr )
    , mMovieData( nullptr )
{
}
//////////////////////////////////////////////////////////////////////////
Movie::~Movie()
{
    this->Close();
}
//////////////////////////////////////////////////////////////////////////
bool Movie::LoadFromFile( const std::string& fileName, const std::string& licenseHash )
{
    FILE* f = fopen( fileName.c_str(), "rb" );
    
    if( f == NULL )
    {
        return false;
    }

    fseek( f, 0, SEEK_END );
    uint32_t fileLen = static_cast<uint32_t>(ftell( f ));
    fseek( f, 0, SEEK_SET );

    std::vector<uint8_t> buffer( fileLen );
    fread( buffer.data(), 1, fileLen, f );
    fclose( f );

    std::string baseFolder;

    // looking for last delimiter to extract base folder (we try both forward and backward slashes)
    std::string::size_type lastDelimiter = fileName.find_last_of( '\\' );
    if( lastDelimiter == std::string::npos ) 
    {
        lastDelimiter = fileName.find_last_of( '/' );
    }

    if( lastDelimiter != std::string::npos ) 
    {
        baseFolder = fileName.substr( 0, lastDelimiter + 1 );
    }

    if( this->LoadFromMemory( buffer.data(), fileLen, baseFolder, licenseHash ) == false )
    {
        return false;
    }

    return true;
}
//////////////////////////////////////////////////////////////////////////
bool Movie::LoadFromMemory( const void* data, size_t dataLength, const std::string& baseFolder, const std::string& licenseHash )
{
    this->Close();

    const aeMovieInstance* movie = ae_create_movie_instance( licenseHash.c_str(),
        &my_alloc,
        &my_alloc_n,
        &my_free,
        &my_free_n,
        &my_strncmp,
        &my_logerror,
        nullptr );

    if( movie == AE_NULL )
    {
        return false;
    }

    MemoryIO io;
    io.data = reinterpret_cast<const uint8_t*>(data);
    io.dataLength = dataLength;
    io.cursor = 0;

    aeMovieStream* stream = ae_create_movie_stream( movie, &my_io_read, &my_memory_copy, &io );

    aeMovieDataProviders dataProviders;
    ae_clear_movie_data_providers( &dataProviders );

    dataProviders.resource_provider = []( const aeMovieResource * _resource, ae_voidptrptr_t _rd, ae_voidptr_t _ud ) -> ae_bool_t 
    {
        Movie* _this = reinterpret_cast<Movie*>(_ud);

        if( _this->OnProvideResource( _resource, _rd, _ud ) == false )
        {
            return AE_FALSE;
        }
        
        return AE_TRUE;
    };

    dataProviders.resource_deleter = []( aeMovieResourceTypeEnum _type, ae_voidptr_t _data, ae_voidptr_t _ud ) 
    {
        Movie* _this = reinterpret_cast<Movie*>(_ud);

        _this->OnDeleteResource( _type, _data, _ud );
    };

    aeMovieData * movie_data = ae_create_movie_data( movie, &dataProviders, this );

    // save the base folder, we'll need it later to look for resources
    mBaseFolder = baseFolder;

    ae_uint32_t major_version;
    ae_uint32_t minor_version;
    ae_result_t movie_data_result = ae_load_movie_data( movie_data, stream, &major_version, &minor_version );

    if( movie_data_result != AE_RESULT_SUCCESSFUL ) 
    {
        ae_delete_movie_data( movie_data );
        ae_delete_movie_stream( stream );
        ae_delete_movie_instance( movie );

        return false;
    }

    // now we can free the stream as all the data is now loaded
    ae_delete_movie_stream( stream );
    mMovieInstance = movie;
    mMovieData = movie_data;
    // Hacky way to find compositions ;)
    ae_visit_movie_layer_data( mMovieData
        , []( const aeMovieCompositionData* _compositionData, const aeMovieLayerData* _layer, ae_voidptr_t _ud ) -> ae_bool_t 
    {
        Movie* _this = reinterpret_cast<Movie*>(_ud);

        if( ae_is_movie_composition_data_master( _compositionData ) == AE_FALSE )
        {
            return AE_TRUE;
        }
            
        _this->AddCompositionData( _compositionData );

        return AE_TRUE;
    }, this );

    return true;
}
//////////////////////////////////////////////////////////////////////////
void Movie::Close() 
{
    if( mMovieData != nullptr )
    {
        ae_delete_movie_data( mMovieData );
        mMovieData = nullptr;
    }

    if( mMovieInstance != nullptr )
    {
        ae_delete_movie_instance( mMovieInstance );
        mMovieInstance = nullptr;
    }

    mCompositionDatas.clear();
}
//////////////////////////////////////////////////////////////////////////
Composition* Movie::OpenComposition( const std::string& name )
{
    if( mMovieData == nullptr )
    {
        return nullptr;
    }

    const aeMovieCompositionData* compData = ae_get_movie_composition_data( mMovieData, name.c_str() );

    if( compData == nullptr )
    {
        return nullptr;
    }

    Composition * result = new Composition();

    if( result->Create( mMovieData, compData ) == false )
    {
        delete result;

        return nullptr;
    }

    return result;
}
//////////////////////////////////////////////////////////////////////////
void Movie::CloseComposition( Composition* composition )
{
    if( composition != nullptr ) 
    {
        delete composition;
    }
}
//////////////////////////////////////////////////////////////////////////
uint32_t Movie::GetMainCompositionsCount() const
{
    VectorMovieCompositionData::size_type size = mCompositionDatas.size();

    return size;
}
//////////////////////////////////////////////////////////////////////////
std::string Movie::GetMainCompositionNameByIdx( uint32_t idx ) const 
{
    if( idx >= mCompositionDatas.size() )
    {
        return std::string();
    }

    const aeMovieCompositionData* compData = mCompositionDatas[idx];

    std::string result = ae_get_movie_composition_data_name( compData );

    return result;
}
//////////////////////////////////////////////////////////////////////////
Composition* Movie::OpenMainCompositionByIdx( uint32_t idx ) const 
{
    if( idx >= mCompositionDatas.size() )
    {
        return nullptr;
    }

    const aeMovieCompositionData* compData = mCompositionDatas[idx];

    if( compData == AE_NULL )
    {
        return nullptr;
    }

    Composition * result = new Composition();
    
    if( result->Create( mMovieData, compData ) == false )
    {
        delete result;

        return nullptr;
    }
     
    return result;
}
//////////////////////////////////////////////////////////////////////////
uint32_t Movie::FindMainCompositionIdx( Composition* composition ) const 
{
    if( composition == nullptr )
    {
        return ~0u;
    }

    const std::string & compName = composition->GetName();
    uint32_t count = this->GetMainCompositionsCount();

    for( uint32_t i = 0, end = count; i < end; ++i )
    {
        std::string testCompositionName = this->GetMainCompositionNameByIdx( i );

        if( testCompositionName != compName )
        {
            continue;
        }
         
        return i;
    }
    
    return ~0u;
}
//////////////////////////////////////////////////////////////////////////
Composition* Movie::OpenDefaultComposition() 
{
    if( mCompositionDatas.empty() == true )
    {
        return nullptr;
    }

    const aeMovieCompositionData* compData = mCompositionDatas.front();

    if( compData == AE_NULL )
    {
        return nullptr;
    }

    Composition* result = new Composition();

    if( result->Create( mMovieData, compData ) == false )
    {
        delete result;

        return nullptr;
    }

    return result;
}
//////////////////////////////////////////////////////////////////////////
void Movie::AddCompositionData( const aeMovieCompositionData* compositionData ) 
{
    if( std::find( mCompositionDatas.cbegin(), mCompositionDatas.cend(), compositionData ) != mCompositionDatas.cend() ) 
    {
        return;        
    }

    mCompositionDatas.push_back( compositionData );
}
//////////////////////////////////////////////////////////////////////////
bool Movie::OnProvideResource( const aeMovieResource* _resource, void** _rd, void* _ud )
{
    AE_UNUSED( _ud );

    ViewerLogger << "Resource provider callback." << std::endl;

    switch( _resource->type )
    {
    case AE_MOVIE_RESOURCE_IMAGE:
        {
            const aeMovieResourceImage* ae_image = reinterpret_cast<const aeMovieResourceImage*>(_resource);

            ViewerLogger << "Resource type: image." << std::endl;
            ViewerLogger << " path        : '" << ae_image->path << "'" << std::endl;
            ViewerLogger << " trim_width  : " << static_cast<int>(ae_image->trim_width) << std::endl;
            ViewerLogger << " trim_height : " << static_cast<int>(ae_image->trim_height) << std::endl;
            ViewerLogger << " has mesh    : " << (ae_image->mesh != nullptr ? "YES" : "NO") << std::endl;

            std::string fullPath = mBaseFolder + ae_image->path;

            if( ae_image->atlas_image == AE_NULL ) 
            {
                *_rd = reinterpret_cast<ae_voidptr_t>(ResourcesManager::Instance().GetTextureRes( fullPath ));
            }
            else
            {
                std::string texturePath = mBaseFolder + ae_image->atlas_image->path;

                ResourceImage* image = ResourcesManager::Instance().GetImageRes( ae_image->name );

                image->textureRes = ResourcesManager::Instance().GetTextureRes( texturePath );
                image->premultAlpha = (ae_image->is_premultiplied == AE_TRUE);

                *_rd = reinterpret_cast<ae_voidptr_t>(image);
            }
        } break;
    case AE_MOVIE_RESOURCE_SEQUENCE:
        {
            ViewerLogger << "Resource type: image sequence." << std::endl;
        } break;
    case AE_MOVIE_RESOURCE_VIDEO:
        {
            const aeMovieResourceVideo * r = (const aeMovieResourceVideo *)_resource;

            ViewerLogger << "Resource type: video." << std::endl;
            ViewerLogger << " path        : '" << r->path << "'" << std::endl;
        } break;
    case AE_MOVIE_RESOURCE_SOUND:
        {
            const aeMovieResourceSound * r = (const aeMovieResourceSound *)_resource;

            ViewerLogger << "Resource type: sound." << std::endl;
            ViewerLogger << " path        : '" << r->path << "'" << std::endl;
        } break;
    case AE_MOVIE_RESOURCE_SLOT:
        {
            const aeMovieResourceSlot * r = (const aeMovieResourceSlot *)_resource;

            ViewerLogger << "Resource type: slot." << std::endl;
            ViewerLogger << " width  = " << r->width << std::endl;
            ViewerLogger << " height = " << r->height << std::endl;
        } break;
    default: 
        {
            ViewerLogger << "Resource type: other (" << _resource->type << ")" << std::endl;
        } break;
    }

    return AE_TRUE;
}
//////////////////////////////////////////////////////////////////////////
void Movie::OnDeleteResource( uint32_t _type, void* _data, void* _ud )
{
    AE_UNUSED( _type );
    AE_UNUSED( _data );
    AE_UNUSED( _ud );
}
