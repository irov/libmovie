#include "HelloWorldScene.h"
#include "SimpleAudioEngine.h"

#include "AEMovie/AEMovieCache.h"
#include "AEMovie/AEMovie.h"
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

	auto movie = extension::AEMovie::create("AEM/", "Knight");
	movie->setPosition(Point(designSize/2));
	movie->setAnchorPoint(Point::ANCHOR_MIDDLE);
	
	movie->setComposition("Knight");
	movie->play();
	
	Size targetSize = designSize;
	Size movieSize = movie->getContentSize();
	float f = targetSize.height / movieSize.height;
	movie->setScaleX(f);
	//TODO: move to AEMovie
	movie->setScaleY(-f);	// invert Y axis for AEM
	movie->setScaleZ(f);
	
	addChild(movie);
	
    return true;
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
