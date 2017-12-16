#ifndef __AERenderDataEx_H__
#define __AERenderDataEx_H__

#include "extensions/ExtensionMacros.h"

NS_CC_EXT_BEGIN;

typedef std::vector<cocos2d::V3F_C4B_T2F> TVectorVertex;
typedef std::vector<unsigned short> TVectorIndex;

struct AERenderData
{
    TVectorVertex vertices;
    TVectorIndex indices;

    cocos2d::TrianglesCommand trianglesCommand;
};

typedef std::vector<AERenderData> TVectorRenderData;

NS_CC_EXT_END;

#endif
