#include "cocos2d.h"
#include "audio/include/AudioEngine.h"
#include "AEMovie.h"

NS_CC_EXT_BEGIN;

AESound * AESound::create( const std::string & path ) {
    AESound * ret = new (std::nothrow) AESound();

    if( ret ) {
        ret->autorelease();
        ret->_path = path;

        return ret;
    }

    CC_SAFE_DELETE( ret );

    return nullptr;
}

AESound::AESound() {
    CCLOG( "AESound::AESound()" );
}

AESound::~AESound() {
    CCLOG( "AESound::~AESound()" );
}

NS_CC_EXT_END;
