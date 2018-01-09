/******************************************************************************
* libMOVIE Software License v1.0
*
* Copyright (c) 2016-2018, Yuriy Levchenko <irov13@mail.ru>
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

#include "AEMovie.h"
//#include "renderer/CCGLProgramCache.h"

NS_CC_EXT_BEGIN;

USING_NS_CC;
using namespace cocos2d::experimental;

//===============================================

// TODO
ae_voidptr_t AEMovie::callbackCameraProvider( const aeMovieCameraProviderCallbackData * _callbackData, ae_voidptr_t _data )
{
    AE_UNUSED( _callbackData );
    AE_UNUSED( _data );

	//XCODE COMPILE COMMENT: CCLOG("CALL: camera provider");
/*
	AEMovie *movie = static_cast<AEMovie *>(_data);

	Camera *c = movie->getCamera(_callbackData->name);

	if (c != nullptr) {
		//XCODE COMPILE COMMENT: CCLOG("Camera '%s' found.", _callbackData->name);
		return c;
	}

	//XCODE COMPILE COMMENT: CCLOG("Creating new camera '%s'.", _callbackData->name);
*/
/*
	auto camera = Camera::createPerspective(_callbackData->fov, _callbackData->width / _callbackData->height, 1.f, 1000.f);
	camera->retain();
	camera->setPosition3D(Vec3(_callbackData->position));
	camera->lookAt(Vec3(_callbackData->direction));
	camera->setCameraFlag(CameraFlag::USER1);

	movie->addChild(camera);	// add to scene graph
	movie->addCamera(_callbackData->name, camera);	// add to map

	return camera;
*/
	return nullptr;
}

ae_voidptr_t AEMovie::callbackNodeProvider( const aeMovieNodeProviderCallbackData * _callbackData, ae_voidptr_t _data ) {
	//XCODE COMPILE COMMENT: CCLOG("CALL: node provider");

    ae_bool_t is_track_matte = ae_is_movie_layer_data_track_mate( _callbackData->layer );

	if( is_track_matte == AE_TRUE )
		return nullptr;

	AEMovie *movie = static_cast<AEMovie *>(_data);

    aeMovieLayerTypeEnum layerType = ae_get_movie_layer_data_type( _callbackData->layer );
    const char * layerName = ae_get_movie_layer_data_name( _callbackData->layer );
	
	//XCODE COMPILE COMMENT: CCLOG("Layer type: %i", layerType);
	//XCODE COMPILE COMMENT: CCLOG("Layer name: '%s'", layerName);

	if( _callbackData->track_matte_layer == nullptr ) {
		//XCODE COMPILE COMMENT: CCLOG(" No track matte.");

		switch( layerType ) {
            case AE_MOVIE_LAYER_TYPE_MOVIE:
            case AE_MOVIE_LAYER_TYPE_TEXT:
            case AE_MOVIE_LAYER_TYPE_EVENT:
            case AE_MOVIE_LAYER_TYPE_SOCKET:
            case AE_MOVIE_LAYER_TYPE_SHAPE:
            case AE_MOVIE_LAYER_TYPE_NULL:
            case AE_MOVIE_LAYER_TYPE_SOLID:
            case AE_MOVIE_LAYER_TYPE_SEQUENCE:
            case AE_MOVIE_LAYER_TYPE_PARTICLE:
            case AE_MOVIE_LAYER_TYPE_IMAGE:
            case AE_MOVIE_LAYER_TYPE_SUB_MOVIE:
                break;
			case AE_MOVIE_LAYER_TYPE_SLOT:
			{
				auto slotNode = AESlotNode::create();
				slotNode->retain();
				//slotNode->setName(layerName);
				slotNode->setAdditionalTransform(Mat4(_callbackData->matrix));

				movie->addSlotNode(layerName, slotNode);

				//XCODE COMPILE COMMENT: CCLOG(" Slot:");
				//XCODE COMPILE COMMENT: CCLOG("  node ptr: %i", slotNode);
//				movie->addChild(slotNode);

				return slotNode;
			}break;
			case AE_MOVIE_LAYER_TYPE_VIDEO: {
				//XCODE COMPILE COMMENT: CCLOG(" Video:");
				
			}break;
			case AE_MOVIE_LAYER_TYPE_SOUND: {
				AESound *sound = static_cast<AESound *>(ae_get_movie_layer_data_resource_data(_callbackData->layer));
				
				//XCODE COMPILE COMMENT: CCLOG(" Sound:");
				//XCODE COMPILE COMMENT: CCLOG("  name: '%s'", sound->getPath().c_str());
				//XCODE COMPILE COMMENT: CCLOG("  sound ptr: %i",  sound);

				// FIXME: wrap into separate method
				auto soundNode = AESoundNode::create(sound);
				soundNode->retain();

				movie->addSoundNode(soundNode);

				//XCODE COMPILE COMMENT: CCLOG(" node ptr: %i", soundNode);

				return soundNode;
			}break;
		}
	}
	else {
		//XCODE COMPILE COMMENT: CCLOG(" Has track matte.");

		switch( layerType ) {
            case AE_MOVIE_LAYER_TYPE_MOVIE:
            case AE_MOVIE_LAYER_TYPE_TEXT:
            case AE_MOVIE_LAYER_TYPE_EVENT:
            case AE_MOVIE_LAYER_TYPE_SOCKET:
            case AE_MOVIE_LAYER_TYPE_SLOT:
            case AE_MOVIE_LAYER_TYPE_NULL:
            case AE_MOVIE_LAYER_TYPE_SOLID:
            case AE_MOVIE_LAYER_TYPE_SEQUENCE:
            case AE_MOVIE_LAYER_TYPE_VIDEO:
            case AE_MOVIE_LAYER_TYPE_SOUND:
            case AE_MOVIE_LAYER_TYPE_PARTICLE:
            case AE_MOVIE_LAYER_TYPE_SUB_MOVIE:
                break;
            case AE_MOVIE_LAYER_TYPE_SHAPE:
				//XCODE COMPILE COMMENT: CCLOG(" Shape:");
				break;
			case AE_MOVIE_LAYER_TYPE_IMAGE:
                Texture2D *texture = static_cast<Texture2D *>(ae_get_movie_layer_data_resource_data( _callbackData->track_matte_layer ));

				auto trackMatteNode = AETrackMatteNode::createFromTexture(texture);
				trackMatteNode->retain();
				//trackMatteNode->setName(layerName);

				movie->addTrackMatteNode(trackMatteNode);

				//XCODE COMPILE COMMENT: CCLOG(" Image:");
				//XCODE COMPILE COMMENT: CCLOG("  node ptr: %i", trackMatteNode);

				return trackMatteNode;
		}
	}

	return nullptr;
}

