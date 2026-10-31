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

#include "Movie.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include <string>
#include <stdint.h>

struct Settings
{
    std::string movieFilePath;
    std::string licenseHash;
    std::string compositionName;
    bool        loopPlay;
    float       backgroundColor[3];
    float       soundVolume;
    bool        soundMuted;

    // Transient settings
    bool        drawNormal;
    bool        drawWireframe;
};

class Viewer
{
private:
    enum class PopupType : size_t
    {
        Error = 0,
        Warning,
        Info,

        NumTypes
    };

public:
    Viewer();
    ~Viewer();

public:
    bool Initialize( int argc, char** argv );
    void Finalize();

public:
    void Loop();

protected:
    void SaveSettings();
    void LoadSettings();
    void ShutdownMovie();
    bool ReloadMovie();
    void DoUI();
    void CalcScaleToFitComposition();
    void CenterCompositionOnScreen();
    void OffsetScene( float _dx, float _dy );
    void ScaleAroundPoint( float _scale, float _x, float _y );
    void OnNewCompositionOpened();
    void ShowPopup( const std::string& _message, PopupType _type );

public:
    void setFocus( bool _focus );
    void setMinimized( bool _minimized );
    void onScroll( float _scrollY );
    void onKey( int _key, int _action, int _mods );
    void onMouseButton( int _button, int _action, int _mods );
    void setCursorPos( float _posX, float _posY );

protected:
    GLFWwindow * mWindow;
    GLFWcursor * mArrowCursor;
    GLFWcursor * mHandCursor;

    uint32_t mWindowWidth;
    uint32_t mWindowHeight;

    Settings mSettings;

    Movie mMovie;
    Composition* mComposition;
    uint32_t mLastCompositionIdx;

    std::string mAppName;
    std::string mSettingsFileName;

    bool mShouldExit;
    bool mWindowFocus;
    bool mWindowMinimized;
    float mManualPlayPos;

    float mContentOffset[2];

    bool mSpaceKeyDown;
    bool mLMBDown;
    float mLastMousePos[2];

    std::string mPopupNessage;
    PopupType mPopupType;
};
