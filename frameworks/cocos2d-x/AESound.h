#ifndef __AESoundEx_H__
#define __AESoundEx_H__

#include "extensions/ExtensionMacros.h"

NS_CC_EXT_BEGIN;

// sound resource
class AESound : public cocos2d::Ref {
public:
    static AESound * create( const std::string & path );

    const std::string & getPath() const { return _path; }

CC_CONSTRUCTOR_ACCESS:
    AESound();
    virtual ~AESound();

protected:
    // relative path to file (eg. AEM/example/audio/sound.ogg)
    std::string _path;

private:
    CC_DISALLOW_COPY_AND_ASSIGN( AESound );
};

NS_CC_EXT_END;

#endif
