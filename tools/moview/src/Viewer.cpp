#include "Viewer.h"

#include "ResourcesManager.h"
#include "Composition.h"
#include "Logger.h"

#include "imgui_impl_glfw_gl3_glad.h"
#include "nfd.h"

#include <algorithm>
#include <thread>
#include <chrono>

#include <stdio.h>
#include <time.h>

//////////////////////////////////////////////////////////////////////////
static const char * g_default_hash = "52ad6f051099762d0a0787b4eb2d07c8a0ee4491";
//////////////////////////////////////////////////////////////////////////
static const float g_ContentScaleMin  = 0.1f;
static const float g_ContentScaleMax  = 10.f;
static const float g_ContentScaleStep = 0.1f;
//////////////////////////////////////////////////////////////////////////
static uint64_t GetCurrentTimeSeconds()
{
    std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();
    std::chrono::system_clock::time_point::duration epoch = tp.time_since_epoch();

    std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(epoch);

    uint64_t ms64 = (uint64_t)ms.count();

    return ms64;
}
//////////////////////////////////////////////////////////////////////////
static bool CompareFloats( const float _a, const float _b )
{
    return std::fabs( _a - _b ) <= FLT_EPSILON;
}
//////////////////////////////////////////////////////////////////////////
static void GLFW_WindowFocusCallback( GLFWwindow * _window, int _focus )
{
    Viewer * viewer = reinterpret_cast<Viewer *>(glfwGetWindowUserPointer( _window ));

    viewer->setFocus( _focus == GLFW_TRUE );
}
//////////////////////////////////////////////////////////////////////////
static void GLFW_WindowIconifyCallback( GLFWwindow * _window, int _iconified )
{
    Viewer * viewer = reinterpret_cast<Viewer *>(glfwGetWindowUserPointer(_window));

    viewer->setMinimized( _iconified == GLFW_TRUE );
}
//////////////////////////////////////////////////////////////////////////
static void GLFW_MouseScrollCallback( GLFWwindow * _window, double _scrollX, double _scrollY )
{
    Viewer * viewer = reinterpret_cast<Viewer *>(glfwGetWindowUserPointer(_window));

    viewer->onScroll( static_cast<float>(_scrollY) );

    ImGui_ImplGlfw_ScrollCallback( _window, _scrollX, _scrollY );
}
//////////////////////////////////////////////////////////////////////////
static void GLFW_KeyCallback( GLFWwindow * _window, int _key, int _scancode, int _action, int _mods )
{
    Viewer * viewer = reinterpret_cast<Viewer *>(glfwGetWindowUserPointer(_window));

    viewer->onKey( _key, _action, _mods );

    ImGui_ImplGlfw_KeyCallback( _window, _key, _scancode, _action, _mods );
}
//////////////////////////////////////////////////////////////////////////
static void GLFW_MouseButtonCallback( GLFWwindow * _window, int _button, int _action, int _mods )
{
    Viewer * viewer = reinterpret_cast<Viewer *>(glfwGetWindowUserPointer(_window));

    viewer->onMouseButton( _button, _action, _mods );

    ImGui_ImplGlfw_MouseButtonCallback( _window, _button, _action, _mods );
}
//////////////////////////////////////////////////////////////////////////
static void GLFW_CursorPosCallback( GLFWwindow * _window, double _posX, double _posY )
{
    Viewer * viewer = reinterpret_cast<Viewer *>(glfwGetWindowUserPointer(_window));

    viewer->setCursorPos( static_cast<float>(_posX), static_cast<float>(_posY) );
}
//////////////////////////////////////////////////////////////////////////
Viewer::Viewer()
    : mWindow( nullptr )
    , mArrowCursor( nullptr )
    , mHandCursor( nullptr )
    , mWindowWidth( 1280 )
    , mWindowHeight( 720 )
    , mShowNormal( true )
    , mShowWireframe( false )
    , mShouldExit( false )
    , mManualPlayPos( 0.f )
    , mToLoopPlay( false )
    , mComposition( nullptr )
    , mLastCompositionIdx( 0 )
    , mWindowFocus( true )
    , mWindowMinimized( false )
    , mSpaceKeyDown( false )
    , mLMBDown( false )
{
    mLicenseHash = g_default_hash;
    mSessionFileName = "session.txt";

    mContentOffset[0] = 0.f;
    mContentOffset[1] = 0.f;

    mLastMousePos[0] = 0.f;
    mLastMousePos[1] = 0.f;

    mBackgroundColor[0] = 0.412f;
    mBackgroundColor[1] = 0.796f;
    mBackgroundColor[2] = 1.f;
};
//////////////////////////////////////////////////////////////////////////
Viewer::~Viewer()
{
}
//////////////////////////////////////////////////////////////////////////
bool Viewer::Initialize( int argc, char** argv )
{
    this->LoadSession();

    if( argc == 5 )
    {
        mMovieFilePath = argv[1];
        mCompositionName = argv[2];
        mToLoopPlay = (strcmp( argv[3], "1" ) == 0);
        mLicenseHash = argv[4];
    }

    if( glfwInit() == 0 )
    {
        return false;
    }

    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
    glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
    glfwWindowHint( GLFW_RESIZABLE, GL_FALSE );

    mWindow = glfwCreateWindow( static_cast<int>(mWindowWidth),
        static_cast<int>(mWindowHeight),
        "libMOVIEW viewer",
        nullptr,
        nullptr );

    if( mWindow == nullptr )
    {
        glfwTerminate();

        return false;
    }

    glfwSetWindowUserPointer( mWindow, this );


    glfwMakeContextCurrent( mWindow );
    gladLoadGLLoader( reinterpret_cast<GLADloadproc>(glfwGetProcAddress) );
    glfwSwapInterval( 1 ); // enable v-sync

    glfwSetScrollCallback( mWindow, GLFW_MouseScrollCallback );
    glfwSetCharCallback( mWindow, ImGui_ImplGlfw_CharCallback );
    glfwSetKeyCallback( mWindow, GLFW_KeyCallback );
    glfwSetMouseButtonCallback( mWindow, GLFW_MouseButtonCallback );
    glfwSetCursorPosCallback( mWindow, GLFW_CursorPosCallback );
    glfwSetWindowFocusCallback( mWindow, GLFW_WindowFocusCallback );
    glfwSetWindowIconifyCallback( mWindow, GLFW_WindowIconifyCallback );

    mArrowCursor = glfwCreateStandardCursor( GLFW_ARROW_CURSOR );
    mHandCursor = glfwCreateStandardCursor( GLFW_HAND_CURSOR );

    glfwSetInputMode( mWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL );
    glfwSetCursor( mWindow, mArrowCursor );

    // Setup Dear ImGui binding
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().IniFilename = nullptr; // disable "imgui.ini"
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange; // tell ImGui to not interfere with our cursors
    ImGui_ImplGlfwGL3_Init( mWindow, false );
    ImGui::StyleColorsClassic();

    ResourcesManager::Instance().Initialize();

    if( !mMovieFilePath.empty() && !mLicenseHash.empty() )
    {
        this->ReloadMovie();
    }

    glViewport( 0, 0, static_cast<GLint>(mWindowWidth), static_cast<GLint>(mWindowHeight) );

    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    return true;
}
//////////////////////////////////////////////////////////////////////////
void Viewer::Finalize()
{
    // save session on exit
    if( !mMovieFilePath.empty() && !mLicenseHash.empty() )
    {
        this->SaveSession();
    }

    this->ShutdownMovie();

    ImGui_ImplGlfwGL3_Shutdown();
    ImGui::DestroyContext();

    glfwSetCursor( mWindow, nullptr );
    glfwDestroyCursor( mArrowCursor );
    glfwDestroyCursor( mHandCursor );

    glfwDestroyWindow( mWindow );
    glfwTerminate();
}
//////////////////////////////////////////////////////////////////////////
void Viewer::Loop()
{
    uint64_t timeLast = GetCurrentTimeSeconds();

    while( !glfwWindowShouldClose( mWindow ) && !mShouldExit )
    {
        glfwPollEvents();

        if( !mWindowMinimized && mWindowFocus )
        {
            glClearColor( mBackgroundColor[0], mBackgroundColor[1], mBackgroundColor[2], 1.f );
            glClear( GL_COLOR_BUFFER_BIT );

            uint64_t timeNow = GetCurrentTimeSeconds();
            float dt = (float)(timeNow - timeLast) / 1000.f;
            timeLast = timeNow;

            ImGui_ImplGlfwGL3_NewFrame();

            if( mComposition )
            {
                if( mComposition->IsPlaying() )
                {
                    mComposition->Update( dt < 0.1f ? dt : 0.1f );
                }

                if( mShowNormal || mShowWireframe )
                {
                    Composition::DrawMode drawMode;
                    if( mShowNormal && mShowWireframe )
                    {
                        drawMode = Composition::DrawMode::SolidWithWireOverlay;
                    }
                    else if( mShowWireframe )
                    {
                        drawMode = Composition::DrawMode::Wireframe;
                    }
                    else
                    {
                        drawMode = Composition::DrawMode::Solid;
                    }

                    mComposition->Draw( drawMode );
                }
            }

            this->DoUI();

            ImGui::Render();
            ImGui_ImplGlfwGL3_RenderDrawData( ImGui::GetDrawData() );

            glfwSwapBuffers( mWindow );

            std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
        }
        else
        {
            std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
        }
    }
}
//////////////////////////////////////////////////////////////////////////
void Viewer::SaveSession()
{
    FILE* f = fopen( mSessionFileName.c_str(), "wt" );
    if( f ) {
        fprintf( f, "%s\n", mMovieFilePath.c_str() );
        fprintf( f, "%s\n", mCompositionName.c_str() );
        fprintf( f, "%s\n", mToLoopPlay ? "yes" : "no" );
        fprintf( f, "%s\n", mLicenseHash.c_str() );
        fprintf( f, "%f/%f/%f\n", mBackgroundColor[0], mBackgroundColor[1], mBackgroundColor[2] );
        fclose( f );
    }
}
//////////////////////////////////////////////////////////////////////////
void Viewer::LoadSession()
{
    FILE* f = fopen( mSessionFileName.c_str(), "rt" );
    if( f ) {
        char line[1025] = { 0 };

        if( fgets( line, 1024, f ) ) {
            mMovieFilePath.assign( line, strlen( line ) - 1 );
        }
        if( fgets( line, 1024, f ) ) {
            mCompositionName.assign( line, strlen( line ) - 1 );
        }
        if( fgets( line, 1024, f ) ) {
            mToLoopPlay = (line[0] == 'y');
        }
        if( fgets( line, 1024, f ) ) {
            mLicenseHash.assign( line, strlen( line ) - 1 );
        }
        if( fgets( line, 1024, f ) ) {
            float temp[3] = { 0.f };
            if( 3 == sscanf( line, "%f/%f/%f", &temp[0], &temp[1], &temp[2] ) ) {
                mBackgroundColor[0] = temp[0];
                mBackgroundColor[1] = temp[1];
                mBackgroundColor[2] = temp[2];
            }
        }

        fclose( f );
    }
}
//////////////////////////////////////////////////////////////////////////
void Viewer::ShutdownMovie()
{
    if( mComposition )
    {
        mMovie.CloseComposition( mComposition );
        mComposition = nullptr;
    }

    mMovie.Close();
    ResourcesManager::Instance().Shutdown();
}
//////////////////////////////////////////////////////////////////////////
bool Viewer::ReloadMovie()
{
    bool result = false;

    this->ShutdownMovie();

    ResourcesManager::Instance().Initialize();

    if( mMovie.LoadFromFile( mMovieFilePath, mLicenseHash ) )
    {
        mComposition = mCompositionName.empty() ? mMovie.OpenDefaultComposition() : mMovie.OpenComposition( mCompositionName );

        if( mComposition )
        {
            OnNewCompositionOpened();

            this->SaveSession();
            mManualPlayPos = 0.f;

            mLastCompositionIdx = mMovie.FindMainCompositionIdx( mComposition );

            result = true;
        }
        else
        {
            ViewerLogger << "Failed to open the default composition" << std::endl;
        }
    }
    else
    {
        ViewerLogger << "Failed to load the movie" << std::endl;
    }

    return result;
}
//////////////////////////////////////////////////////////////////////////
void Viewer::DoUI()
{
    const ImGuiWindowFlags kPanelFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;// | ImGuiWindowFlags_NoCollapse;

    float rightPanelWidth = 200.f;
    float leftPanelWidth = 300.f;
    float panelHeight = 200.f;
    float nextY = 0.f;

    bool openNewMovie = false;

    ImGui::SetNextWindowPos( ImVec2( 0.f, nextY ) );
    ImGui::SetNextWindowSize( ImVec2( leftPanelWidth, 0.f ) );
    ImGui::Begin( "Movie:", nullptr, kPanelFlags );
    {
        char moviePath[1024] = { 0 };
        char licenseHash[1024] = { 0 };
        char compositionName[1024] = { 0 };

        if( !mMovieFilePath.empty() )
        {
            memcpy( moviePath, mMovieFilePath.c_str(), mMovieFilePath.length() );
        }
        if( !mLicenseHash.empty() )
        {
            memcpy( licenseHash, mLicenseHash.c_str(), mLicenseHash.length() );
        }

        ImGui::Text( "Movie file path:" );
        ImGui::PushItemWidth( leftPanelWidth - 50.f );
        {
            if( ImGui::InputText( "##FilePath", moviePath, sizeof( moviePath ) - 1 ) )
            {
                mMovieFilePath = moviePath;
            }
        }
        ImGui::PopItemWidth();
        ImGui::SameLine();
        if( ImGui::Button( "..." ) )
        {
            nfdchar_t* outPath = nullptr;
            if( NFD_OKAY == NFD_OpenDialog( "aem", nullptr, &outPath ) )
            {
                mMovieFilePath = outPath;
                free( outPath );
                openNewMovie = true;
            }
        }

        ImGui::Text( "Licence hash:" );
        ImGui::PushItemWidth( leftPanelWidth - 50.f );
        {
            if( ImGui::InputText( "##LicenseHash", licenseHash, sizeof( licenseHash ) - 1 ) )
            {
                mLicenseHash = licenseHash;

                if( mLicenseHash.empty() == true )
                {
                    mLicenseHash = g_default_hash;
                }
            }
        }
        ImGui::PopItemWidth();
    }
    nextY += ImGui::GetWindowHeight();
    ImGui::End();

    if( openNewMovie && !mMovieFilePath.empty() && !mLicenseHash.empty() )
    {
        mCompositionName.clear();
        this->ReloadMovie();
    }

    // If we have more then 1 main composition - let's allow user to choose one to play
    uint32_t numMainCompositions = mMovie.GetMainCompositionsCount();

    if( numMainCompositions > 0 )
    {
        ImGui::SetNextWindowPos( ImVec2( 0.f, nextY ) );
        ImGui::SetNextWindowSize( ImVec2( leftPanelWidth, panelHeight ) );
        ImGui::Begin( "Main compositions:", nullptr, kPanelFlags );
        {
            int option = static_cast<int>(mLastCompositionIdx);

            for( uint32_t i = 0; i < numMainCompositions; ++i )
            {
                std::string guiID = std::to_string( i );

                std::string fullLabel = mMovie.GetMainCompositionNameByIdx( i ) + "##" + guiID;
                ImGui::RadioButton( fullLabel.c_str(), &option, static_cast<int>(i) );
            }

            if( static_cast<uint32_t>(option) != mLastCompositionIdx )
            {
                mLastCompositionIdx = static_cast<uint32_t>(option);
                mMovie.CloseComposition( mComposition );
                mComposition = mMovie.OpenMainCompositionByIdx( mLastCompositionIdx );
                OnNewCompositionOpened();
                mManualPlayPos = 0.f;
            }
        }
        ImGui::End();
    }

    nextY = 0.f;
    ImGui::SetNextWindowPos( ImVec2( static_cast<float>(mWindowWidth) - rightPanelWidth, nextY ) );
    ImGui::SetNextWindowSize( ImVec2( rightPanelWidth, 0.f ) );
    ImGui::Begin( "Viewer:", nullptr, kPanelFlags );
    {
        const float wndWidth = static_cast<float>(mWindowWidth);
        const float wndHeight = static_cast<float>(mWindowHeight);

        ImGui::Text( "%.1f FPS (%.3f ms)", ImGui::GetIO().Framerate, 1000.f / ImGui::GetIO().Framerate );
        ImGui::Checkbox( "Draw normal", &mShowNormal );
        ImGui::Checkbox( "Draw wireframe", &mShowWireframe );
        {
            float contentScale = (mComposition == nullptr) ? 1.0f : mComposition->GetContentScale();
            const float oldScale = contentScale;
            ImGui::Text( "Content scale:" );
            ImGui::PushItemWidth( ImGui::GetWindowWidth() * 0.92f );
            ImGui::SliderFloat( "##ContentScale", &contentScale, g_ContentScaleMin, g_ContentScaleMax );
            ImGui::PopItemWidth();
            if( !CompareFloats( oldScale, contentScale ) && mComposition )
            {
                ScaleAroundPoint( contentScale, wndWidth * 0.5f, wndHeight * 0.5f );
            }
        }

        if( ImGui::Button( "Reset scale" ) )
        {
            ScaleAroundPoint( 1.f, wndWidth * 0.5f, wndHeight * 0.5f );
        }
        ImGui::SameLine();
        if( ImGui::Button( "Reset offset" ) )
        {
            CenterCompositionOnScreen();
        }

        ImGui::Text( "Background color:" );
        ImGui::ColorEdit3( "##BkgColor", mBackgroundColor );
    }
    nextY += ImGui::GetWindowHeight();
    ImGui::End();

    if( mComposition )
    {
        ImGui::SetNextWindowPos( ImVec2( static_cast<float>(mWindowWidth) - rightPanelWidth, nextY ) );
        ImGui::SetNextWindowSize( ImVec2( rightPanelWidth, panelHeight ) );
        ImGui::Begin( "Movie info:", nullptr, kPanelFlags );
        {
            float duration = mComposition->GetDuration();
            float playTime = mComposition->GetCurrentPlayTime();
            float playPos = playTime / duration;

            ImGui::Text( "Composition: %s", mComposition->GetName().c_str() );
            ImGui::Text( "Duration: %0.2fs", duration );
            ImGui::Text( "Play time: %0.2fs", playTime );

            ImGui::ProgressBar( playPos );

            mManualPlayPos = mComposition->GetCurrentPlayTime();
            ImGui::Text( "Manual position:" );
            ImGui::PushItemWidth( ImGui::GetWindowWidth() * 0.92f );
            ImGui::SliderFloat( "##ManualPos", &mManualPlayPos, 0.f, mComposition->GetDuration() );
            ImGui::PopItemWidth();
            mComposition->SetCurrentPlayTime( mManualPlayPos );

            if( ImGui::Button( "Play" ) )
            {
                if( mComposition->IsEndedPlay() )
                {
                    mManualPlayPos = 0.f;
                }
                mComposition->Play( mManualPlayPos );
            }
            ImGui::SameLine();
            if( ImGui::Button( "Pause" ) )
            {
                mComposition->Pause();
                mManualPlayPos = mComposition->GetCurrentPlayTime();
            }
            ImGui::SameLine();
            if( ImGui::Button( "Stop" ) )
            {
                mComposition->Stop();
                mManualPlayPos = 0.f;
            }
            ImGui::SameLine();
            if( ImGui::Button( "Rewind" ) )
            {
                mComposition->SetCurrentPlayTime( 0.f );
                mManualPlayPos = 0.f;
            }

            bool loopComposition = mComposition->IsLooped();
            ImGui::Checkbox( "Loop composition", &loopComposition );
            if( loopComposition != mComposition->IsLooped() )
            {
                mComposition->SetLoop( loopComposition );
            }
        }
        nextY += ImGui::GetWindowHeight();
        ImGui::End();

        uint32_t numSubCompositions = mComposition->GetNumSubCompositions();
        if( numSubCompositions != 0 )
        {
            ImGui::SetNextWindowPos( ImVec2( static_cast<float>(mWindowWidth) - rightPanelWidth, nextY ) );
            ImGui::SetNextWindowSize( ImVec2( rightPanelWidth, panelHeight ) );
            ImGui::Begin( "Sub compositions:", nullptr, kPanelFlags );
            {
                for( uint32_t i = 0; i < numSubCompositions; ++i )
                {
                    std::string guiID = std::to_string( i );
                    std::string fullName = guiID + ") " + mComposition->GetSubCompositionName( i ) + ":";
                    ImGui::Text( "%s", fullName.c_str() );

                    std::string btnPlayNameAndId = std::string( "Play##" ) + guiID;
                    std::string btnPauseNameAndId = std::string( "Pause##" ) + guiID;
                    std::string btnStopNameAndId = std::string( "Stop##" ) + guiID;
                    std::string btnRewindNameAndId = std::string( "Rewind##" ) + guiID;

                    if( ImGui::Button( btnPlayNameAndId.c_str() ) )
                    {
                        mComposition->PlaySubComposition( i );
                    }
                    ImGui::SameLine();
                    if( ImGui::Button( btnPauseNameAndId.c_str() ) )
                    {
                        mComposition->PauseSubComposition( i );
                    }
                    ImGui::SameLine();
                    if( ImGui::Button( btnStopNameAndId.c_str() ) )
                    {
                        mComposition->StopSubComposition( i );
                    }
                    ImGui::SameLine();
                    if( ImGui::Button( btnRewindNameAndId.c_str() ) )
                    {
                        mComposition->SetTimeSubComposition( i, 0.f );
                    }

                    std::string chkLoopNameAndId = std::string( "Loop play##" ) + guiID;

                    bool loopPlay = mComposition->IsLoopedSubComposition( i );
                    ImGui::Checkbox( chkLoopNameAndId.c_str(), &loopPlay );
                    if( loopPlay != mComposition->IsLoopedSubComposition( i ) )
                    {
                        mComposition->SetLoopSubComposition( i, loopPlay );
                    }

                    ImGui::NewLine();
                }
            }
            ImGui::End();
        }
    }
}
//////////////////////////////////////////////////////////////////////////
void Viewer::CalcScaleToFitComposition()
{
    if( mComposition )
    {
        const float wndWidth = static_cast<float>(mWindowWidth);
        const float wndHeight = static_cast<float>(mWindowHeight);

        // Now we need to scale and position our content so that it's centered and fits the screen
        const float contentWidth = mComposition->GetWidth();
        const float contentHeight = mComposition->GetHeight();

        float scale = 1.0f;
        if( contentWidth > wndWidth || contentHeight > wndHeight )
        {
            // Content's size is bigger then the window, scaling needed
            const float dW = contentWidth - wndWidth;
            const float dH = contentHeight - wndHeight;

            if( dW > dH )
            {
                scale = wndWidth / contentWidth;
            }
            else
            {
                scale = wndHeight / contentHeight;
            }
        }

        mComposition->SetContentScale( scale );
    }
}
//////////////////////////////////////////////////////////////////////////
void Viewer::CenterCompositionOnScreen()
{
    if( mComposition )
    {
        const float wndWidth = static_cast<float>(mWindowWidth);
        const float wndHeight = static_cast<float>(mWindowHeight);

        const float scale = mComposition->GetContentScale();
        const float contentWidth = mComposition->GetWidth() * scale;
        const float contentHeight = mComposition->GetHeight() * scale;

        const float dx = (wndWidth - contentWidth) * 0.5f;
        const float dy = (wndHeight - contentHeight) * 0.5f;

        mContentOffset[0] = 0.f;
        mContentOffset[1] = 0.f;
        OffsetScene( dx, dy );
    }
}
//////////////////////////////////////////////////////////////////////////
void Viewer::OffsetScene( float _dx, float _dy )
{
    const float scale = mComposition->GetContentScale();

    mContentOffset[0] += _dx / scale;
    mContentOffset[1] += _dy / scale;

    mComposition->SetContentOffset( mContentOffset[0], mContentOffset[1] );
}
void Viewer::ScaleAroundPoint( float _scale, float _x, float _y )
{
    if( mComposition )
    {
        OffsetScene( -_x, -_y );
        mComposition->SetContentScale( _scale );
        OffsetScene( _x, _y );
    }
}
//////////////////////////////////////////////////////////////////////////
void Viewer::OnNewCompositionOpened()
{
    if( mComposition )
    {
        mComposition->SetViewportSize( static_cast<float>( mWindowWidth ), static_cast<float>( mWindowHeight ) );

        mCompositionName = mComposition->GetName();
        ViewerLogger << "Composition \"" << mCompositionName << "\" loaded successfully" << std::endl;
        ViewerLogger << " Duration: " << mComposition->GetDuration() << " seconds" << std::endl;

        mComposition->SetLoop( mToLoopPlay );
        mComposition->Play();

        // Now we need to scale and position our content so that it's centered and fits the screen
        CalcScaleToFitComposition();
        CenterCompositionOnScreen();
    }
}
//////////////////////////////////////////////////////////////////////////
void Viewer::setFocus( bool _value )
{
    mWindowFocus = _value;
}
//////////////////////////////////////////////////////////////////////////
void Viewer::setMinimized( bool _minimized )
{
    mWindowMinimized = _minimized;
}
//////////////////////////////////////////////////////////////////////////
void Viewer::onScroll( float _scrollY )
{
    if( mComposition )
    {
        float contentScale = mComposition->GetContentScale();
        contentScale += _scrollY * g_ContentScaleStep;
        contentScale = std::max( g_ContentScaleMin, contentScale );
        contentScale = std::min( g_ContentScaleMax, contentScale );

        ScaleAroundPoint( contentScale, mLastMousePos[0], mLastMousePos[1] );
    }
}
//////////////////////////////////////////////////////////////////////////
void Viewer::onKey( int _key, int _action, int _mods )
{
    AE_UNUSED( _mods );

    if( _key == GLFW_KEY_SPACE )
    {
        if( _action == GLFW_PRESS )
        {
            mSpaceKeyDown = true;

            glfwSetCursor( mWindow, mHandCursor );
        }
        else if( _action == GLFW_RELEASE )
        {
            mSpaceKeyDown = false;

            glfwSetCursor( mWindow, mArrowCursor );
        }
    }
    else if( _key == GLFW_KEY_F5 )
    {
        if( _action == GLFW_PRESS )
        {
            ReloadMovie();
        }
    }
}
//////////////////////////////////////////////////////////////////////////
void Viewer::onMouseButton( int _button, int _action, int _mods )
{
    AE_UNUSED( _mods );

    if( _button == GLFW_MOUSE_BUTTON_LEFT )
    {
        if( _action == GLFW_PRESS )
        {
            mLMBDown = true;
        }
        else if( _action == GLFW_RELEASE )
        {
            mLMBDown = false;
        }
    }
}
//////////////////////////////////////////////////////////////////////////
void Viewer::setCursorPos( float _posX, float _posY )
{
    if( mSpaceKeyDown && mLMBDown && mComposition )
    {
        const float dx =_posX - mLastMousePos[0];
        const float dy =_posY - mLastMousePos[1];

        OffsetScene( dx, dy );
    }

    mLastMousePos[0] = _posX;
    mLastMousePos[1] = _posY;
}
//////////////////////////////////////////////////////////////////////////