void AEMovie::callbackNodeDeleter( const aeMovieNodeDeleterCallbackData * _callbackData, ae_voidptr_t _data ) {
    AE_UNUSED( _callbackData );
    AE_UNUSED( _data );
    
	//XCODE COMPILE COMMENT: CCLOG("CALL: node destroyer");
}

void AEMovie::callbackNodeUpdate( const aeMovieNodeUpdateCallbackData * _callbackData, ae_voidptr_t _data ) {
    AE_UNUSED( _data );

    //XCODE COMPILE COMMENT: CCLOG("CALL: node update");

//	AEMovie *movie = static_cast<AEMovie *>(_data);

	switch (_callbackData->state)
	{
        case AE_MOVIE_STATE_UPDATE_BEGIN:
        {
            //XCODE COMPILE COMMENT: CCLOG("AE_MOVIE_STATE_UPDATE_BEGIN");
            
            switch (_callbackData->type)
            {
                case AE_MOVIE_LAYER_TYPE_MOVIE:
                case AE_MOVIE_LAYER_TYPE_TEXT:
                case AE_MOVIE_LAYER_TYPE_EVENT:
                case AE_MOVIE_LAYER_TYPE_SOCKET:
                case AE_MOVIE_LAYER_TYPE_SHAPE:
                case AE_MOVIE_LAYER_TYPE_SLOT:
                case AE_MOVIE_LAYER_TYPE_NULL:
                case AE_MOVIE_LAYER_TYPE_SOLID:
                case AE_MOVIE_LAYER_TYPE_SEQUENCE:
                case AE_MOVIE_LAYER_TYPE_PARTICLE:
                case AE_MOVIE_LAYER_TYPE_IMAGE:
                case AE_MOVIE_LAYER_TYPE_SUB_MOVIE:
                    break;
                case AE_MOVIE_LAYER_TYPE_VIDEO:
                    //XCODE COMPILE COMMENT: CCLOG(" Video:");
                    break;
                case AE_MOVIE_LAYER_TYPE_SOUND:
                    AESoundNode *soundNode = static_cast<AESoundNode *>(_callbackData->element);
                    
                    //XCODE COMPILE COMMENT: CCLOG("  node ptr: %i", soundNode);
                    //XCODE COMPILE COMMENT: CCLOG(" Sound:");
                    //XCODE COMPILE COMMENT: CCLOG("  sound ptr: %i", soundNode->getSound());
                    //XCODE COMPILE COMMENT: CCLOG("  name: '%s'", soundNode->getSound()->getPath());
                    //XCODE COMPILE COMMENT: CCLOG("  offset: %.2f sec", _callbackData->offset * 0.001f);
                    
                    int id = soundNode->getAudioId();
                    
                    // FIXME: maybe don't stop, just set time instead
                    if (id != AudioEngine::INVALID_AUDIO_ID) {
                        AudioEngine::stop(id);
                    }
                    
                    id = AudioEngine::play2d(soundNode->getSound()->getPath());
                    
                    //XCODE COMPILE COMMENT: CCLOG("  node audio id: %i", id);
                    //XCODE COMPILE COMMENT: CCLOG("  node audio state: %i", AudioEngine::getState(id));
                    
                    soundNode->setAudioId(id);
                    soundNode->setTime(_callbackData->offset * 0.001f);
                    
                    break;
            }
        }break;
		case AE_MOVIE_STATE_UPDATE_PROCESS:
		{
			//XCODE COMPILE COMMENT: CCLOG("AE_MOVIE_STATE_UPDATE_PROCESS");

			switch (_callbackData->type) {
                case AE_MOVIE_LAYER_TYPE_MOVIE:
                case AE_MOVIE_LAYER_TYPE_TEXT:
                case AE_MOVIE_LAYER_TYPE_EVENT:
                case AE_MOVIE_LAYER_TYPE_SOCKET:
                case AE_MOVIE_LAYER_TYPE_SHAPE:
                case AE_MOVIE_LAYER_TYPE_NULL:
                case AE_MOVIE_LAYER_TYPE_SOLID:
                case AE_MOVIE_LAYER_TYPE_SEQUENCE:
                case AE_MOVIE_LAYER_TYPE_VIDEO:
                case AE_MOVIE_LAYER_TYPE_SOUND:
                case AE_MOVIE_LAYER_TYPE_IMAGE:
                case AE_MOVIE_LAYER_TYPE_SUB_MOVIE:
                    break;
				case AE_MOVIE_LAYER_TYPE_PARTICLE:
					//XCODE COMPILE COMMENT: CCLOG(" Particle:");
					break;
				case AE_MOVIE_LAYER_TYPE_SLOT:
					//XCODE COMPILE COMMENT: CCLOG(" Slot:");

					AESlotNode *slotNode = static_cast<AESlotNode *>(_callbackData->element);
					slotNode->setAdditionalTransform(Mat4(_callbackData->matrix));

					#if COCOS2D_DEBUG > 0
					const float *f = _callbackData->matrix;
					//XCODE COMPILE COMMENT: CCLOG("f0: %.2f %.2f %.2f %.2f", f[0], f[1], f[2], f[3]);
					//XCODE COMPILE COMMENT: CCLOG("f1: %.2f %.2f %.2f %.2f", f[4], f[5], f[6], f[7]);
					//XCODE COMPILE COMMENT: CCLOG("f2: %.2f %.2f %.2f %.2f", f[8], f[9], f[10], f[11]);
					//XCODE COMPILE COMMENT: CCLOG("f3: %.2f %.2f %.2f %.2f", f[12], f[13], f[14], f[15]);
					#endif

					break;
			}
		}break;
        case AE_MOVIE_STATE_UPDATE_PAUSE:
            break;
        case AE_MOVIE_STATE_UPDATE_RESUME:
            break;
		case AE_MOVIE_STATE_UPDATE_END:
		{
			//XCODE COMPILE COMMENT: CCLOG("AE_MOVIE_STATE_UPDATE_END");

			switch (_callbackData->type)
			{
                case AE_MOVIE_LAYER_TYPE_MOVIE:
                case AE_MOVIE_LAYER_TYPE_TEXT:
                case AE_MOVIE_LAYER_TYPE_EVENT:
                case AE_MOVIE_LAYER_TYPE_SOCKET:
                case AE_MOVIE_LAYER_TYPE_SHAPE:
                case AE_MOVIE_LAYER_TYPE_SLOT:
                case AE_MOVIE_LAYER_TYPE_NULL:
                case AE_MOVIE_LAYER_TYPE_SOLID:
                case AE_MOVIE_LAYER_TYPE_SEQUENCE:
                case AE_MOVIE_LAYER_TYPE_PARTICLE:
                case AE_MOVIE_LAYER_TYPE_IMAGE:
                case AE_MOVIE_LAYER_TYPE_SUB_MOVIE:
                    break;
				case AE_MOVIE_LAYER_TYPE_VIDEO:
					//XCODE COMPILE COMMENT: CCLOG(" Video:");
					break;
				case AE_MOVIE_LAYER_TYPE_SOUND:
					AESoundNode *soundNode = static_cast<AESoundNode *>(_callbackData->element);
					//XCODE COMPILE COMMENT: CCLOG("  node ptr: %i", soundNode);

					//XCODE COMPILE COMMENT: CCLOG(" Sound:");
					//XCODE COMPILE COMMENT: CCLOG("  name: '%s'", soundNode->getSound()->getPath());

					AudioEngine::stop(soundNode->getAudioId());

					// TODO: rework into soundNode->stop() etc.
					soundNode->setAudioId(AudioEngine::INVALID_AUDIO_ID);

					break;
			}
		}break;
	}
}

