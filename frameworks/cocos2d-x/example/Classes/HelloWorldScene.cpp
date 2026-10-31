/******************************************************************************
* libMOVIE Software License v1.0
*
* Copyright (c) 2016-2026, Yuriy Levchenko <irov13@mail.ru>
* All rights reserved.
*
* You are granted a perpetual, non-exclusive, non-sublicensable, and
* non-transferable license to use, install, execute, and perform the libMOVIE
* software and derivative works solely for personal or internal
* use. Without the written permission of Yuriy Levchenko, you may not (a) modify, translate,
* adapt, or develop new applications using the libMOVIE or otherwise
* create derivative works or improvements of the libMOVIE or (b) remove,
* delete, alter, or obscure any trademarks or any copyright, trademark, patent,
* or other intellectual property or proprietary rights notices on or in the
* Software, including any copy thereof. Redistributions in binary or source
* form must include this license and terms.
*
* THIS SOFTWARE IS PROVIDED BY YURIY LEVCHENKO "AS IS" AND ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
* EVENT SHALL YURIY LEVCHENKO BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES, BUSINESS INTERRUPTION,
* OR LOSS OF USE, DATA, OR PROFITS) HOWEVER CAUSED AND ON ANY THEORY OF
* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
* OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************/

#include "HelloWorldScene.h"
#include "SimpleAudioEngine.h"

#include "AEMovieCache.h"
#include "movie_hash.h"

USING_NS_CC;

Scene* HelloWorld::createScene()
{
    return HelloWorld::create();
}

// on "init" you need to initialize your instance
bool HelloWorld::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !Scene::init() )
    {
        return false;
    }
	
	//
	// pre-load shaders
	//
	auto p1 = GLProgram::createWithFilenames("Shaders/AEM/AEM_Normal.vsh", "Shaders/AEM/AEM_Normal.fsh");
	auto p2 = GLProgram::createWithFilenames("Shaders/AEM/AEM_TrackMatte.vsh", "Shaders/AEM/AEM_TrackMatte.fsh");
	auto pc = GLProgramCache::getInstance();
	pc->addGLProgram(p1, "AEM_Normal");
	pc->addGLProgram(p2, "AEM_TrackMatte");
	
	extension::AEMovieCache::getInstance()->initialize(ex_example_license_hash);

	Size designSize = Director::getInstance()->getOpenGLView()->getDesignResolutionSize();

	const bool usePlist = true;
	
	if(usePlist)
	{
		addMovie(extension::AEMovie::createWithPlist("AEM/Knight/Knight.aem", "knight.plist"),
				 Point(designSize.width / 4, designSize.height / 2), "Knight");
	}
	else
	{
		addMovie(extension::AEMovie::create("AEM/Knight/Knight.aem"),
				 Point(designSize.width / 4, designSize.height / 2), "Knight");
	}
	
    return true;
}

void HelloWorld::addMovie(extension::AEMovie* movie, const cocos2d::Point& position, const std::string& composition)
{
	movie->setPosition(position);
	movie->setAnchorPoint(Point::ANCHOR_MIDDLE);
	
	movie->setComposition(composition);
	movie->play();
	
	Size designSize = Director::getInstance()->getOpenGLView()->getDesignResolutionSize();
	Size targetSize = designSize;
	Size movieSize = movie->getContentSize();
	float f = targetSize.height / movieSize.height;
	movie->setScaleX(f);
	//TODO: move to AEMovie
	movie->setScaleY(-f);	// invert Y axis for AEM
	movie->setScaleZ(f);
	
	addChild(movie);
}


void HelloWorld::menuCloseCallback(Ref* pSender)
{
    //Close the cocos2d-x game scene and quit the application
    Director::getInstance()->end();

    #if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    exit(0);
#endif
    
    /*To navigate back to native iOS screen(if present) without quitting the application  ,do not use Director::getInstance()->end() and exit(0) as given above,instead trigger a custom event created in RootViewController.mm as below*/
    
    //EventCustom customEndEvent("game_scene_close_event");
    //_eventDispatcher->dispatchEvent(&customEndEvent);
    
    
}
