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
