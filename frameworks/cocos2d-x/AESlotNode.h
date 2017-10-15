#ifndef __AESlotNodeEx_H__
#define __AESlotNodeEx_H__

#include "extensions/ExtensionMacros.h"

NS_CC_EXT_BEGIN;

#define AE_SLOTNODE_DEBUG_DRAW

// sound node inside composition
// TODO: later create an abstract class for all AEM nodes
class AESlotNode : public cocos2d::Node {
public:
    static AESlotNode * create();

    virtual void draw( cocos2d::Renderer *renderer, const cocos2d::Mat4 &transform, uint32_t flags ) override;

CC_CONSTRUCTOR_ACCESS:
    AESlotNode();
    virtual ~AESlotNode();

    virtual bool init();

protected:
#ifdef AE_SLOTNODE_DEBUG_DRAW
    cocos2d::DrawNode *_debugDrawNode;
#endif

private:
    CC_DISALLOW_COPY_AND_ASSIGN( AESlotNode );
};

NS_CC_EXT_END;

#endif
