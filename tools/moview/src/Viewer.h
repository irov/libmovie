#pragma once

#include "Movie.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include <string>
#include <stdint.h>

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
    void SaveSession();
    void LoadSession();
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

    std::string mMovieFilePath;
    std::string mLicenseHash;
    std::string mCompositionName;
    bool mToLoopPlay;
    Movie mMovie;
    Composition* mComposition;
    uint32_t mLastCompositionIdx;
    float mBackgroundColor[3];

    std::string mSessionFileName;

    bool mShowNormal;
    bool mShowWireframe;
    bool mShouldExit;
    bool mWindowFocus;
    bool mWindowMinimized;
    float mManualPlayPos;

    float mContentOffset[2];

    bool mSpaceKeyDown;
    bool mLMBDown;
    float mLastMousePos[2];

};