ae_voidptr_t AEMovie::callbackCompositionTrackMatteProvider( const aeMovieTrackMatteProviderCallbackData * _callbackData, ae_voidptr_t _data ) {
    
    AEMovie *movie = static_cast<AEMovie *>(_data);

    // FIXME: count track matte on load & use a vector of simple structs instead
    auto tmData = AETrackMatteData::create();
    tmData->retain();
    tmData->setMatrix( _callbackData->matrix );
    tmData->setRenderMesh( *_callbackData->mesh );

    movie->addTrackMatteData( tmData );

    return tmData;
}

void AEMovie::callbackCompositionTrackMatteUpdate( const aeMovieTrackMatteUpdateCallbackData * _callbackData, ae_voidptr_t _data ) {
	//XCODE COMPILE COMMENT: CCLOG("CALL: composition track matte update");

	AEMovie *movie = static_cast<AEMovie *>(_data);
    AE_UNUSED(movie);

	switch (_callbackData->state) {
		case AE_MOVIE_STATE_UPDATE_BEGIN: {
			//XCODE COMPILE COMMENT: CCLOG("AE_MOVIE_STATE_UPDATE_BEGIN");
			
			AETrackMatteData * tmData = static_cast<AETrackMatteData *>(_callbackData->track_matte_data);
			tmData->setMatrix(_callbackData->matrix);
			tmData->setRenderMesh(*_callbackData->mesh);
			
		}break;
		case AE_MOVIE_STATE_UPDATE_PROCESS: {
			//XCODE COMPILE COMMENT: CCLOG("AE_MOVIE_STATE_UPDATE_PROCESS");

			AETrackMatteData * tmData = static_cast<AETrackMatteData *>(_callbackData->track_matte_data);
			tmData->setMatrix(_callbackData->matrix);
			tmData->setRenderMesh(*_callbackData->mesh);
			
		}break;
        case AE_MOVIE_STATE_UPDATE_PAUSE:
        case AE_MOVIE_STATE_UPDATE_RESUME:
            break;
		case AE_MOVIE_STATE_UPDATE_END: {
			//XCODE COMPILE COMMENT: CCLOG("NODE_UPDATE_END");
/*
			AETrackMatteData * tmData = static_cast<AETrackMatteData *>(_callbackData->track_matte_data);

			if (tmData == nullptr)
				//XCODE COMPILE COMMENT: CCLOG("WTF data NULL");

			CC_SAFE_RELEASE(tmData);
*/
		}break;
	}
}

void AEMovie::callbackCompositionEvent( const aeMovieCompositionEventCallbackData * _callbackData, ae_voidptr_t _data ) {
    AE_UNUSED( _callbackData );
    AE_UNUSED( _data );
	//XCODE COMPILE COMMENT: CCLOG("CALL: composition event");
}

void AEMovie::callbackCompositionState( const aeMovieCompositionStateCallbackData * _callbackData, ae_voidptr_t _data ) {
	//XCODE COMPILE COMMENT: CCLOG("CALL: composition state");

	if (_callbackData->subcomposition != nullptr)
		return;

	AEMovie *movie = static_cast<AEMovie *>(_data);

	if (_callbackData->state == AE_MOVIE_COMPOSITION_END) {
		movie->stop();
	}
}

void AEMovie::addCamera(const std::string & name, Camera *camera) {
	TMapCamera::iterator it = _cameras.find(name);

	if (it != _cameras.end()) {
		//XCODE COMPILE COMMENT: CCLOG("addCamera(): camera '%s' already exists.", name);
		return;
	}

	_cameras.insert(std::make_pair(name, camera));
}

Camera *AEMovie::getCamera(const std::string & name) {
	TMapCamera::iterator it = _cameras.find(name);

	if (it == _cameras.end()) {
		//XCODE COMPILE COMMENT: CCLOG("getCamera(): camera '%s' does not exist.", name);
		return nullptr;
	}

	return it->second;
}

void AEMovie::addSoundNode(AESoundNode * node) {
	_soundNodes.push_back(node);
}

void AEMovie::addSlotNode(const std::string& name, AESlotNode *node) {
	TMapSlotNode::iterator it = _slotNodes.find(name);

	if (it != _slotNodes.end()) {
		//XCODE COMPILE COMMENT: CCLOG("addSlotNode(): node '%s' already exists.", name);
		return;
	}

	_slotNodes.insert(std::make_pair(name, node));
}

AESlotNode *AEMovie::getSlotNode(const std::string & name) {
	TMapSlotNode::iterator it = _slotNodes.find(name);

	if (it == _slotNodes.end()) {
		//XCODE COMPILE COMMENT: CCLOG("getSlotNode(): node '%s' does not exist.", name);
		return nullptr;
	}

	return it->second;
}

