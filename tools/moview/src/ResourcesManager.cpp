/******************************************************************************
* libMOVIE Software License v1.0
*
* Copyright (c) 2016-2026, Yuriy Levchenko <irov13@mail.ru>
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

#include "ResourcesManager.h"
#include "Sound.h"
#include "Platform.h"

//////////////////////////////////////////////////////////////////////////
#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_HDR
#define STBI_NO_PSD
#define STBI_NO_PIC
#define STBI_NO_PNM
#include <stb_image.h>
//////////////////////////////////////////////////////////////////////////
static uint32_t FNV1A_Hash( const std::string& str )
{
    uint32_t result = 0;
    const char * traitName = str.c_str();
    uint32_t len = static_cast<uint32_t>(str.length());
    uint32_t mult = 1u, c = 816753u;
    for( uint32_t i = 0; i < len; ++i )
    {
        uint32_t n = static_cast<uint32_t>(traitName[i]);

        result += n * mult;
        mult += c;
        c += (c + 13u) >> 1u;
    }

    return result;
}
//////////////////////////////////////////////////////////////////////////
ResourcesManager::ResourcesManager()
    : mWhiteTexture( 0 )
{
}
//////////////////////////////////////////////////////////////////////////
ResourcesManager::~ResourcesManager()
{
}
//////////////////////////////////////////////////////////////////////////
void ResourcesManager::Initialize()
{
    if( !mWhiteTexture )
    {
        uint32_t whitePixel = 0xFFFFFFFF;

        glGenTextures( 1, &mWhiteTexture );
        glBindTexture( GL_TEXTURE_2D, mWhiteTexture );
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &whitePixel );

        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

        glBindTexture( GL_TEXTURE_2D, 0 );
    }
}
//////////////////////////////////////////////////////////////////////////
void ResourcesManager::Shutdown()
{
    if( mWhiteTexture != 0 )
    {
        glDeleteTextures( 1, &mWhiteTexture );
        mWhiteTexture = 0;
    }

    PurgeAllResources();
}
//////////////////////////////////////////////////////////////////////////
GLuint ResourcesManager::GetWhiteTexture() const
{
    return mWhiteTexture;
}
//////////////////////////////////////////////////////////////////////////
ResourceTexture* ResourcesManager::GetTextureRes( const std::string& fileName )
{
    uint32_t hash = FNV1A_Hash( fileName );

    ResourcesTable::iterator it = mResources.find( hash );
    if( it != mResources.end() && it->second->type == Resource::Texture )
    {
        ResourceTexture* texture = static_cast<ResourceTexture*>( it->second );

        return texture;
    }

    ResourceTexture* texture = this->LoadTextureRes( fileName );

    return texture;
}
//////////////////////////////////////////////////////////////////////////
ResourceImage* ResourcesManager::GetImageRes( const std::string& imageName )
{
    uint32_t hash = FNV1A_Hash( imageName );

    ResourcesTable::iterator it = mResources.find( hash );
    if( it != mResources.end() && it->second->type == Resource::Image )
    {
        ResourceImage* image = static_cast<ResourceImage*>( it->second );

        return image;
    }

    ResourceImage* image = new ResourceImage();
    mResources.insert( { hash, image } );

    return image;
}
//////////////////////////////////////////////////////////////////////////
ResourceSound* ResourcesManager::GetSoundRes( const std::string& fileName )
{
    uint32_t hash = FNV1A_Hash( fileName );

    ResourcesTable::iterator it = mResources.find( hash );
    if( it != mResources.end() && it->second->type == Resource::Sound )
    {
        ResourceSound * sound = static_cast<ResourceSound*>( it->second );

        return sound;
    }

    ResourceSound * sound = LoadSoundRes( fileName );

    return sound;
}
//////////////////////////////////////////////////////////////////////////
void ResourcesManager::PurgeAllResources()
{
    for( ResourcesTable::value_type & p : mResources )
    {
        Resource* res = p.second;

        if( res->type == Resource::Texture )
        {
            ResourceTexture * tex = static_cast<ResourceTexture*>( res );
            glDeleteTextures( 1, &tex->texture );
        }

        delete res;
    }

    SoundDevice::Instance().DeleteAllSounds();

    mResources.clear();
}
//////////////////////////////////////////////////////////////////////////
ResourceTexture* ResourcesManager::LoadTextureRes( const std::string& fileName )
{
    int width;
    int height;
    int comp;

    uint8_t* data = stbi_load( fileName.c_str(), &width, &height, &comp, STBI_default );

    if( data == nullptr )
    {
        return nullptr;
    }

    ResourceTexture* texture = new ResourceTexture();
    texture->width = static_cast<uint32_t>(width);
    texture->height = static_cast<uint32_t>(height);

    GLint internalFmt;
    GLenum format;
    switch( comp )
    {
    case 1:
        {
            texture->format = ResourceTexture::R8;
            internalFmt = GL_R8;
            format = GL_RED;
        }break;
    case 2:
        {
            texture->format = ResourceTexture::R8G8;
            internalFmt = GL_RG8;
            format = GL_RG;
        }break;
    case 3:
        {
            texture->format = ResourceTexture::R8G8B8;
            internalFmt = GL_RGB8;
            format = GL_RGB;
        }break;
    case 4:
        {
            texture->format = ResourceTexture::R8G8B8A8;
            internalFmt = GL_RGBA8;
            format = GL_RGBA;
        }break;
    }

    glGenTextures( 1, &texture->texture );
    glBindTexture( GL_TEXTURE_2D, texture->texture );
    glTexImage2D( GL_TEXTURE_2D, 0, internalFmt, width, height, 0, format, GL_UNSIGNED_BYTE, data );

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

    glBindTexture( GL_TEXTURE_2D, 0 );

    stbi_image_free( data );

    uint32_t hash = FNV1A_Hash( fileName );
    mResources.insert( { hash, texture } );

    return texture;
}

ResourceSound * ResourcesManager::LoadSoundRes( const std::string& fileName )
{
    if( !Platform::CheckIfExists( fileName ) )
    {
        return nullptr;
    }

    Sound * sound = SoundDevice::Instance().CreateSound();
    if( !sound->LoadFromFile( fileName ) )
    {
        SoundDevice::Instance().DeleteSound( sound );
        return nullptr;
    }

    ResourceSound * soundRes = new ResourceSound();
    soundRes->sound = sound;

    uint32_t hash = FNV1A_Hash(fileName);
    mResources.insert( { hash, soundRes } );

    return soundRes;
}
