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
    void OnNewCompositionOpened();

public:
    void setFocus( bool _focus );

protected:
    GLFWwindow * mWindow;

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
    float mManualPlayPos;

};