#ifndef __AESoundNodeEx_H__
#define __AESoundNodeEx_H__

#include "extensions/ExtensionMacros.h"

NS_CC_EXT_BEGIN;

// sound node inside composition
// TODO: later create an abstract class for all AEM nodes
class AESoundNode : public cocos2d::Ref {
public:
    static AESoundNode * create( AESound *sound );

    AESound *getSound() const { return _sound; }

    // FIXME: make play/pause/stop etc instead
    void setAudioId( int id ) { _audioId = id; }
    int getAudioId() const { return _audioId; }

    void setTime( float t );

CC_CONSTRUCTOR_ACCESS:
    AESoundNode();
    virtual ~AESoundNode();

protected:
    // FIXME: a hack to overcome cocos audio state/setCurrentTime bullshit
    void updateTimeCallback( float dt );

    // audio resource reference
    AESound * _sound;

    // currently playing audio instance, given by cocos2d::AudioEngine
    int _audioId;

    // target time
    float _updateTime;

private:
    CC_DISALLOW_COPY_AND_ASSIGN( AESoundNode );
};

NS_CC_EXT_END;

#endif
