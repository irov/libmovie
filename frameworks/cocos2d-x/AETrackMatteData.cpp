#include "audio/include/AudioEngine.h"
#include "AEMovie.h"

NS_CC_EXT_BEGIN;

USING_NS_CC;

AETrackMatteData * AETrackMatteData::create() {
    AETrackMatteData * ret = new (std::nothrow) AETrackMatteData();

    if( ret ) {
        ret->autorelease();
        return ret;
    }

    CC_SAFE_DELETE( ret );

    return nullptr;
}

AETrackMatteData::AETrackMatteData() {
    CCLOG( "AETrackMatteData::AETrackMatteData()" );
}

AETrackMatteData::~AETrackMatteData() {
    CCLOG( "AETrackMatteData::~AETrackMatteData()" );
}

NS_CC_EXT_END;
