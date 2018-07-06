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
};