void AEMovie::addTrackMatteNode(AETrackMatteNode * node) {
	_trackMatteNodes.push_back(node);
}

void AEMovie::addTrackMatteData(AETrackMatteData * data) {
	_trackMatteDatas.push_back(data);
}

AEMovie * AEMovie::create(const std::string & filepath) {
	AEMovie * ret = new (std::nothrow) AEMovie();

	if (ret && ret->initWithFile(filepath)) {
		ret->autorelease();
		return ret;
	}

       	CC_SAFE_DELETE(ret);

	return nullptr;
}

AEMovie::AEMovie()
: _normalGPS(nullptr)
, _trackMatteGPS(nullptr)
, _movieData(nullptr)
, _composition(nullptr)
, _time(0.f)
, _duration(0.f)
, _eventCallback(nullptr)
{
#ifdef AE_MOVIE_DEBUG_DRAW
	_debugDrawNode = DrawNode::create();
	addChild(_debugDrawNode);
#endif
}

AEMovie::~AEMovie()
{
	if (_composition != nullptr) {
		//XCODE COMPILE COMMENT: CCLOG("Deleting movie composition.");
		ae_delete_movie_composition(_composition );
		_composition = nullptr;
	}

//	_isPlayingSubComposition = false;

	// decrease all cocos Ref counters
	for (TMapCamera::iterator cameraIt = _cameras.begin(); cameraIt != _cameras.end(); cameraIt++)
		CC_SAFE_RELEASE(cameraIt->second);
	for (TVectorSoundNode::iterator soundNodeIt = _soundNodes.begin(); soundNodeIt != _soundNodes.end(); soundNodeIt++)
		CC_SAFE_RELEASE(*soundNodeIt);
	for (TMapSlotNode::iterator slotNodeIt = _slotNodes.begin(); slotNodeIt != _slotNodes.end(); slotNodeIt++)
		CC_SAFE_RELEASE(slotNodeIt->second);
	for (TVectorTrackMatteNode::iterator tmNodeIt = _trackMatteNodes.begin(); tmNodeIt != _trackMatteNodes.end(); tmNodeIt++)
		CC_SAFE_RELEASE(*tmNodeIt);
	for (TVectorTrackMatteData::iterator tmDataIt = _trackMatteDatas.begin(); tmDataIt != _trackMatteDatas.end(); tmDataIt++)
		CC_SAFE_RELEASE(*tmDataIt);

	// clear lists
/*
	_renderDatas.clear();
	_cameras.clear();
	_soundNodes.clear();
	_slotNodes.clear();
	_trackMatteNodes.clear();
	_trackMatteDatas.clear();
*/
}

bool AEMovie::initWithFile(const std::string & filepath)
{
	// FIXME: move cache to the Director
	AEMovieData * data = AEMovieCache::getInstance()->addMovie(filepath);
	return initWithData(data);
}

bool AEMovie::initWithData(const AEMovieData * data)
{
/*
	_renderDatas.clear();
	_cameras.clear();
	_soundNodes.clear();
	_slotNodes.clear();
	_trackMatteNodes.clear();
	_trackMatteDatas.clear();
*/

	// get pre-cached shaders
	auto pc = GLProgramCache::getInstance();
	auto p1 = pc->getGLProgram("AEM_Normal");
	auto p2 = pc->getGLProgram("AEM_TrackMatte");

	// create or get cached GL states for them
	_normalGPS = GLProgramState::getOrCreateWithGLProgram(p1);
	_trackMatteGPS = GLProgramState::getOrCreateWithGLProgram(p2);

	// set data pointer
	_movieData = data;

	//
	// start update loop
	//

	scheduleUpdate();

	return true;
}

void AEMovie::setComposition(const std::string & name)
{
    if (name.empty())
    {
        //XCODE COMPILE COMMENT: CCLOG("Called AEMovie::setComposition with blank composition name.");
        return;
    }

    if (!_movieData)
    {
        //XCODE COMPILE COMMENT: CCLOG("Called AEMovie::setComposition with NULL movie data.");
        return;
    }


	// look for it
	const aeMovieCompositionData * compositionData = ae_get_movie_composition_data( _movieData->getData(), name.c_str() );

	if( compositionData == AE_NULL )
	{
		//XCODE COMPILE COMMENT: CCLOG("Composition '%s' not found.", name.c_str());
		return;
	}

    float composition_duration = ae_get_movie_composition_data_duration( compositionData );
	
    _duration = composition_duration;

	//XCODE COMPILE COMMENT: CCLOG("Composition: '%s'", name.c_str());
	//XCODE COMPILE COMMENT: CCLOG(" width: %.2f", compositionData->width);
	//XCODE COMPILE COMMENT: CCLOG(" height: %.2f", compositionData->height);
	//XCODE COMPILE COMMENT: CCLOG(" duration: %.2f sec", _duration);
	//XCODE COMPILE COMMENT: CCLOG(" anchor point: %.2f %.2f %.2f", compositionData->anchor_point[0], compositionData->anchor_point[1], compositionData->anchor_point[2]);
	//XCODE COMPILE COMMENT: CCLOG(" offset point: %.2f %.2f %.2f", compositionData->offset_point[0], compositionData->offset_point[1], compositionData->offset_point[2]);
	//XCODE COMPILE COMMENT: CCLOG(" bounds: %.2f %.2f %.2f %.2f", compositionData->bounds[0], compositionData->bounds[1], compositionData->bounds[2], compositionData->bounds[3]);

	// free the previous one
	if (_composition) {
		ae_delete_movie_composition(_composition );
		_composition = nullptr;
	}

	// create new
	aeMovieCompositionProviders providers;

	providers.camera_provider = &callbackCameraProvider;
	providers.node_provider = &callbackNodeProvider;
	providers.node_deleter = &callbackNodeDeleter;
	providers.node_update = &callbackNodeUpdate;
    providers.track_matte_provider = &callbackCompositionTrackMatteProvider;
	providers.track_matte_update = &callbackCompositionTrackMatteUpdate;
	providers.composition_event = &callbackCompositionEvent;
	providers.composition_state = &callbackCompositionState;

	_composition = ae_create_movie_composition( _movieData->getData(), compositionData, AE_TRUE, &providers, this );

	if (_composition == nullptr)
	{
		//XCODE COMPILE COMMENT: CCLOG("Failed to create composition.");
		return;
	}

	ae_set_movie_composition_loop( _composition, AE_TRUE );

	// reserve storage
    aeMovieCompositionRenderInfo info;
    ae_calculate_movie_composition_render_info( _composition, &info );

    _renderDatas.reserve( info.max_render_node );

	// don't play from start
	stop();

	// set node original content size
    float width = ae_get_movie_composition_data_width( compositionData );
    float height = ae_get_movie_composition_data_height( compositionData );

    setContentSize( Size( width, height ) );
}

