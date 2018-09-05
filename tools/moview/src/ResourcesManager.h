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

protected:
    ResourceTexture * LoadTextureRes( const std::string& fileName );
    ResourceSound * LoadSoundRes( const std::string& fileName );

protected:
    GLuint mWhiteTexture;

    typedef std::unordered_map<uint32_t, Resource*> ResourcesTable;
    ResourcesTable mResources;
};
