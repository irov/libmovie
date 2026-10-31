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

#include "moviec_internal.h"

#include <string.h>

//////////////////////////////////////////////////////////////////////////
typedef struct MoviecProperty
{
    size_t dimension;
    const char * suffixes[3];
    uint32_t properties[3];
    float fallback;
} MoviecProperty;
//////////////////////////////////////////////////////////////////////////
static MoviecProperty moviec_property( MoviecContext * _context, const char * _name )
{
    static const struct
    {
        const char * name;
        MoviecProperty property;
    } PROPERTIES[] = {
        { "position", { 2, { ".x", ".y" }, { AE_MOTION_X, AE_MOTION_Y }, 0.f } },
        { "anchor", { 2, { ".x", ".y" }, { AE_MOTION_ANCHOR_X, AE_MOTION_ANCHOR_Y }, 0.f } },
        { "scale", { 2, { ".x", ".y" }, { AE_MOTION_SCALE_X, AE_MOTION_SCALE_Y }, 1.f } },
        { "color", { 3, { ".r", ".g", ".b" }, { AE_MOTION_RED, AE_MOTION_GREEN, AE_MOTION_BLUE }, 1.f } },
        { "x", { 1, { "" }, { AE_MOTION_X }, 0.f } },
        { "y", { 1, { "" }, { AE_MOTION_Y }, 0.f } },
        { "position.x", { 1, { "" }, { AE_MOTION_X }, 0.f } },
        { "position.y", { 1, { "" }, { AE_MOTION_Y }, 0.f } },
        { "anchor.x", { 1, { "" }, { AE_MOTION_ANCHOR_X }, 0.f } },
        { "anchor.y", { 1, { "" }, { AE_MOTION_ANCHOR_Y }, 0.f } },
        { "scale.x", { 1, { "" }, { AE_MOTION_SCALE_X }, 1.f } },
        { "scale.y", { 1, { "" }, { AE_MOTION_SCALE_Y }, 1.f } },
        { "rotation", { 1, { "" }, { AE_MOTION_ROTATION }, 0.f } },
        { "skew", { 1, { "" }, { AE_MOTION_SKEW }, 0.f } },
        { "opacity", { 1, { "" }, { AE_MOTION_OPACITY }, 1.f } },
        { "volume", { 1, { "" }, { AE_MOTION_VOLUME }, 1.f } },
        { "pathProgress", { 1, { "" }, { AE_MOTION_PATH }, 0.f } } };

    for( size_t index = 0; index != sizeof( PROPERTIES ) / sizeof( PROPERTIES[0] ); ++index )
    {
        if( strcmp( PROPERTIES[index].name, _name ) != 0 )
        {
            continue;
        }

        MoviecProperty result = PROPERTIES[index].property;

        return result;
    }

    moviec_require( _context, strncmp( _name, "custom.", 7 ) == 0, "unknown property: %s", _name );
    MoviecProperty result = { 1, { "" }, { AE_MOTION_CUSTOM }, 0.f };

    return result;
}
//////////////////////////////////////////////////////////////////////////
uint32_t moviec_resource_type( MoviecContext * _context, json_t * _resource )
{
    static const char * const NAMES[] = { "image", "sound", "particle", "video", "sequence", "solid" };
    static const uint32_t TYPES[] = { AE_MOVIE_RESOURCE_IMAGE, AE_MOVIE_RESOURCE_SOUND, AE_MOVIE_RESOURCE_PARTICLE, AE_MOVIE_RESOURCE_VIDEO, AE_MOVIE_RESOURCE_SEQUENCE, AE_MOVIE_RESOURCE_SOLID };
    moviec_members( _context, _resource, "type path size textureSize uv trim color duration fps images alpha" );
    const char * name = moviec_text( moviec_get( _resource, "type" ), "image" );

    for( size_t index = 0; index != 6; ++index )
    {
        if( strcmp( name, NAMES[index] ) != 0 )
        {
            continue;
        }

        uint32_t result = TYPES[index];

        return result;
    }

    moviec_fail( _context, "unsupported resource type: %s", name );
}
//////////////////////////////////////////////////////////////////////////
float moviec_effective_fps( MoviecContext * _context, json_t * _composition )
{
    float fps = moviec_number( _context, moviec_get( _composition, "fps" ), 30.f );
    json_t * layers = moviec_get( _composition, "layers" );
    static const char * const TYPES[] = { "mesh", "warp" };

    for( size_t index = 0; index != json_array_size( layers ); ++index )
    {
        for( size_t type = 0; type != 2; ++type )
        {
            json_t * geometry = moviec_get( json_array_get( layers, index ), TYPES[type] );

            if( json_array_size( moviec_get( geometry, "keys" ) ) == 0 )
            {
                continue;
            }

            float baking = moviec_number( _context, moviec_get( geometry, "bakeFps" ), 60.f );
            moviec_require( _context, baking >= 1.f && baking <= 240.f, "invalid geometry baking rate" );

            if( fps < baking )
            {
                fps = baking;
            }
        }
    }

    moviec_require( _context, fps >= 1.f && fps <= 240.f, "invalid baking frame rate" );

    return fps;
}
//////////////////////////////////////////////////////////////////////////
static void moviec_collect_parameters( MoviecContext * _context )
{
    json_t * defaults = moviec_get( _context->document, "parameters" );

    if( defaults == NULL )
    {
        defaults = moviec_object( _context );
        moviec_json_set( _context, _context->document, "parameters", defaults );
    }

    moviec_require( _context, json_is_object( defaults ), "parameters must be an object" );

    for( uint32_t index = 0; index != AE_MOVIE_MOTION_PARAMETER_COUNT; ++index )
    {
        const char * name = ae_get_movie_motion_parameter_name( (aeMovieMotionParameter)index );

        if( moviec_get( defaults, name ) == NULL )
        {
            float value = ae_get_movie_motion_parameter_default( (aeMovieMotionParameter)index );
            moviec_json_number( _context, defaults, name, value );
        }

        moviec_json_integer( _context, _context->parameters, name, (json_int_t)json_array_size( _context->parameterNames ) );
        moviec_json_append_string( _context, _context->parameterNames, name );
    }

    json_t * registers = moviec_get( _context->document, "registers" );
    moviec_require( _context, registers == NULL || json_is_array( registers ), "registers must be an array" );

    for( size_t index = 0; index != json_array_size( registers ); ++index )
    {
        json_t * entry = json_array_get( registers, index );
        moviec_members( _context, entry, "name value" );
        const char * name = moviec_text( moviec_get( entry, "name" ), "" );
        moviec_require( _context, name[0] != '\0', "register name must not be empty" );
        moviec_require( _context, moviec_get( defaults, name ) == NULL, "duplicate or reserved register name: %s", name );
        float value = moviec_number( _context, moviec_get( entry, "value" ), 0.f );
        moviec_json_number( _context, defaults, name, value );
        moviec_json_integer( _context, _context->parameters, name, (json_int_t)json_array_size( _context->parameterNames ) );
        moviec_json_append_string( _context, _context->parameterNames, name );
    }

    json_t * names = moviec_keys( _context, defaults );

    for( size_t index = 0; index != json_array_size( names ); ++index )
    {
        const char * name = json_string_value( json_array_get( names, index ) );
        moviec_number( _context, moviec_get( defaults, name ), 0.f );

        if( moviec_get( _context->parameters, name ) != NULL )
        {
            continue;
        }

        moviec_json_integer( _context, _context->parameters, name, (json_int_t)json_array_size( _context->parameterNames ) );
        moviec_json_append_string( _context, _context->parameterNames, name );
    }
}
//////////////////////////////////////////////////////////////////////////
static float moviec_component( MoviecContext * _context, json_t * _value, size_t _dimension, size_t _component )
{
    float values[4];
    moviec_vector( _context, _value, _dimension, 0.f, values );
    float result = values[_component];

    return result;
}
//////////////////////////////////////////////////////////////////////////
static void moviec_collect_keys( MoviecContext * _context, MoviecTrack * _track, json_t * _channel, float _duration )
{
    json_t * keys = moviec_get( _channel, "keys" );
    moviec_require( _context, keys == NULL || json_is_array( keys ), "keys must be an array" );
    _track->keyCount = json_array_size( keys );
    _track->keys = moviec_allocate( _context, _track->keyCount * sizeof( aeMotionKey ) );

    for( size_t index = 0; index != _track->keyCount; ++index )
    {
        json_t * key = json_array_get( keys, index );
        moviec_members( _context, key, "time value interpolation in out" );
        moviec_needs( _context, key, "time value" );
        aeMotionKey * compiled = _track->keys + index;
        compiled->time = moviec_number( _context, moviec_get( key, "time" ), 0.f );
        compiled->value = moviec_component( _context, moviec_get( key, "value" ), _track->dimension, _track->component );
        moviec_require( _context, compiled->time >= 0.f && compiled->time <= _duration, "key time must be within duration" );
        moviec_require( _context, index == 0 || compiled->time > _track->keys[index - 1].time, "keys must have strictly increasing times" );
        const char * interpolation = moviec_text( moviec_get( key, "interpolation" ), "linear" );
        moviec_require( _context, strcmp( interpolation, "hold" ) == 0 || strcmp( interpolation, "linear" ) == 0 || strcmp( interpolation, "bezier" ) == 0 || strcmp( interpolation, "ease_in" ) == 0 || strcmp( interpolation, "ease_out" ) == 0 || strcmp( interpolation, "ease_in_out" ) == 0, "unknown interpolation: %s", interpolation );
        compiled->interpolation = strcmp( interpolation, "hold" ) == 0 ? 0 : strcmp( interpolation, "linear" ) == 0 ? 1
                                                                                                                    : 2;
        compiled->in_time = compiled->out_time = compiled->time;
        compiled->in_value = compiled->out_value = compiled->value;
    }

    for( size_t index = 1; index < _track->keyCount; ++index )
    {
        aeMotionKey * previous = _track->keys + index - 1;
        aeMotionKey * next = _track->keys + index;
        float third = ( next->time - previous->time ) / 3.f;
        json_t * previousKey = json_array_get( keys, index - 1 );
        json_t * nextKey = json_array_get( keys, index );
        const char * easing = moviec_text( moviec_get( previousKey, "interpolation" ), "" );
        previous->out_time = previous->time + third;
        next->in_time = next->time - third;
        previous->out_value = previous->value + ( next->value - previous->value ) / 3.f;
        next->in_value = next->value - ( next->value - previous->value ) / 3.f;

        if( strcmp( easing, "ease_in" ) == 0 || strcmp( easing, "ease_in_out" ) == 0 )
        {
            previous->out_value = previous->value;
        }

        if( strcmp( easing, "ease_out" ) == 0 || strcmp( easing, "ease_in_out" ) == 0 )
        {
            next->in_value = next->value;
        }

        json_t * out = moviec_get( previousKey, "out" );

        if( out != NULL )
        {
            moviec_members( _context, out, "time value" );
            moviec_needs( _context, out, "time value" );
            previous->out_time = moviec_number( _context, moviec_get( out, "time" ), 0.f );
            previous->out_value = moviec_component( _context, moviec_get( out, "value" ), _track->dimension, _track->component );
        }

        json_t * in = moviec_get( nextKey, "in" );

        if( in != NULL )
        {
            moviec_members( _context, in, "time value" );
            moviec_needs( _context, in, "time value" );
            next->in_time = moviec_number( _context, moviec_get( in, "time" ), 0.f );
            next->in_value = moviec_component( _context, moviec_get( in, "value" ), _track->dimension, _track->component );
        }

        moviec_require( _context, previous->out_time >= previous->time && previous->out_time <= next->time && next->in_time >= previous->time && next->in_time <= next->time, "Bezier time handles must stay within their segment" );
    }

    json_t * samples = moviec_get( _channel, "samples" );

    if( samples == NULL )
    {
        return;
    }

    moviec_require( _context, _track->keyCount == 0 && json_is_array( samples ) && json_array_size( samples ) > 0, "samples require a nonempty array and cannot be combined with keys" );
    float rate = moviec_number( _context, moviec_get( _channel, "sampleRate" ), 60.f );
    float start = moviec_number( _context, moviec_get( _channel, "sampleStart" ), 0.f );
    moviec_require( _context, rate > 0.f && rate <= 240.f && start >= 0.f, "invalid sample timing" );
    _track->keyCount = json_array_size( samples );
    _track->keys = moviec_allocate( _context, _track->keyCount * sizeof( aeMotionKey ) );

    for( size_t index = 0; index != _track->keyCount; ++index )
    {
        aeMotionKey * key = _track->keys + index;
        key->time = start + (float)index / rate;
        moviec_require( _context, key->time <= _duration + .00001f, "samples extend beyond composition duration" );
        key->value = moviec_component( _context, json_array_get( samples, index ), _track->dimension, _track->component );
        key->interpolation = 1;
    }
}
//////////////////////////////////////////////////////////////////////////
static void moviec_collect_channels( MoviecContext * _context, json_t * _layer, uint32_t _compositionIndex, uint32_t _layerIndex, const char * _compositionId, float _duration )
{
    const char * layerId = moviec_text( moviec_get( _layer, "id" ), "" );
    json_t * properties = moviec_get( _layer, "channels" );
    json_t * names = moviec_keys( _context, properties );
    unsigned char occupied[AE_MOTION_CUSTOM + 1] = { 0 };

    for( size_t propertyIndex = 0; propertyIndex != json_array_size( names ); ++propertyIndex )
    {
        const char * name = json_string_value( json_array_get( names, propertyIndex ) );
        _context->path = moviec_format( _context, "/compositions/%u/layers/%u/channels/%s", _compositionIndex, _layerIndex, name );
        MoviecProperty property = moviec_property( _context, name );
        json_t * channel = moviec_get( properties, name );
        json_t * channelValue = json_is_object( channel ) ? moviec_get( channel, "value" ) : channel;

        if( json_is_object( channel ) )
        {
            moviec_members( _context, channel, "value keys expression samples sampleRate sampleStart extrapolation" );
            json_t * expression = moviec_get( channel, "expression" );
            moviec_require( _context, expression == NULL || json_is_string( expression ), "expression must be a string" );
        }

        float values[4];
        moviec_vector( _context, channelValue, property.dimension, property.fallback, values );
        json_t * indices = moviec_array( _context );
        size_t first = _context->trackCount;

        for( size_t component = 0; component != property.dimension; ++component )
        {
            uint32_t type = property.properties[component];
            moviec_require( _context, type == AE_MOTION_CUSTOM || occupied[type] == 0, "property is assigned more than once: %s", name );
            occupied[type] = 1;

            if( _context->trackCount == _context->trackCapacity )
            {
                size_t capacity = ( _context->trackCount + 1 ) * 2;
                _context->tracks = moviec_grow( _context, _context->tracks, _context->trackCount * sizeof( MoviecTrack ), capacity * sizeof( MoviecTrack ) );
                _context->trackCapacity = capacity;
            }

            MoviecTrack * track = _context->tracks + _context->trackCount;
            track->name = moviec_format( _context, "%s.%s.%s%s", _compositionId, layerId, name, property.suffixes[component] );
            track->composition = _compositionIndex;
            track->layer = _layerIndex;
            track->property = type;
            track->value = values[component];
            track->dimension = property.dimension;
            track->component = component;
            const char * extrapolation = moviec_text( moviec_get( channel, "extrapolation" ), "hold" );
            moviec_require( _context, strcmp( extrapolation, "hold" ) == 0 || strcmp( extrapolation, "cycle" ) == 0 || strcmp( extrapolation, "pingpong" ) == 0, "unknown extrapolation" );
            track->extrapolation = strcmp( extrapolation, "cycle" ) == 0 ? 1 : strcmp( extrapolation, "pingpong" ) == 0 ? 2
                                                                                                                        : 0;
            track->expression = moviec_text( moviec_get( channel, "expression" ), "" );
            moviec_collect_keys( _context, track, channel, _duration );
            uint32_t index = (uint32_t)_context->trackCount++;
            moviec_json_append_integer( _context, indices, index );
            moviec_require( _context, moviec_get( _context->channels, track->name ) == NULL, "duplicate channel: %s", track->name );
            json_t * scalar = moviec_array( _context );
            moviec_json_append_integer( _context, scalar, index );
            moviec_json_set( _context, _context->channels, track->name, scalar );
            moviec_json_string( _context, _context->sourceMap, track->name, _context->path );
        }

        for( size_t index = first; index != _context->trackCount; ++index )
        {
            for( size_t component = 0; component != property.dimension; ++component )
            {
                _context->tracks[index].base[component] = (uint32_t)( first + component );
            }
        }

        if( property.dimension > 1 )
        {
            const char * channelName = moviec_format( _context, "%s.%s.%s", _compositionId, layerId, name );
            moviec_require( _context, moviec_get( _context->channels, channelName ) == NULL, "duplicate vector channel: %s", channelName );
            moviec_json_set( _context, _context->channels, channelName, indices );
        }
    }
}
//////////////////////////////////////////////////////////////////////////
static void moviec_visit_composition( MoviecContext * _context, uint32_t _index, unsigned _depth, unsigned char * _visited )
{
    moviec_require( _context, _depth <= 32, "composition nesting exceeds 32" );
    moviec_require( _context, _visited[_index] != 1, "composition cycle" );

    if( _visited[_index] == 2 )
    {
        return;
    }

    _visited[_index] = 1;
    json_t * composition = json_array_get( moviec_get( _context->document, "compositions" ), _index );
    json_t * layers = moviec_get( composition, "layers" );

    for( size_t index = 0; index != json_array_size( layers ); ++index )
    {
        const char * child = moviec_text( moviec_get( json_array_get( layers, index ), "composition" ), "" );

        if( child[0] == '\0' )
        {
            continue;
        }

        uint32_t childIndex = moviec_map_index( _context, _context->compositions, child );
        moviec_visit_composition( _context, childIndex, _depth + 1, _visited );
    }

    _visited[_index] = 2;
}
//////////////////////////////////////////////////////////////////////////
static void moviec_visit_track( MoviecContext * _context, uint32_t _index, unsigned _depth, unsigned char * _visited, uint32_t * _order, size_t * _count )
{
    moviec_require( _context, _depth <= 64, "channel dependency depth exceeds 64" );
    moviec_require( _context, _visited[_index] != 1, "channel dependency cycle: %s", _context->tracks[_index].name );

    if( _visited[_index] == 2 )
    {
        return;
    }

    _visited[_index] = 1;
    const MoviecCode * code = &_context->tracks[_index].code;

    for( size_t index = 0; index != code->count; ++index )
    {
        if( code->instructions[index].opcode != AE_MOTION_REF )
        {
            continue;
        }

        moviec_visit_track( _context, code->instructions[index].index, _depth + 1, _visited, _order, _count );
    }

    _visited[_index] = 2;
    _order[( *_count )++] = _index;
}
//////////////////////////////////////////////////////////////////////////
static void moviec_order_tracks( MoviecContext * _context )
{
    for( size_t index = 0; index != _context->trackCount; ++index )
    {
        MoviecTrack * track = _context->tracks + index;

        if( track->expression[0] == '\0' )
        {
            continue;
        }

        _context->path = moviec_text( moviec_get( _context->sourceMap, track->name ), "/" );
        moviec_compile_expression( _context, track );
    }

    unsigned char * visited = moviec_allocate( _context, _context->trackCount );
    uint32_t * order = moviec_allocate( _context, _context->trackCount * sizeof( uint32_t ) );
    uint32_t * remap = moviec_allocate( _context, _context->trackCount * sizeof( uint32_t ) );
    MoviecTrack * tracks = moviec_allocate( _context, _context->trackCount * sizeof( MoviecTrack ) );
    size_t count = 0;

    for( uint32_t index = 0; index != _context->trackCount; ++index )
    {
        moviec_visit_track( _context, index, 0, visited, order, &count );
    }

    for( uint32_t index = 0; index != _context->trackCount; ++index )
    {
        remap[order[index]] = index;
        tracks[index] = _context->tracks[order[index]];
    }

    _context->tracks = tracks;

    for( size_t index = 0; index != _context->trackCount; ++index )
    {
        MoviecTrack * track = tracks + index;

        for( size_t instructionIndex = 0; instructionIndex != track->code.count; ++instructionIndex )
        {
            aeMotionInstruction * instruction = track->code.instructions + instructionIndex;

            if( instruction->opcode != AE_MOTION_REF && instruction->opcode != AE_MOTION_BASE_REF )
            {
                continue;
            }

            instruction->index = remap[instruction->index];
        }
    }

    const char * name;
    json_t * indices;
    json_object_foreach( _context->channels, name, indices )
    {
        (void)name;

        for( size_t index = 0; index != json_array_size( indices ); ++index )
        {
            uint32_t original = (uint32_t)json_integer_value( json_array_get( indices, index ) );
            moviec_require( _context, json_array_set_new( indices, index, json_integer( remap[original] ) ) == 0, "cannot remap channel" );
        }
    }
}
//////////////////////////////////////////////////////////////////////////
void moviec_collect( MoviecContext * _context )
{
    moviec_members( _context, _context->document, "$schema version name parameters registers resources compositions" );
    moviec_needs( _context, _context->document, "version name compositions" );
    moviec_require( _context, moviec_number( _context, moviec_get( _context->document, "version" ), 0.f ) == 1.f, "unsupported JSON version" );
    _context->parameters = moviec_object( _context );
    _context->parameterNames = moviec_array( _context );
    _context->resources = moviec_object( _context );
    _context->resourceNames = moviec_array( _context );
    _context->compositions = moviec_object( _context );
    _context->channels = moviec_object( _context );
    _context->manifest = moviec_object( _context );
    _context->sourceMap = moviec_object( _context );
    moviec_collect_parameters( _context );
    json_t * assets = moviec_keys( _context, moviec_get( _context->document, "resources" ) );

    for( unsigned pass = 0; pass != 2; ++pass )
    {
        for( size_t index = 0; index != json_array_size( assets ); ++index )
        {
            const char * name = json_string_value( json_array_get( assets, index ) );
            uint32_t type = moviec_resource_type( _context, moviec_get( moviec_get( _context->document, "resources" ), name ) );

            if( ( type == AE_MOVIE_RESOURCE_IMAGE ) != ( pass == 0 ) )
            {
                continue;
            }

            moviec_json_integer( _context, _context->resources, name, (json_int_t)json_array_size( _context->resourceNames ) );
            moviec_json_append_string( _context, _context->resourceNames, name );
        }
    }

    json_t * list = moviec_get( _context->document, "compositions" );
    size_t count = json_array_size( list );
    moviec_require( _context, json_is_array( list ) && count > 0 && count <= 1024, "compositions must be a nonempty array (at most 1024)" );

    for( size_t index = 0; index != count; ++index )
    {
        json_t * composition = json_array_get( list, index );
        moviec_members( _context, composition, "id size duration fps layers markers" );
        moviec_needs( _context, composition, "id size duration layers" );
        const char * id = moviec_text( moviec_get( composition, "id" ), "" );
        moviec_require( _context, id[0] != '\0' && strlen( id ) <= 127, "composition needs an id (at most 127 bytes)" );
        moviec_require( _context, moviec_get( _context->compositions, id ) == NULL, "duplicate composition id: %s", id );
        moviec_json_integer( _context, _context->compositions, id, (json_int_t)index );
    }

    for( uint32_t compositionIndex = 0; compositionIndex != count; ++compositionIndex )
    {
        json_t * composition = json_array_get( list, compositionIndex );
        const char * id = moviec_text( moviec_get( composition, "id" ), "" );
        _context->path = moviec_format( _context, "/compositions/%u", compositionIndex );
        float duration = moviec_number( _context, moviec_get( composition, "duration" ), 0.f );
        float fps = moviec_number( _context, moviec_get( composition, "fps" ), 30.f );
        moviec_require( _context, duration > 0.f && duration <= 3600.f && fps >= 1.f && fps <= 240.f, "invalid duration or fps" );
        float dimensions[2];
        moviec_vector( _context, moviec_get( composition, "size" ), 2, 0.f, dimensions );
        moviec_require( _context, dimensions[0] > 0.f && dimensions[1] > 0.f, "composition needs positive size" );
        json_t * layers = moviec_get( composition, "layers" );
        moviec_require( _context, json_is_array( layers ) && json_array_size( layers ) <= 4096, "layers must be an array (at most 4096)" );
        json_t * layerIds = moviec_object( _context );

        for( size_t layerIndex = 0; layerIndex != json_array_size( layers ); ++layerIndex )
        {
            json_t * layer = json_array_get( layers, layerIndex );
            moviec_members( _context, layer, "id resource composition type parent in out loop blend isMatte matte matteMode channels path mesh warp" );
            const char * layerId = moviec_text( moviec_get( layer, "id" ), "" );
            moviec_require( _context, layerId[0] != '\0' && strlen( layerId ) <= 127 && moviec_get( layerIds, layerId ) == NULL, "missing or duplicate layer id: %s", layerId );
            moviec_json_integer( _context, layerIds, layerId, (json_int_t)layerIndex );
        }

        for( uint32_t layerIndex = 0; layerIndex != json_array_size( layers ); ++layerIndex )
        {
            json_t * layer = json_array_get( layers, layerIndex );
            const char * layerId = moviec_text( moviec_get( layer, "id" ), "" );
            _context->path = moviec_format( _context, "/compositions/%u/layers/%u", compositionIndex, layerIndex );
            const char * asset = moviec_text( moviec_get( layer, "resource" ), "" );
            const char * child = moviec_text( moviec_get( layer, "composition" ), "" );
            moviec_require( _context, asset[0] == '\0' || moviec_get( _context->resources, asset ) != NULL, "unknown resource: %s", asset );
            moviec_require( _context, child[0] == '\0' || moviec_get( _context->compositions, child ) != NULL, "unknown composition: %s", child );
            moviec_require( _context, asset[0] == '\0' || child[0] == '\0', "layer cannot be both resource and composition" );
            const char * parent = moviec_text( moviec_get( layer, "parent" ), "" );
            size_t parentCount = 0;

            while( parent[0] != '\0' )
            {
                moviec_require( _context, strcmp( parent, layerId ) != 0 && ++parentCount <= json_array_size( layers ), "parent cycle" );
                uint32_t parentIndex = moviec_map_index( _context, layerIds, parent );
                parent = moviec_text( moviec_get( json_array_get( layers, parentIndex ), "parent" ), "" );
            }

            float in = moviec_number( _context, moviec_get( layer, "in" ), 0.f );
            float out = moviec_number( _context, moviec_get( layer, "out" ), duration );
            moviec_require( _context, in >= 0.f && out > in && out <= duration, "invalid layer in/out interval" );
            moviec_collect_channels( _context, layer, compositionIndex, layerIndex, id, duration );
        }
    }

    unsigned char * visited = moviec_allocate( _context, count );

    for( uint32_t index = 0; index != count; ++index )
    {
        moviec_visit_composition( _context, index, 0, visited );
    }

    moviec_order_tracks( _context );
}
