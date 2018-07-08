#include "Viewer.h"

#include "ResourcesManager.h"
#include "Composition.h"
#include "Logger.h"

#include "imgui_impl_glfw_gl3_glad.h"
#include "nfd.h"

#include <algorithm>
#include <chrono>

#include <stdio.h>
#include <time.h>

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
Viewer::Viewer()
    : mWindowWidth( 1280 )
    , mWindowHeight( 720 )
    , mShowNormal( true )
    , mShowWireframe( false )
    , mShouldExit( false )
    , mManualPlayPos( 0.f )
    , mToLoopPlay( false )
    , mComposition( nullptr )
    , mLastCompositionIdx( 0 )
{
    mLicenseHash = "52ad6f051099762d0a0787b4eb2d07c8a0ee4491";
    mSessionFileName = "session.txt";

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

    if( argc == 5 ) {
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
        "libMovie - GLFW viewer",
        nullptr,
        nullptr );

    if( mWindow == nullptr )
    {
        glfwTerminate();

        return false;
    }

    glfwMakeContextCurrent( mWindow );
    gladLoadGLLoader( reinterpret_cast<GLADloadproc>(glfwGetProcAddress) );
    glfwSwapInterval( 1 ); // enable v-sync

    glfwSetScrollCallback( mWindow, ImGui_ImplGlfw_ScrollCallback );
    glfwSetCharCallback( mWindow, ImGui_ImplGlfw_CharCallback );
    glfwSetKeyCallback( mWindow, ImGui_ImplGlfw_KeyCallback );
    glfwSetMouseButtonCallback( mWindow, ImGui_ImplGlfw_MouseButtonCallback );

    // Setup Dear ImGui binding
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().IniFilename = nullptr; // disable "imgui.ini"
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
                mComposition->Update( dt );
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

    if( mMovie.LoadFromFile( mMovieFilePath, mLicenseHash ) ) {
        mComposition = mCompositionName.empty() ? mMovie.OpenDefaultComposition() : mMovie.OpenComposition( mCompositionName );

        if( mComposition ) {
            mCompositionName = mComposition->GetName();
            ViewerLogger << "Composition \"" << mCompositionName << "\" loaded successfully" << std::endl;
            ViewerLogger << " Duration: " << mComposition->GetDuration() << " seconds" << std::endl;
            mComposition->SetLoop( mToLoopPlay );
            mComposition->Play();

            this->SaveSession();
            mManualPlayPos = 0.f;

            mLastCompositionIdx = mMovie.FindMainCompositionIdx( mComposition );

            result = true;
        }
        else {
            ViewerLogger << "Failed to open the default composition" << std::endl;
        }
    }
    else {
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

        if( !mMovieFilePath.empty() ) {
            memcpy( moviePath, mMovieFilePath.c_str(), mMovieFilePath.length() );
        }
        if( !mLicenseHash.empty() ) {
            memcpy( licenseHash, mLicenseHash.c_str(), mLicenseHash.length() );
        }

        ImGui::Text( "Movie file path:" );
        ImGui::PushItemWidth( leftPanelWidth - 50.f );
        {
            if( ImGui::InputText( "##FilePath", moviePath, sizeof( moviePath ) - 1 ) ) {
                mMovieFilePath = moviePath;
            }
        }
        ImGui::PopItemWidth();
        ImGui::SameLine();
        if( ImGui::Button( "..." ) ) {
            nfdchar_t* outPath = nullptr;
            if( NFD_OKAY == NFD_OpenDialog( "aem", nullptr, &outPath ) ) {
                mMovieFilePath = outPath;
                free( outPath );
                openNewMovie = true;
            }
        }

        ImGui::Text( "Licence hash:" );
        ImGui::PushItemWidth( leftPanelWidth - 50.f );
        {
            if( ImGui::InputText( "##LicenseHash", licenseHash, sizeof( licenseHash ) - 1 ) ) {
                mLicenseHash = licenseHash;
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
                mComposition->SetLoop( mToLoopPlay );
                mComposition->Play();
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
        ImGui::Text( "%.1f FPS (%.3f ms)", ImGui::GetIO().Framerate, 1000.f / ImGui::GetIO().Framerate );
        ImGui::Checkbox( "Draw normal", &mShowNormal );
        ImGui::Checkbox( "Draw wireframe", &mShowWireframe );
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
