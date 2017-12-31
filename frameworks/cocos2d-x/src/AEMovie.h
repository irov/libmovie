#ifndef __AEMovieEx_H__
#define __AEMovieEx_H__

#include "cocos/2d/CCNode.h"
#include "cocos/2d/CCDrawNode.h"
#include "cocos/2d/CCCamera.h"
#include "cocos/renderer/CCRenderer.h"
#include "cocos/renderer/CCGLProgramState.h"
#include "extensions/ExtensionMacros.h"
#include "extensions/ExtensionExport.h"

#include "movie/movie.hpp"

#include "AESound.h"
#include "AESoundNode.h"
#include "AESlotNode.h"
#include "AERenderData.h"
#include "AETrackMatteData.h"
#include "AETrackMatteNode.h"
#include "AEMovieData.h"
#include "AEMovieCache.h"

NS_CC_EXT_BEGIN;

#define AE_MOVIE_DEBUG_DRAW

//
// the movie player
//

class CC_DLL AEMovie : public cocos2d::Node
{
public:
    // movie state report
    enum class EventType
    {
        PLAYING = 0,
        PAUSED,
        STOPPED,
        TIME_CHANGED
    };

    // the only function to create new nodes
    static AEMovie * create( const std::string & path, const std::string & name );

    // select composition inside the movie
    void setComposition( const std::string & name );

    // set playback to the specified time
    void setTime( float t );
    float getTime() const { return _time; }

    float getDuration() const { return _duration; }

    // playback controls
    void play();
    void stop();
    void interrupt( bool skip );

    virtual void pause() override;
    virtual void resume() override;

    // returns node for attachments, if it exists by the given name
    AESlotNode * getSlotNode( const std::string & name );

    // sets _eventCallback
    typedef std::function<void( Ref *, EventType )> TMovieCallback;
    virtual void addEventListener( const AEMovie::TMovieCallback & callback );

    // calls _eventCallback
    virtual void onPlayEvent( int event );

CC_CONSTRUCTOR_ACCESS:
    AEMovie();
    virtual ~AEMovie();

    virtual bool initWithFile( const std::string & path, const std::string & name );
    virtual bool initWithData( const AEMovieData * data );

protected:
    static ae_voidptr_t callbackCameraProvider( const aeMovieCameraProviderCallbackData * _callbackData, ae_voidptr_t _data );
    static ae_voidptr_t callbackNodeProvider( const aeMovieNodeProviderCallbackData * _callbackData, ae_voidptr_t _data );
    static void callbackNodeDeleter( const aeMovieNodeDeleterCallbackData * _callbackData, ae_voidptr_t _data );
    static void callbackNodeUpdate( const aeMovieNodeUpdateCallbackData * _callbackData, ae_voidptr_t _data );
    static ae_voidptr_t callbackCompositionTrackMatteProvider( const aeMovieTrackMatteProviderCallbackData * _callbackData, ae_voidptr_t _data );
    static void callbackCompositionTrackMatteUpdate( const aeMovieTrackMatteUpdateCallbackData * _callbackData, ae_voidptr_t _data );
    static void callbackCompositionEvent( const aeMovieCompositionEventCallbackData * _callbackData, ae_voidptr_t _data );
    static void callbackCompositionState( const aeMovieCompositionStateCallbackData * _callbackData, ae_voidptr_t _data );

protected:
    void addCamera( const std::string & name, cocos2d::Camera * camera );
    cocos2d::Camera * getCamera( const std::string & name );

    void addSoundNode( AESoundNode * node );

    void addSlotNode( const std::string & name, AESlotNode * node );

    void addTrackMatteNode( AETrackMatteNode * node );
    void addTrackMatteData( AETrackMatteData * data );

    // this is scheduled to update every frame after a composition is loaded
    virtual void update( float delta ) override;

    // renders the node
    virtual void draw( cocos2d::Renderer *renderer, const cocos2d::Mat4 &transform, uint32_t flags ) override;

    cocos2d::GLProgramState * _normalGPS;
    cocos2d::GLProgramState * _trackMatteGPS;

#ifdef AE_MOVIE_DEBUG_DRAW
    cocos2d::DrawNode *_debugDrawNode;
#endif

    // the movie resource being played
    const AEMovieData * _movieData;

    // current composition info
    aeMovieComposition * _composition;

    //	bool _isPlayingSubComposition;

    float _time;
    float _duration;

    // this is called in onPlayEvent()
    TMovieCallback _eventCallback;

    // various data arrays
    // TODO: use cocos Map & Vector classes
    TVectorRenderData _renderDatas;

    typedef std::map<std::string, cocos2d::Camera *> TMapCamera;
    TMapCamera _cameras;

    typedef std::vector<AESoundNode *> TVectorSoundNode;
    TVectorSoundNode _soundNodes;

    // FIXME: rework to use cocos2d::Node::_name and getChildByName()
    typedef std::unordered_map<std::string, AESlotNode *> TMapSlotNode;
    TMapSlotNode _slotNodes;

    //	typedef std::unordered_map<std::string, AETrackMatteNode *> TMapTrackMatteNode;
    //	TMapTrackMatteNode _trackMatteNodes;
    typedef std::vector<AETrackMatteNode *> TVectorTrackMatteNode;
    TVectorTrackMatteNode _trackMatteNodes;

    typedef std::vector<AETrackMatteData *> TVectorTrackMatteData;
    TVectorTrackMatteData _trackMatteDatas;

private:
    CC_DISALLOW_COPY_AND_ASSIGN( AEMovie );
};

NS_CC_EXT_END;

#endif


