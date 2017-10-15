#include "cocos2d.h"
#include "audio/include/AudioEngine.h"
#include "AEMovie.h"

NS_CC_EXT_BEGIN;

USING_NS_CC;

AETrackMatteNode * AETrackMatteNode::createFromTexture( Texture2D * texture ) {
    AETrackMatteNode * ret = new (std::nothrow) AETrackMatteNode();

    if( ret ) {
        ret->autorelease();
        ret->_texture = texture;
        return ret;
    }

    CC_SAFE_DELETE( ret );

    return nullptr;
}

AETrackMatteNode::AETrackMatteNode()
    : _texture( nullptr )
{
    CCLOG( "AETrackMatteNode::AETrackMatteNode()" );

    //#ifdef AE_SLOTNODE_DEBUG_DRAW
    //	_debugDrawNode = DrawNode::create();
    //	addChild(_debugDrawNode);
    //#endif
}

AETrackMatteNode::~AETrackMatteNode() {
    CCLOG( "AETrackMatteNode::~AETrackMatteNode()" );
}

/*
void AETrackMatteNode::draw(Renderer *renderer, const Mat4 &transform, uint32_t flags) {
#ifdef AE_SLOTNODE_DEBUG_DRAW
        _debugDrawNode->clear();
    _debugDrawNode->drawRect(Vec2(0.f, 0.f), Vec2(100.f, 100.f), Color4F::WHITE);
#endif
}
*/

NS_CC_EXT_END;
