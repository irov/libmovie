#pragma once

#include "Singleton.h"

#include <string>


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

private:
    void*       mALDevice;
    void*       mALContext;
    std::string mDevString;
};
