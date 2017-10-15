#ifndef __AETrackMatteNodeEx_H__
#define __AETrackMatteNodeEx_H__

#include "extensions/ExtensionMacros.h"

NS_CC_EXT_BEGIN;

//#define AE_TRACKMATTENODE_DEBUG_DRAW

class AETrackMatteNode : public cocos2d::Ref {
public:
    static AETrackMatteNode * createFromTexture( cocos2d::Texture2D * texture );

    //	virtual void draw(cocos2d::Renderer *renderer, const cocos2d::Mat4 &transform, uint32_t flags) override;

CC_CONSTRUCTOR_ACCESS:
    AETrackMatteNode();
    virtual ~AETrackMatteNode();

protected:
    cocos2d::Texture2D * _texture;

    //#ifdef AE_TRACKMATTENODE_DEBUG_DRAW
    //	cocos2d::DrawNode *_debugDrawNode;
    //#endif

private:
    CC_DISALLOW_COPY_AND_ASSIGN( AETrackMatteNode );
};

NS_CC_EXT_END;

#endif
