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

#include "movie/movie.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

ae_void_t EX_LOG( const char * _format, ... );

//
// Working structure of our application.
//

typedef struct
{
    //
    // Movie parameters.
    //

    ae_char_t license[41];
    ae_char_t license_padding[3];
    ae_char_t movie_path[256];
    ae_char_t comp_name[128];

    //
    // Current working objects
    //

    const aeMovieInstance * instance;
    aeMovieData * data;
    const aeMovieComposition * composition;

    //
    // Callbacks for data loading & setup.
    //

    ae_movie_data_callback_resource_provider_t resource_provider;
    ae_movie_data_callback_resource_deleter_t resource_deleter;
    aeMovieCompositionProviders comp_providers;
} examples_t;

extern examples_t ex;

//
// Platform-dependent functions.
//

ae_float_t ex_get_time( ae_void_t );

//
// Callbacks for loading movie resource data (images, videos, sounds etc.)
// This data might be shared among different compositions.
//

ae_bool_t ex_callback_resource_provider_empty( const aeMovieResource * _resource, ae_userdataptr_t _rd, ae_userdata_t _ud );
ae_void_t ex_callback_resource_deleter_empty( aeMovieResourceTypeEnum _type, ae_userdata_t _rd, ae_userdata_t _ud );
ae_bool_t ex_callback_resource_provider( const aeMovieResource * _resource, ae_userdataptr_t _rd, ae_userdata_t _ud );

//
// Callbacks for loading composition data & managing it.
//

ae_bool_t ex_callback_camera_provider( const aeMovieCameraProviderCallbackData * _callbackData, ae_userdataptr_t _cd, ae_userdata_t _ud );
ae_bool_t ex_callback_node_provider( const aeMovieNodeProviderCallbackData * _callbackData, ae_userdataptr_t _nd, ae_userdata_t _ud );
ae_void_t ex_callback_node_destroyer( const aeMovieNodeDeleterCallbackData * _callbackData, ae_userdata_t _ud );
ae_void_t ex_callback_node_update( const aeMovieNodeUpdateCallbackData * _callbackData, ae_userdata_t _ud );
ae_bool_t ex_callback_track_matte_provider( const aeMovieTrackMatteProviderCallbackData * _callbackData, ae_userdataptr_t _tmd, ae_userdata_t _ud );
ae_void_t ex_callback_track_matte_deleter( const aeMovieTrackMatteDeleterCallbackData * _callbackData, ae_userdata_t _ud );
ae_void_t ex_callback_track_matte_update( const aeMovieTrackMatteUpdateCallbackData * _callbackData, ae_userdata_t _ud );
ae_void_t ex_callback_composition_event( const aeMovieCompositionEventCallbackData * _callbackData, ae_userdata_t _ud );
ae_void_t ex_callback_composition_state( const aeMovieCompositionStateCallbackData * _callbackData, ae_userdata_t _ud );

//
// Miscellaneous & shared functions.
//

ae_void_t ex_load_movie_data( ae_void_t );
ae_void_t ex_set_composition( ae_void_t );
ae_void_t ex_update( ae_float_t dt );
ae_void_t ex_render( ae_void_t );

ae_void_t ex_init( const ae_char_t * license, const ae_char_t * path, const ae_char_t * composition );
ae_void_t ex_shutdown( ae_void_t );
