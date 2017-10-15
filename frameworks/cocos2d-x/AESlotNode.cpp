#include "cocos2d.h"
#include "audio/include/AudioEngine.h"
#include "AEMovie.h"

NS_CC_EXT_BEGIN;

USING_NS_CC;

AESlotNode * AESlotNode::create() {
    AESlotNode * ret = new (std::nothrow) AESlotNode();

    if( ret && ret->init() ) {
        ret->autorelease();
        return ret;
    }

    CC_SAFE_DELETE( ret );

    return nullptr;
}

AESlotNode::AESlotNode()
{
    CCLOG( "AESlotNode::AESlotNode()" );

#ifdef AE_SLOTNODE_DEBUG_DRAW
    _debugDrawNode = DrawNode::create();
    addChild( _debugDrawNode );
#endif
}

AESlotNode::~AESlotNode() {
    CCLOG( "AESlotNode::~AESlotNode()" );
}

bool AESlotNode::init() {
    CCLOG( "AESlotNode::init()" );
    return true;
}

void AESlotNode::draw( Renderer *renderer, const Mat4 &transform, uint32_t flags ) {
#ifdef AE_SLOTNODE_DEBUG_DRAW
    _debugDrawNode->clear();
    _debugDrawNode->drawRect( Vec2( 0.f, 0.f ), Vec2( 100.f, 100.f ), Color4F::WHITE );
#endif
}
NS_CC_EXT_END;
