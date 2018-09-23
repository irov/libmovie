#pragma once

#include "Singleton.h"

#include <string>
#include <vector>


class Sound;
class SoundDevice;

class Sound
{
public:
    Sound();
    ~Sound();

    bool    LoadFromFile( const std::string & _path );
    void    Destroy();

    void    Play();
    void    Pause();
    void    Stop();

    bool    IsPlaying() const;
    bool    IsPaused() const;
    bool    IsStopped() const;

private:
    int         mNumChannels;
    int         mSampleRate;
    int         mNumSamples;

    uint32_t    mALSource;
    uint32_t    mALBuffer;
};


class SoundDevice
    : public Singleton<SoundDevice>
{
public:
    SoundDevice();
    ~SoundDevice();

public:
    bool Initialize();
    void Shutdown();

    const std::string&  GetDeviceString() const;

    Sound*              CreateSound();
    void                DeleteSound( Sound* _sound );

    void                StopAllSounds();
    void                DeleteAllSounds();

    void                SetGlobalVolume( float _volume );
    float               GetGlobalVolume() const;
    void                SetMuted( bool _muted );
    bool                IsMuted() const;

private:
    void*       mALDevice;
    void*       mALContext;
    std::string mDevString;

    float       mGlobalVolume;
    bool        mMuted;

    std::vector<Sound*> mSounds;
};
