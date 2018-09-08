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

    std::string mSettingsFileName;

    bool mShouldExit;
    bool mWindowFocus;
    bool mWindowMinimized;
    float mManualPlayPos;

    float mContentOffset[2];

    bool mSpaceKeyDown;
    bool mLMBDown;
    float mLastMousePos[2];

};