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

#pragma once

#include "Singleton.h"

#include "glad/glad.h"

#include <string>
#include <unordered_map>

struct Resource
{
    enum : uint32_t 
    {
        Solid = 0,
        Video,
        Sound,
        Image,
        Sequence,
        Particle,
        Slot,

        // System
        Texture = 1000
    };

    Resource( uint32_t _type )
        : type( _type )
    {
    }

    uint32_t  type;
};
//////////////////////////////////////////////////////////////////////////
struct ResourceTexture 
    : public Resource 
{
    enum : uint32_t 
    {
        R8 = 0,
        R8G8,
        R8G8B8,
        R8G8B8A8
    };

    ResourceTexture()
        : Resource( Resource::Texture )
    {
    }

    uint32_t  width;
    uint32_t  height;
    uint32_t  format;
    GLuint  texture;
};
//////////////////////////////////////////////////////////////////////////
struct ResourceImage 
    : public Resource 
{
    ResourceImage()
        : Resource( Resource::Image )
        , textureRes( nullptr )
        , premultAlpha( true )
    {
    }

    ResourceTexture * textureRes;
    bool premultAlpha;
};
//////////////////////////////////////////////////////////////////////////
struct ResourceSound
    : public Resource
{
    ResourceSound()
        : Resource( Resource::Sound )
        , sound( nullptr )
    {
    }

    class Sound * sound;
};
//////////////////////////////////////////////////////////////////////////
class ResourcesManager
    : public Singleton<ResourcesManager>
{
public:
    ResourcesManager();
    ~ResourcesManager();

public:
    void Initialize();
    void Shutdown();

public:
    GLuint GetWhiteTexture() const;

    ResourceTexture * GetTextureRes( const std::string& fileName );
    ResourceImage * GetImageRes( const std::string& imageName );
    ResourceSound * GetSoundRes( const std::string& fileName );

    void PurgeAllResources();

protected:
    ResourceTexture * LoadTextureRes( const std::string& fileName );
    ResourceSound * LoadSoundRes( const std::string& fileName );

protected:
    GLuint mWhiteTexture;

    typedef std::unordered_map<uint32_t, Resource*> ResourcesTable;
    ResourcesTable mResources;
};
