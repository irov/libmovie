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

#include "movie/movie.hpp"

#include <vector>
#include <string>

class Composition;

class Movie
{
public:
    Movie();
    ~Movie();

public:
    bool LoadFromFile( const std::string& fileName, const std::string& licenseHash );
    bool LoadFromMemory( const void* data, size_t dataLength, const std::string& baseFolder, const std::string& licenseHash );
    void Close();

    Composition * OpenComposition( const std::string& name );
    void CloseComposition( Composition* composition );

    uint32_t GetMainCompositionsCount() const;
    std::string GetMainCompositionNameByIdx( uint32_t idx ) const;
    Composition * OpenMainCompositionByIdx( uint32_t idx ) const;

    uint32_t FindMainCompositionIdx( Composition* composition ) const;

    Composition * OpenDefaultComposition();

    const std::string& GetLastErrorDescription() const;

protected:
    void AddCompositionData( const aeMovieCompositionData* compositionData );

    bool OnProvideResource( const aeMovieResource* _resource, void** _rd, void* _ud );
    void OnDeleteResource( uint32_t _type, void* _data, void* _ud );

protected:
    const aeMovieInstance * mMovieInstance;
    aeMovieData * mMovieData;
    std::string mBaseFolder;

    typedef std::vector<const aeMovieCompositionData*> VectorMovieCompositionData;
    VectorMovieCompositionData mCompositionDatas;

    std::string mLastErrorDescription;
};
