#include "Sound.h"
#include "Platform.h"

#include <stb_vorbis.h>

// Stupid Apple monkeys decided to "be different"  >:(
#ifdef PLATFORM_MACOS
#   include <OpenAL/al.h>
#   include <OpenAL/alc.h>
#else
#   include <AL/al.h>
#   include <AL/alc.h>
#endif

Sound::Sound()
    : mNumChannels( 0 )
    , mSampleRate( 0 )
    , mNumSamples( 0 )
    , mALSource( 0 )
    , mALBuffer( 0 )
{
}
Sound::~Sound()
{
}

bool Sound::LoadFromFile( const std::string & _path )
{
    bool result = false;

    short* soundData = nullptr;
    int channels, rate;
    const int numSamples = stb_vorbis_decode_filename( _path.c_str(), &channels, &rate, &soundData );
    if( numSamples > 0 )
    {
        mNumChannels = channels;
        mSampleRate = rate;
        mNumSamples = numSamples;

        // Create OpenAL source and initialize it
        alGenSources( 1, &mALSource );
        alSourcef( mALSource, AL_PITCH, 1.f );
        alSourcef( mALSource, AL_GAIN, 1.f );
        alSourcef( mALSource, AL_REFERENCE_DISTANCE, 1.f );
        alSourcei( mALSource, AL_LOOPING, AL_FALSE );
        alSourceStop( mALSource );

        // Create OpenAL buffer and fill with data
        alGenBuffers( 1, &mALBuffer );

        ALenum format = (2 == channels) ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;
        ALsizei dataSize = channels * sizeof(short) * numSamples;
        alBufferData( mALBuffer, format, soundData, dataSize, rate );

        // Attach buffer to the source
        alSourceQueueBuffers( mALSource, 1, &mALBuffer );

        // Free PCM data
        free( soundData );

        result = true;
    }

    return result;
}

void Sound::Destroy()
{
    if( mALSource )
    {
        alSourceStop( mALSource );
        alSourceUnqueueBuffers( mALSource, 1, &mALBuffer );
        alDeleteBuffers( 1, &mALBuffer );
        alDeleteSources( 1, &mALSource );

        mALSource = 0;
        mALBuffer = 0;
    }
}

void Sound::Play()
{
    if( !IsPlaying() )
    {
        if( IsStopped() )
        {
            alSourceRewind( mALSource );
        }

        alSourcePlay( mALSource );
    }
}

void Sound::Pause()
{
    if( IsPlaying() )
    {
        alSourcePause( mALSource );
    }
}

void Sound::Stop()
{
    if( !IsStopped() )
    {
        alSourceStop( mALSource );
    }
}

bool Sound::IsPlaying() const
{
    ALint state;
    alGetSourcei( mALSource, AL_SOURCE_STATE, &state );
    return ( AL_PLAYING == state );
}

bool Sound::IsPaused() const
{
    ALint state;
    alGetSourcei( mALSource, AL_SOURCE_STATE, &state );
    return ( AL_PAUSED == state );
}

bool Sound::IsStopped() const
{
    ALint state;
    alGetSourcei( mALSource, AL_SOURCE_STATE, &state );
    return ( AL_STOPPED == state );
}



SoundDevice::SoundDevice()
    : mALDevice( nullptr )
    , mALContext( nullptr )
{
}
SoundDevice::~SoundDevice()
{
}

bool SoundDevice::Initialize()
{
    bool result = false;

    // Open default device
    ALCdevice * device = alcOpenDevice( nullptr );
    if( device )
    {
        ALCcontext * context = alcCreateContext( device, nullptr );
        if( context && alcMakeContextCurrent( context ) )
        {
            mALContext = context;
            mALDevice = device;

            mDevString = alGetString( AL_VENDOR );
            mDevString += ", ";
            mDevString += alGetString( AL_RENDERER );
            mDevString += ", v";
            mDevString += alGetString( AL_VERSION );

            result = true;
        }
    }

    return result;
}

void SoundDevice::Shutdown()
{
    alcMakeContextCurrent( nullptr );
    if( mALContext )
    {
        alcDestroyContext( static_cast<ALCcontext*>( mALContext ) );
    }
    if( mALDevice )
    {
        alcCloseDevice( static_cast<ALCdevice*>( mALDevice ) );
    }
}

const std::string& SoundDevice::GetDeviceString() const
{
    return mDevString;
}
