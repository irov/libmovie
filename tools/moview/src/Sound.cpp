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

#include "Sound.h"
#include "Platform.h"

#include "stb_vorbis.h"

// Stupid Apple monkeys decided to "be different"  >:(
#ifdef PLATFORM_MACOS
#   include <OpenAL/al.h>
#   include <OpenAL/alc.h>
#else
#   define AL_LIBTYPE_STATIC
#   include <AL/al.h>
#   include <AL/alc.h>
#endif

#include <algorithm>  // for std::find

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
        alSourcei( mALSource, AL_BUFFER, mALBuffer );

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
        alSourcei( mALSource, AL_BUFFER, 0 );
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
    , mGlobalVolume( 1.f )
    , mMuted( false )
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

            alListenerf( AL_GAIN, 1.0f );

            result = true;
        }
    }

    return result;
}

void SoundDevice::Shutdown()
{
    DeleteAllSounds();

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

Sound* SoundDevice::CreateSound()
{
    mSounds.push_back( new Sound() );
    return mSounds.back();
}

void SoundDevice::DeleteSound( Sound* _sound )
{
    if( _sound )
    {
        auto it = std::find( mSounds.begin(), mSounds.end(), _sound );
        if( it != mSounds.end() )
        {
            mSounds.erase( it );
        }

        _sound->Destroy();
        delete _sound;
    }
}

void SoundDevice::StopAllSounds()
{
    for( Sound* sound : mSounds )
    {
        sound->Stop();
    }
}

void SoundDevice::DeleteAllSounds()
{
    for( Sound* sound : mSounds )
    {
        sound->Destroy();
        delete sound;
    }

    mSounds.clear();
}

void SoundDevice::SetGlobalVolume( float _volume )
{
    if( mGlobalVolume != _volume )
    {
        mGlobalVolume = _volume;

        if( !mMuted )
        {
            alListenerf( AL_GAIN, _volume );
        }
    }
}

float SoundDevice::GetGlobalVolume() const
{
    return mGlobalVolume;
}

void SoundDevice::SetMuted( bool _muted )
{
    if( mMuted != _muted )
    {
        mMuted = _muted;
        alListenerf( AL_GAIN, _muted ? 0.0f : mGlobalVolume );
    }
}

bool SoundDevice::IsMuted() const
{
    return mMuted;
}
