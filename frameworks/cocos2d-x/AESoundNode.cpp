#include "cocos2d.h"
#include "audio/include/AudioEngine.h"
#include "AEMovie.h"

NS_CC_EXT_BEGIN;

USING_NS_CC;
using namespace cocos2d::experimental;

AESoundNode * AESoundNode::create( AESound *sound ) {
    AESoundNode * ret = new (std::nothrow) AESoundNode();

    if( ret ) {
        ret->autorelease();
        ret->_sound = sound;
        CCLOG( "  sound ptr: %i", ret->getSound() );
        CCLOG( " Sound: '%s'", ret->getSound()->getPath() );

        return ret;
    }

    CC_SAFE_DELETE( ret );

    return nullptr;
}

AESoundNode::AESoundNode()
    : _sound( nullptr )
    , _audioId( AudioEngine::INVALID_AUDIO_ID )
{
    CCLOG( "AESoundNode::AESoundNode()" );
}

AESoundNode::~AESoundNode() {
    CCLOG( "AESoundNode::~AESoundNode()" );
    AudioEngine::stop( _audioId );
}

void AESoundNode::setTime( float t ) {
    if( _audioId == AudioEngine::INVALID_AUDIO_ID )
        return;

    AudioEngine::AudioState state = AudioEngine::getState( _audioId );

    Scheduler* const scheduler( Director::getInstance()->getScheduler() );

    if( state >= AudioEngine::AudioState::PLAYING ) {
        scheduler->unschedule( SEL_SCHEDULE( &AESoundNode::updateTimeCallback ), this );
        AudioEngine::setCurrentTime( _audioId, t );
    }
    else {
        _updateTime = t;
        scheduler->schedule( SEL_SCHEDULE( &AESoundNode::updateTimeCallback ), this, 0.f, CC_REPEAT_FOREVER, 0.f, false );
    }
}

void AESoundNode::updateTimeCallback( float dt ) {
    Scheduler* const scheduler( Director::getInstance()->getScheduler() );

    if( _audioId == AudioEngine::INVALID_AUDIO_ID ) {
        scheduler->unschedule( SEL_SCHEDULE( &AESoundNode::updateTimeCallback ), this );
        return;
    }

    AudioEngine::AudioState state = AudioEngine::getState( _audioId );

    if( state >= AudioEngine::AudioState::PLAYING ) {
        AudioEngine::setCurrentTime( _audioId, _updateTime );
        scheduler->unschedule( SEL_SCHEDULE( &AESoundNode::updateTimeCallback ), this );
    }
}

NS_CC_EXT_END;
