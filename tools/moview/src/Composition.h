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

#include "glad/glad.h"
#include "movie/movie.hpp"

#include <vector>
#include <string>

struct ResourceImage;

class Composition 
{
public:
    enum class DrawMode : uint32_t 
    {
        Solid,
        Wireframe,
        SolidWithWireOverlay
    };

    enum class BlendMode : uint32_t 
    {
        Normal,
        Add
    };

public:
    Composition();
    ~Composition();

public:
    void SetViewportSize( const float width, const float height );
    void SetContentScale( const float scale );
    float GetContentScale() const;
    void SetContentOffset( const float offX, const float offY );

    float GetWidth() const;
    float GetHeight() const;

    const std::string & GetName() const;
    float GetDuration() const;
    void SetCurrentPlayTimeSpeedFactor( float factor );
    float GetCurrentPlayTimeSpeedFactor() const;
    void SetCurrentPlayTime( float time );
    float GetCurrentPlayTime() const;
    

    bool IsPlaying() const;
    void Play( float startTime = 0.f );
    void Pause();
    bool IsPaused() const;
    void Stop();
    void SetLoop( bool toLoop );
    bool IsLooped() const;
    bool IsEndedPlay() const;

    void Update( float deltaTime );
    void Draw( const DrawMode mode );

    // sub compositions
    uint32_t GetNumSubCompositions() const;
    const char * GetSubCompositionName( uint32_t idx ) const;
    void PlaySubComposition( uint32_t idx );
    void PauseSubComposition( uint32_t idx );
    void StopSubComposition( uint32_t idx );
    void SetTimeSubComposition( uint32_t idx, float time );
    void SetLoopSubComposition( uint32_t idx, bool toLoop );
    bool IsLoopedSubComposition( uint32_t idx ) const;

protected:
    bool Create( const aeMovieData* moviewData, const aeMovieCompositionData* compData );
    void AddSubComposition( const aeMovieSubComposition* subComposition );
    bool CreateDrawingData();
    void DestroyDrawingData();

    void BeginDraw();
    void EndDraw();
    void DrawMesh( const aeMovieRenderMesh* mesh, const ResourceImage* imageRGB, const ResourceImage* imageA, const float* alternativeUV );
    void FlushDraw();

    bool OnProvideNode( const aeMovieNodeProviderCallbackData* _callbackData, void** _nd );
    void OnDeleteNode( const aeMovieNodeDeleterCallbackData* _callbackData );
    void OnUpdateNode( const aeMovieNodeUpdateCallbackData* _callbackData );
    bool OnProvideCamera( const aeMovieCameraProviderCallbackData* _callbackData, void** _cd );
    bool OnProvideTrackMatte( const aeMovieTrackMatteProviderCallbackData * _callbackData, void** _tmd );
    void OnUpdateTrackMatte( const aeMovieTrackMatteUpdateCallbackData* _callbackData );
    void OnDeleteTrackMatte( const aeMovieTrackMatteDeleterCallbackData* _callbackData );
    void OnCompositionEffect( const aeMovieCompositionEventCallbackData* _callbackData );
    void OnCompositionState( const aeMovieCompositionStateCallbackData* _callbackData );

protected:
    std::string mName;
    const aeMovieComposition * mComposition;

    typedef std::vector<const aeMovieSubComposition*> VectorMovieSubComposition;
    VectorMovieSubComposition mSubCompositions;

    // rendering stuff
    GLuint mShader;
    GLuint mWireShader;
    GLint mIsPremultAlphaUniform;
    GLuint mVAO;
    GLuint mVB;
    GLuint mIB;
    GLuint mCurrentTextureRGB;
    GLuint mCurrentTextureA;
    BlendMode mCurrentBlendMode;
    bool mPremultipliedAlpha;
    uint32_t mNumVertices;
    uint32_t mNumIndices;
    void * mVerticesData;
    void * mIndicesData;

    DrawMode mDrawMode;
    float mViewportWidth;
    float mViewportHeight;
    float mContentScale;
    float mContentOffX;
    float mContentOffY;
    float mTimeSpeedFactor;

protected:
    friend class Movie;
};
