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

#if defined( _WIN32 ) || defined( _WIN64 )
# define PLATFORM_WINDOWS
#elif defined( __APPLE__ )
# define PLATFORM_MACOS
#elif defined( __linux__ )
# define PLATFORM_LINUX
#else
# error "Unknown platform"
#endif

#include <cstdio>
#include <string>

typedef void* Handle;

namespace Platform
{
    enum class FOMode : size_t
    {
        Read,
        Write,
        ReadWrite
    };

    enum class SeekOrigin : size_t
    {
        Current,
        Begin,
        End
    };

    Handle FOpen( const std::string & _path, const FOMode _mode );
    size_t FRead( void * _buffer, const size_t _numBytes, Handle _handle );
    size_t FWrite( const void * _buffer, const size_t _numBytes, Handle _handle );
    size_t FSeek( const size_t _offset, const SeekOrigin _origin, Handle _handle );
    size_t FTell( Handle _handle );
    void   FClose( Handle _handle );

    bool   CheckIfExists( const std::string & _path );
    bool   CreateDirs( const std::string & _path );

    std::string GetAppDataFolder();
    std::string PathConcat( const std::string & _pathA, const std::string & _pathB );

} // namespace Platform
