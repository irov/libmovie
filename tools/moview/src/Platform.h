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
