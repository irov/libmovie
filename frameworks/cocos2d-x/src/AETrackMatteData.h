#ifndef __AETrackMatteDataEx_H__
#define __AETrackMatteDataEx_H__

#include "extensions/ExtensionMacros.h"

NS_CC_EXT_BEGIN;

// track matte resource
class AETrackMatteData : public cocos2d::Ref {
public:
    static AETrackMatteData * create();

    void setMatrix( const cocos2d::Mat4 & m ) { _matrix = m; }
    const cocos2d::Mat4 & getMatrix() const { return _matrix; }

    void setRenderMesh( const aeMovieRenderMesh & m ) { _renderMesh = m; }
    const aeMovieRenderMesh & getRenderMesh() const { return _renderMesh; }

    AERenderData & getRenderData() { return _renderData; }

CC_CONSTRUCTOR_ACCESS:
    AETrackMatteData();
    virtual ~AETrackMatteData();

protected:
    cocos2d::Mat4 _matrix;
    aeMovieRenderMesh _renderMesh;
    AERenderData _renderData;

private:
    CC_DISALLOW_COPY_AND_ASSIGN( AETrackMatteData );
};

NS_CC_EXT_END;

#endif