void AEMovie::setTime(float t) {
	_time = t;
	onPlayEvent((int)AEMovie::EventType::TIME_CHANGED);

	//XCODE COMPILE COMMENT: CCLOG("Set time to: %.2f sec.", _time);

	ae_stop_movie_composition( _composition );
	ae_play_movie_composition( _composition, _time );
//	onPlayEvent((int)AEMovie::EventType::PLAYING);

/*
	// TODO: make work (might need to add that to libmovie)
	for (TVectorSoundNode::iterator soundNodeIt = _soundNodes.begin(); soundNodeIt != _soundNodes.end(); soundNodeIt++) {
		AudioEngine::setCurrentTime(*soundNodeIt->getAudioId(), );
	}
*/
}

void AEMovie::play() {
	//XCODE COMPILE COMMENT: CCLOG("AEMOVIEPLAY");

	resume();

	ae_play_movie_composition( _composition, _time );

	//
	// run subcomposition (start playing it in a loop just once for test)
	//
/*
	if (!_isPlayingSubComposition) {
		const aeMovieSubComposition * subComposition = ae_get_movie_sub_composition( _composition, "1-1" );

		if( subComposition != nullptr )
		{
			ae_set_movie_sub_composition_loop( subComposition, AE_TRUE );
			ae_play_movie_sub_composition( _composition, subComposition, 0.f );
		}

		_isPlayingSubComposition = true;
	}
*/

	onPlayEvent((int)AEMovie::EventType::PLAYING);
}

void AEMovie::stop() {
	//XCODE COMPILE COMMENT: CCLOG("AEMOVIESTOP");	

	_time = 0.f;
	onPlayEvent((int)AEMovie::EventType::TIME_CHANGED);

	ae_stop_movie_composition( _composition );
	onPlayEvent((int)AEMovie::EventType::STOPPED);

	// stop all playing audio streams
	for (TVectorSoundNode::iterator soundNodeIt = _soundNodes.begin(); soundNodeIt != _soundNodes.end(); soundNodeIt++)
		AudioEngine::stop((*soundNodeIt)->getAudioId());
}

void AEMovie::interrupt(bool skip) {
	//XCODE COMPILE COMMENT: CCLOG("AEMOVIEINTERRUPT");	

    ae_interrupt_movie_composition( _composition, skip );
}

void AEMovie::pause() {
	//XCODE COMPILE COMMENT: CCLOG("AEMOVIEPAUSE");	

	Node::pause();

	onPlayEvent((int)AEMovie::EventType::PAUSED);

	// pause all playing audio streams
	for (TVectorSoundNode::iterator soundNodeIt = _soundNodes.begin(); soundNodeIt != _soundNodes.end(); soundNodeIt++)
		AudioEngine::pause((*soundNodeIt)->getAudioId());
}

void AEMovie::resume() {
	//XCODE COMPILE COMMENT: CCLOG("AEMOVIERESUME");

	Node::resume();

	// resume all playing audio streams
	for (TVectorSoundNode::iterator soundNodeIt = _soundNodes.begin(); soundNodeIt != _soundNodes.end(); soundNodeIt++)
		AudioEngine::resume((*soundNodeIt)->getAudioId());

	onPlayEvent((int)AEMovie::EventType::PLAYING);
}

void AEMovie::addEventListener(const AEMovie::TMovieCallback& callback) {
	_eventCallback = callback;
}

void AEMovie::onPlayEvent(int event) {
        AEMovie::EventType movieEvent = (AEMovie::EventType)event;

//	if (movieEvent == AEMovie::EventType::STOPPED) {
//		_isPlayingSubComposition = false;
//	}

        if (_eventCallback) {
            _eventCallback(this, movieEvent);
        }
}

void AEMovie::update(float delta) {
	Node::update(delta);

	if (_composition == nullptr)
		return;
	if (ae_is_play_movie_composition(_composition) == AE_FALSE)
		return;

	ae_update_movie_composition( _composition, delta );

//	float t = ae_get_movie_composition_time(_composition);

	_time += delta;
	onPlayEvent((int)AEMovie::EventType::TIME_CHANGED);

	//XCODE COMPILE COMMENT: CCLOG("Composition time mark is %.2f sec.", _time);
}

static void calc_uv_clip_vectors( const Vec2 & _a, const Vec2 & _b, const Vec2 & _c, const Vec2 & _auv, const Vec2 & _buv, const Vec2 & _cuv, Vec3 * _out) {
	Vec2 _uvA = _auv;
	Vec2 _dAB = _b - _a;
	Vec2 _dAC = _c - _a;

	float inv_v = 1.f / (_dAB.x * _dAC.y - _dAB.y * _dAC.x);
	_dAB *= inv_v;
	_dAC *= inv_v;

	Vec2 _dac;
	_dac.x = _dAC.x * _a.y - _dAC.y * _a.x;
	_dac.y = -(_dAB.x * _a.y - _dAB.y * _a.x);

	_dAB *= -1.f;

	_dAB.x *= -1.f;
	_dAC.x *= -1.f;

	Vec2 _duvAB = _buv - _uvA;
	Vec2 _duvAC = _cuv - _uvA;

	_out[0] = Vec3( _dac.x, _dAC.y, _dAC.x );
	_out[1] = Vec3( _dac.y, _dAB.y, _dAB.x );
	_out[2] = Vec3( _uvA.x, _duvAB.x, _duvAC.x );
	_out[3] = Vec3( _uvA.y, _duvAB.y, _duvAC.y );
}

void AEMovie::draw(Renderer * renderer, const Mat4 & transform, uint32_t flags) {
#ifdef AE_MOVIE_DEBUG_DRAW
	auto size = getContentSize();

	_debugDrawNode->clear();
	_debugDrawNode->drawRect(Vec2(0.f, 0.f), Vec2(size.width, size.height), Color4F::WHITE);
#endif

	if (_composition == nullptr) {
		return;
	}

	_renderDatas.clear();

	uint32_t renderMeshIt = 0;
	aeMovieRenderMesh renderMesh;

	//XCODE COMPILE COMMENT: CCLOG("AEMovie::Draw(): BEGIN");

	while( ae_compute_movie_mesh( _composition, &renderMeshIt, &renderMesh ) == AE_TRUE )
	{	
		// TODO: pre-cache mesh data
		//XCODE COMPILE COMMENT: CCLOG( "Rendering mesh %i.", renderMeshIt );

		if( renderMesh.camera_data != nullptr ) {
			//XCODE COMPILE COMMENT: CCLOG("Has camera.");
		}
		else {
			//XCODE COMPILE COMMENT: CCLOG("No camera.");
		}

		if( renderMesh.track_matte_data == nullptr ) {
			//
			// layer has no track matte
			//

			//XCODE COMPILE COMMENT: CCLOG("No track matte.");

			switch( renderMesh.layer_type )
			{
                case AE_MOVIE_LAYER_TYPE_TEXT:
                case AE_MOVIE_LAYER_TYPE_EVENT:
                case AE_MOVIE_LAYER_TYPE_SOCKET:
                case AE_MOVIE_LAYER_TYPE_NULL:
                    break;
				case AE_MOVIE_LAYER_TYPE_MOVIE:
				{
					//XCODE COMPILE COMMENT: CCLOG("Layer type: movie.");
					break;
				}
				case AE_MOVIE_LAYER_TYPE_SHAPE:
				{
					//XCODE COMPILE COMMENT: CCLOG("Layer type: shape.");
					break;
				}
				case AE_MOVIE_LAYER_TYPE_SLOT:
				{
					//XCODE COMPILE COMMENT: CCLOG("Layer type: slot.");

					AESlotNode * slotNode = static_cast<AESlotNode *>(renderMesh.element_data);
					//XCODE COMPILE COMMENT: CCLOG(" node ptr: %i", (int)slotNode);

					// render it & children
					slotNode->visit(renderer, transform, flags);

					break;
				}
				case AE_MOVIE_LAYER_TYPE_SOLID:
				{
					//XCODE COMPILE COMMENT: CCLOG("Layer type: solid.");
					break;
				}
				case AE_MOVIE_LAYER_TYPE_SEQUENCE:
				case AE_MOVIE_LAYER_TYPE_IMAGE:
				{
					//XCODE COMPILE COMMENT: if (renderMesh.layer_type == AE_MOVIE_LAYER_TYPE_SEQUENCE)
						//XCODE COMPILE COMMENT: CCLOG("Layer type: sequence.");
					//XCODE COMPILE COMMENT: else
						//XCODE COMPILE COMMENT: CCLOG("Layer type: image.");

					if (renderMesh.vertexCount == 0 || renderMesh.indexCount == 0)
						break;

					//XCODE COMPILE COMMENT: CCLOG("mesh info:");
					//XCODE COMPILE COMMENT: CCLOG(" vertex count = %i", renderMesh.vertexCount);
					//XCODE COMPILE COMMENT: CCLOG(" index count = %i", renderMesh.indexCount);
					//XCODE COMPILE COMMENT: CCLOG(" RGBA: %.2f %.2f %.2f %.2f", renderMesh.r, renderMesh.g, renderMesh.b, renderMesh.opacity);
					//XCODE COMPILE COMMENT: CCLOG(" blendfunc: %i", renderMesh.blend_mode);

					_renderDatas.push_back(AERenderData());
					AERenderData & renderData = _renderDatas.back();

					//
					// setup vertices
					//

					renderData.vertices.resize(renderMesh.vertexCount);

                    ae_uint32_t i;
					const float * p;
					const float * uv;

					for (i = 0, p = &renderMesh.position[0][0], uv = renderMesh.uv[0]; i < renderMesh.vertexCount; i++, p += 3, uv += 2) {
						V3F_C4B_T2F &v = renderData.vertices[i];

						v.vertices = Vec3(p[0], p[1], p[2]);
						v.colors = Color4B(Color4F(renderMesh.color.r, renderMesh.color.g, renderMesh.color.b, renderMesh.opacity));
						v.texCoords = Tex2F(uv[0], uv[1]);

						//XCODE COMPILE COMMENT: CCLOG(" vertex %i:", i);
						//XCODE COMPILE COMMENT: CCLOG("  position = %.2f %.2f %.2f", p[0], p[1], p[2]);
						//XCODE COMPILE COMMENT: CCLOG("  uv       = %.2f %.2f", uv[0], uv[1], uv[2]);
						//XCODE COMPILE COMMENT: CCLOG("  color    = %.2f %.2f %.2f %.2f", renderMesh.r, renderMesh.g, renderMesh.b, renderMesh.opacity);
					}

					TrianglesCommand::Triangles triangles;
					triangles.verts = renderData.vertices.data();
					triangles.vertCount = renderMesh.vertexCount;

					//
					// setup indices
					//

					renderData.indices.assign(renderMesh.indices, renderMesh.indices + renderMesh.indexCount);

					triangles.indices = renderData.indices.data();
					triangles.indexCount = renderMesh.indexCount;

					#if COCOS2D_DEBUG > 0
					for (i = 0; i < triangles.indexCount; i++) {
						//XCODE COMPILE COMMENT: CCLOG(" index %i = %i", i, triangles.indices[i]);
					}
					#endif

					BlendFunc blendFunc;

					switch(renderMesh.blend_mode) {
						case AE_MOVIE_BLEND_ADD:
							blendFunc.src = GL_SRC_ALPHA;
							blendFunc.dst = GL_ONE;
							break;
//						case AE_MOVIE_BLEND_NORMAL:
						default:
							blendFunc.src = GL_SRC_ALPHA;
							blendFunc.dst = GL_ONE_MINUS_SRC_ALPHA;
							break;
					}

//					//XCODE COMPILE COMMENT: CCLOG(" resource type: %i", (int)renderMesh.resource_type);
//					//XCODE COMPILE COMMENT: CCLOG(" resource data: %i", (int)renderMesh.resource_data);

					Texture2D * texture = static_cast<Texture2D *>(renderMesh.resource_data);

					#if COCOS2D_DEBUG > 0
					const Texture2D::PixelFormatInfo & pfi = Texture2D::getPixelFormatInfoMap().at(texture->getPixelFormat());

					//XCODE COMPILE COMMENT: CCLOG(" Texture: '%s'", texture->getDescription().c_str());
					//XCODE COMPILE COMMENT: CCLOG("  internal format: 0x%x", pfi.internalFormat);
					//XCODE COMPILE COMMENT: CCLOG("  format: 0x%x", pfi.format);
					//XCODE COMPILE COMMENT: CCLOG("  type: 0x%x", pfi.type);
					//XCODE COMPILE COMMENT: CCLOG("  bpp: %i", pfi.bpp);
					//XCODE COMPILE COMMENT: CCLOG("  compressed: %i", pfi.compressed);
					//XCODE COMPILE COMMENT: CCLOG("  alpha: %i", pfi.alpha);
					#endif

					// don't do this because cocos binds the trianglesCommand.texture as CC_Texture0
//					_normalGPS->setUniformTexture("u_texture", texture);

					setGLProgramState(_normalGPS);

///					setGLProgramState(GLProgramState::getOrCreateWithGLProgramName(GLProgram::SHADER_NAME_POSITION_TEXTURE_COLOR_NO_MVP, texture));
//					setGLProgramState(GLProgramState::getOrCreateWithGLProgramName(GLProgram::SHADER_NAME_POSITION_TEXTURE_COLOR, texture));
//					setGLProgramState(GLProgramState::getOrCreateWithGLProgramName(GLProgram::SHADER_NAME_POSITION_COLOR_NO_MVP));
//					setGLProgramState(GLProgramState::getOrCreateWithGLProgramName(GLProgram::SHADER_NAME_POSITION_COLOR));

					renderData.trianglesCommand.init(
						_globalZOrder,
						texture,
						getGLProgramState(),
						blendFunc,
						triangles,
						transform,
						flags);

					renderer->addCommand(&renderData.trianglesCommand);

					break;
				}
				case AE_MOVIE_LAYER_TYPE_VIDEO:
				{
					//XCODE COMPILE COMMENT: CCLOG("Layer type: video.");
					break;
				}
				case AE_MOVIE_LAYER_TYPE_SOUND:
				{
					//XCODE COMPILE COMMENT: CCLOG("Layer type: sound.");
					break;
				}
				case AE_MOVIE_LAYER_TYPE_PARTICLE:
				{
					//XCODE COMPILE COMMENT: CCLOG("Layer type: particle.");
					break;
				}
				case AE_MOVIE_LAYER_TYPE_SUB_MOVIE:
				{
					//XCODE COMPILE COMMENT: CCLOG("Layer type: submovie.");
					break;
				}
			}
		}
		else {
			//
			// layer has track matte
			//

			//XCODE COMPILE COMMENT: CCLOG("Has track matte.");

			switch( renderMesh.layer_type )
			{
                case AE_MOVIE_LAYER_TYPE_MOVIE:
                case AE_MOVIE_LAYER_TYPE_TEXT:
                case AE_MOVIE_LAYER_TYPE_EVENT:
                case AE_MOVIE_LAYER_TYPE_SOCKET:
                case AE_MOVIE_LAYER_TYPE_SHAPE:
                case AE_MOVIE_LAYER_TYPE_SLOT:
                case AE_MOVIE_LAYER_TYPE_NULL:
                case AE_MOVIE_LAYER_TYPE_SOLID:
                case AE_MOVIE_LAYER_TYPE_SEQUENCE:
                case AE_MOVIE_LAYER_TYPE_VIDEO:
                case AE_MOVIE_LAYER_TYPE_SOUND:
                case AE_MOVIE_LAYER_TYPE_PARTICLE:
                case AE_MOVIE_LAYER_TYPE_SUB_MOVIE:
                    break;
				case AE_MOVIE_LAYER_TYPE_IMAGE:
				{
					//XCODE COMPILE COMMENT: CCLOG("Layer type: image.");

					const AETrackMatteNode * trackMatteNode = static_cast<const AETrackMatteNode *>(renderMesh.element_data);

					if (trackMatteNode == nullptr)
						break;
					if (renderMesh.vertexCount == 0 || renderMesh.indexCount == 0)
						break;

					//XCODE COMPILE COMMENT: CCLOG("mesh info:");
					//XCODE COMPILE COMMENT: CCLOG(" vertex count = %i", renderMesh.vertexCount);
					//XCODE COMPILE COMMENT: CCLOG(" index count = %i", renderMesh.indexCount);
					//XCODE COMPILE COMMENT: CCLOG(" RGBA: %.2f %.2f %.2f %.2f", renderMesh.r, renderMesh.g, renderMesh.b, renderMesh.opacity);
					//XCODE COMPILE COMMENT: CCLOG(" blendfunc: %i", renderMesh.blend_mode);

					_renderDatas.push_back(AERenderData());
					AERenderData & renderData = _renderDatas.back();

					AETrackMatteData * tmData = static_cast<AETrackMatteData *>(renderMesh.track_matte_data);
//					const Mat4 & tmMatrix = tmData->getMatrix();
					const aeMovieRenderMesh & tmRenderMesh = tmData->getRenderMesh();
/*
					#if COCOS2D_DEBUG > 0
					const float *f = tmMatrix.m;
					//XCODE COMPILE COMMENT: CCLOG(" tmMatrix:");
					//XCODE COMPILE COMMENT: CCLOG("  f0: %.2f %.2f %.2f %.2f", f[0], f[1], f[2], f[3]);
					//XCODE COMPILE COMMENT: CCLOG("  f1: %.2f %.2f %.2f %.2f", f[4], f[5], f[6], f[7]);
					//XCODE COMPILE COMMENT: CCLOG("  f2: %.2f %.2f %.2f %.2f", f[8], f[9], f[10], f[11]);
					//XCODE COMPILE COMMENT: CCLOG("  f3: %.2f %.2f %.2f %.2f", f[12], f[13], f[14], f[15]);
					#endif
*/
					if (tmRenderMesh.vertexCount == 0 || tmRenderMesh.indexCount == 0)
						break;

					//XCODE COMPILE COMMENT: CCLOG("tm mesh info:");
					//XCODE COMPILE COMMENT: CCLOG(" position[0]: %.2f %.2f", tmRenderMesh.position[0][0], tmRenderMesh.position[0][1]);
					//XCODE COMPILE COMMENT: CCLOG(" position[1]: %.2f %.2f", tmRenderMesh.position[1][0], tmRenderMesh.position[1][1]);
					//XCODE COMPILE COMMENT: CCLOG(" position[2]: %.2f %.2f", tmRenderMesh.position[2][0], tmRenderMesh.position[2][1]);
					//XCODE COMPILE COMMENT: CCLOG(" uv[0]: %.2f %.2f", tmRenderMesh.uv[0][0], tmRenderMesh.uv[0][1]);
					//XCODE COMPILE COMMENT: CCLOG(" uv[1]: %.2f %.2f", tmRenderMesh.uv[1][0], tmRenderMesh.uv[1][1]);
					//XCODE COMPILE COMMENT: CCLOG(" uv[2]: %.2f %.2f", tmRenderMesh.uv[2][0], tmRenderMesh.uv[2][1]);

					Vec3 clip[4];
					Vec3 pos[3] = {
						Vec3(tmRenderMesh.position[0][0], tmRenderMesh.position[0][1], 0.f),
						Vec3(tmRenderMesh.position[1][0], tmRenderMesh.position[1][1], 0.f),
						Vec3(tmRenderMesh.position[2][0], tmRenderMesh.position[2][1], 0.f) };

					transform.transformPoint(&pos[0]);
					transform.transformPoint(&pos[1]);
					transform.transformPoint(&pos[2]);

					calc_uv_clip_vectors(
						Vec2(pos[0].x, pos[0].y),
						Vec2(pos[1].x, pos[1].y),
						Vec2(pos[2].x, pos[2].y),
						Vec2(tmRenderMesh.uv[0][0], tmRenderMesh.uv[0][1]),
						Vec2(tmRenderMesh.uv[1][0], tmRenderMesh.uv[1][1]),
						Vec2(tmRenderMesh.uv[2][0], tmRenderMesh.uv[2][1]),
						clip);

					//
					// setup vertices
					//

					renderData.vertices.resize(renderMesh.vertexCount);

                    ae_uint32_t i;
					const float *p;
					const float *uv;

					for (i = 0, p = &renderMesh.position[0][0], uv = renderMesh.uv[0]; i < renderMesh.vertexCount; i++, p += 3, uv += 2) {
						V3F_C4B_T2F &v = renderData.vertices[i];

						v.vertices = Vec3(p[0], p[1], p[2]);
						v.colors = Color4B(Color4F(renderMesh.color.r, renderMesh.color.g, renderMesh.color.b, renderMesh.opacity));
						v.texCoords = Tex2F(uv[0], uv[1]);

					#if COCOS2D_DEBUG > 0
						//XCODE COMPILE COMMENT: CCLOG(" vertex %i:", i);
						//XCODE COMPILE COMMENT: CCLOG("  position = %.2f %.2f %.2f", p[0], p[1], p[2]);
						//XCODE COMPILE COMMENT: CCLOG("  uv       = %.2f %.2f", uv[0], uv[1], uv[2]);
						//XCODE COMPILE COMMENT: CCLOG("  color    = %.2f %.2f %.2f %.2f", renderMesh.r, renderMesh.g, renderMesh.b, renderMesh.opacity);

						float a = clip[0].dot(Vec3(1.f, v.vertices.x, v.vertices.y));
						float b = clip[1].dot(Vec3(1.f, v.vertices.x, v.vertices.y));
						Vec2 tmUV(
							clip[2].dot(Vec3(1.f, a, b)),
							clip[3].dot(Vec3(1.f, a, b)));

						//XCODE COMPILE COMMENT: CCLOG("  tmUV     = %.2f %.2f", tmUV.x, tmUV.y);
					#endif
					}

					TrianglesCommand::Triangles triangles;
					triangles.verts = renderData.vertices.data();
					triangles.vertCount = renderMesh.vertexCount;

					//
					// setup indices
					//

					renderData.indices.assign(renderMesh.indices, renderMesh.indices + renderMesh.indexCount);

					triangles.indices = renderData.indices.data();
					triangles.indexCount = renderMesh.indexCount;

					#if COCOS2D_DEBUG > 0
					for (i = 0; i < triangles.indexCount; i++) {
						//XCODE COMPILE COMMENT: CCLOG(" index %i = %i", i, triangles.indices[i]);
					}
					#endif

					BlendFunc blendFunc;

					switch(renderMesh.blend_mode) {
						case AE_MOVIE_BLEND_ADD:
							blendFunc.src = GL_SRC_ALPHA;
							blendFunc.dst = GL_ONE;
							break;
//						case AE_MOVIE_BLEND_NORMAL:
						default:
							blendFunc.src = GL_SRC_ALPHA;
							blendFunc.dst = GL_ONE_MINUS_SRC_ALPHA;
					}

//					//XCODE COMPILE COMMENT: CCLOG(" resource type: %i", (int)renderMesh.resource_type);
//					//XCODE COMPILE COMMENT: CCLOG(" resource data: %i", (int)renderMesh.resource_data);

					Texture2D * texture = static_cast<Texture2D *>(renderMesh.resource_data);
					Texture2D * tmTexture = static_cast<Texture2D *>(tmRenderMesh.resource_data);

					// don't do this because cocos binds the trianglesCommand.texture as CC_Texture0
//					_trackMatteGPS->setUniformTexture("u_texture", texture);
					_trackMatteGPS->setUniformTexture("u_tmTexture", tmTexture);
//					_trackMatteGPS->setVertexAttribPointer("a_tmTexCoord", 2, GL_FLOAT, GL_FALSE, 0, );

					_trackMatteGPS->setUniformVec3("u_clip0", clip[0]);
					_trackMatteGPS->setUniformVec3("u_clip1", clip[1]);
					_trackMatteGPS->setUniformVec3("u_clip2", clip[2]);
					_trackMatteGPS->setUniformVec3("u_clip3", clip[3]);

					setGLProgramState(_trackMatteGPS);

					renderData.trianglesCommand.init(
						_globalZOrder,
						texture,
						_glProgramState,
						blendFunc,
						triangles,
						transform,
						flags);

					renderer->addCommand(&renderData.trianglesCommand);

					break;
				}
			}
		}

		//XCODE COMPILE COMMENT: CCLOG("");
	}

	//XCODE COMPILE COMMENT: CCLOG("AEMovie::Draw(): END");
}

NS_CC_EXT_END;
