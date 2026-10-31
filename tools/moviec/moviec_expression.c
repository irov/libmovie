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

#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

//////////////////////////////////////////////////////////////////////////
typedef struct MoviecValue
{
    size_t dimension;
    MoviecCode components[4];
} MoviecValue;
//////////////////////////////////////////////////////////////////////////
typedef struct MoviecExpression
{
    MoviecContext * context;
    const MoviecTrack * track;
    const char * source;
    const char * token;
    size_t position;
    unsigned depth;
    unsigned conditionalDepth;
} MoviecExpression;
//////////////////////////////////////////////////////////////////////////
static void moviec_code_append( MoviecContext * _context, MoviecCode * _code, const aeMotionInstruction * _instructions, size_t _count )
{
    moviec_require( _context, _code->count + _count <= 4096, "expression is too complex" );

    if( _code->count + _count > _code->capacity )
    {
        size_t capacity = ( _code->count + _count ) * 2;
        _code->instructions = moviec_grow( _context, _code->instructions, _code->count * sizeof( aeMotionInstruction ), capacity * sizeof( aeMotionInstruction ) );
        _code->capacity = capacity;
    }

    memcpy( _code->instructions + _code->count, _instructions, _count * sizeof( aeMotionInstruction ) );
    _code->count += _count;
}
//////////////////////////////////////////////////////////////////////////
static MoviecValue moviec_expression_leaf( MoviecExpression * _expression, uint32_t _opcode, uint32_t _index, float _value )
{
    MoviecValue result = { 0 };
    aeMotionInstruction instruction = { _opcode, _index, _value };
    result.dimension = 1;
    moviec_code_append( _expression->context, result.components, &instruction, 1 );

    return result;
}
//////////////////////////////////////////////////////////////////////////
static MoviecValue moviec_expression_operation( MoviecExpression * _expression, uint32_t _opcode, const MoviecValue * _arguments, size_t _count )
{
    MoviecValue result = { 0 };
    result.dimension = 1;

    for( size_t index = 0; index != _count; ++index )
    {
        if( result.dimension < _arguments[index].dimension )
        {
            result.dimension = _arguments[index].dimension;
        }
    }

    for( size_t index = 0; index != _count; ++index )
    {
        const MoviecValue * value = _arguments + index;
        moviec_require( _expression->context, value->dimension == 1 || value->dimension == result.dimension, "incompatible expression vector dimensions" );

        for( size_t component = 0; component != result.dimension; ++component )
        {
            const MoviecCode * code = value->components + ( value->dimension == 1 ? 0 : component );
            moviec_code_append( _expression->context, result.components + component, code->instructions, code->count );
        }
    }

    aeMotionInstruction instruction = { _opcode, 0, 0.f };

    for( size_t component = 0; component != result.dimension; ++component )
    {
        moviec_code_append( _expression->context, result.components + component, &instruction, 1 );
    }

    return result;
}
//////////////////////////////////////////////////////////////////////////
static void moviec_expression_next( MoviecExpression * _expression )
{
    const char * source = _expression->source;

    while( isspace( (unsigned char)source[_expression->position] ) != 0 )
    {
        ++_expression->position;
    }

    size_t begin = _expression->position;
    char ch = source[_expression->position];

    if( ch == '\0' )
    {
        _expression->token = "";

        return;
    }

    ++_expression->position;

    if( isdigit( (unsigned char)ch ) != 0 || ( ch == '.' && isdigit( (unsigned char)source[_expression->position] ) != 0 ) )
    {
        char * end;
        strtod( source + begin, &end );
        _expression->position = (size_t)( end - source );
    }
    else if( isalpha( (unsigned char)ch ) != 0 || ch == '_' )
    {
        while( isalnum( (unsigned char)source[_expression->position] ) != 0 || source[_expression->position] == '_' || source[_expression->position] == '.' )
        {
            ++_expression->position;
        }
    }
    else if( ch == '"' || ch == '\'' )
    {
        while( source[_expression->position] != '\0' && source[_expression->position] != ch )
        {
            moviec_require( _expression->context, source[_expression->position] != '\\', "escapes in channel names are unsupported" );
            ++_expression->position;
        }

        moviec_require( _expression->context, source[_expression->position] != '\0', "unterminated string" );
        ++_expression->position;
    }
    else if( source[_expression->position] == '=' && ( ch == '<' || ch == '>' || ch == '=' || ch == '!' ) )
    {
        ++_expression->position;
    }

    _expression->token = moviec_string_copy( _expression->context, source + begin, _expression->position - begin );
}
//////////////////////////////////////////////////////////////////////////
static void moviec_expression_expect( MoviecExpression * _expression, const char * _token )
{
    moviec_require( _expression->context, strcmp( _expression->token, _token ) == 0, "expected '%s', got '%s'", _token, _expression->token );
    moviec_expression_next( _expression );
}
//////////////////////////////////////////////////////////////////////////
static MoviecValue moviec_expression_conditional( MoviecExpression * _expression );
static MoviecValue moviec_expression_unary( MoviecExpression * _expression );
//////////////////////////////////////////////////////////////////////////
static MoviecValue moviec_expression_primary( MoviecExpression * _expression )
{
    MoviecContext * context = _expression->context;

    if( strcmp( _expression->token, "(" ) == 0 )
    {
        moviec_expression_next( _expression );
        MoviecValue result = moviec_expression_conditional( _expression );
        moviec_expression_expect( _expression, ")" );

        return result;
    }

    if( strcmp( _expression->token, "[" ) == 0 )
    {
        moviec_expression_next( _expression );
        MoviecValue result = { 0 };

        for( ;; )
        {
            moviec_require( context, result.dimension < 4, "vectors have at most four components" );
            MoviecValue value = moviec_expression_conditional( _expression );
            moviec_require( context, value.dimension == 1, "vector components must be scalars" );
            result.components[result.dimension++] = value.components[0];

            if( strcmp( _expression->token, "," ) != 0 )
            {
                break;
            }

            moviec_expression_next( _expression );
        }

        moviec_expression_expect( _expression, "]" );

        return result;
    }

    const char * name = _expression->token;
    moviec_require( context, name[0] != '\0', "incomplete expression" );
    moviec_expression_next( _expression );

    if( isdigit( (unsigned char)name[0] ) != 0 || name[0] == '.' )
    {
        float value = strtof( name, NULL );
        moviec_require( context, isfinite( value ) != 0, "non-finite literal" );
        MoviecValue result = moviec_expression_leaf( _expression, AE_MOTION_CONST, 0, value );

        return result;
    }

    if( strcmp( name, "pi" ) == 0 || strcmp( name, "tau" ) == 0 )
    {
        float value = strcmp( name, "pi" ) == 0 ? 3.14159265358979323846f : 6.28318530717958647692f;
        MoviecValue result = moviec_expression_leaf( _expression, AE_MOTION_CONST, 0, value );

        return result;
    }

    if( strcmp( name, "time" ) == 0 )
    {
        MoviecValue result = moviec_expression_leaf( _expression, AE_MOTION_TIME, 0, 0.f );

        return result;
    }

    if( strcmp( name, "value" ) == 0 )
    {
        MoviecValue result = { 0 };
        result.dimension = _expression->track->dimension;

        for( size_t component = 0; component != result.dimension; ++component )
        {
            MoviecValue value = moviec_expression_leaf( _expression, AE_MOTION_BASE_REF, _expression->track->base[component], 0.f );
            result.components[component] = value.components[0];
        }

        return result;
    }

    if( strncmp( name, "params.", 7 ) == 0 )
    {
        uint32_t index;
        moviec_require( context, moviec_map_find( context->parameters, name + 7, &index ), "unknown parameter: %s", name );
        MoviecValue result = moviec_expression_leaf( _expression, AE_MOTION_PARAM, index, 0.f );

        return result;
    }

    moviec_expression_expect( _expression, "(" );

    if( strcmp( name, "ref" ) == 0 )
    {
        const char * token = _expression->token;
        size_t length = strlen( token );
        moviec_require( context, length >= 2 && ( token[0] == '"' || token[0] == '\'' ), "ref requires a literal channel name" );
        const char * channel = moviec_string_copy( context, token + 1, length - 2 );
        json_t * indices = moviec_get( context->channels, channel );
        moviec_require( context, indices != NULL, "unknown channel: %s", channel );
        moviec_expression_next( _expression );
        moviec_expression_expect( _expression, ")" );
        MoviecValue result = { 0 };
        result.dimension = json_array_size( indices );

        for( size_t component = 0; component != result.dimension; ++component )
        {
            uint32_t index = (uint32_t)json_integer_value( json_array_get( indices, component ) );
            MoviecValue value = moviec_expression_leaf( _expression, AE_MOTION_REF, index, 0.f );
            result.components[component] = value.components[0];
        }

        return result;
    }

    MoviecValue arguments[4];
    size_t argumentCount = 0;

    if( strcmp( _expression->token, ")" ) != 0 )
    {
        for( ;; )
        {
            moviec_require( context, argumentCount < 4, "too many function arguments" );
            arguments[argumentCount++] = moviec_expression_conditional( _expression );

            if( strcmp( _expression->token, "," ) != 0 )
            {
                break;
            }

            moviec_expression_next( _expression );
        }
    }

    moviec_expression_expect( _expression, ")" );

    static const struct
    {
        const char * name;
        uint32_t opcode;
        size_t arguments;
    } FUNCTIONS[] = {
        { "sin", AE_MOTION_SIN, 1 }, { "cos", AE_MOTION_COS, 1 }, { "abs", AE_MOTION_ABS, 1 }, { "exp", AE_MOTION_EXP, 1 }, { "sqrt", AE_MOTION_SQRT, 1 }, { "min", AE_MOTION_MIN, 2 }, { "max", AE_MOTION_MAX, 2 }, { "pow", AE_MOTION_POW, 2 }, { "clamp", AE_MOTION_CLAMP, 3 }, { "lerp", AE_MOTION_LERP, 3 }, { "smoothstep", AE_MOTION_SMOOTH, 3 }, { "noise", AE_MOTION_NOISE, 2 }, { "wiggle", AE_MOTION_WIGGLE, 4 }, { "spring", AE_MOTION_SPRING, 4 } };

    for( size_t index = 0; index != sizeof( FUNCTIONS ) / sizeof( FUNCTIONS[0] ); ++index )
    {
        if( strcmp( FUNCTIONS[index].name, name ) != 0 )
        {
            continue;
        }

        moviec_require( context, argumentCount == FUNCTIONS[index].arguments, "wrong argument count for %s", name );
        MoviecValue result = moviec_expression_operation( _expression, FUNCTIONS[index].opcode, arguments, argumentCount );

        return result;
    }

    moviec_fail( context, "unknown function: %s", name );
}
//////////////////////////////////////////////////////////////////////////
static MoviecValue moviec_expression_unary( MoviecExpression * _expression )
{
    moviec_require( _expression->context, ++_expression->depth <= 64, "expression nesting exceeds 64" );
    MoviecValue result;

    if( strcmp( _expression->token, "-" ) == 0 )
    {
        moviec_expression_next( _expression );
        MoviecValue argument = moviec_expression_unary( _expression );
        result = moviec_expression_operation( _expression, AE_MOTION_NEG, &argument, 1 );
    }
    else if( strcmp( _expression->token, "+" ) == 0 )
    {
        moviec_expression_next( _expression );
        result = moviec_expression_unary( _expression );
    }
    else
    {
        result = moviec_expression_primary( _expression );
    }

    --_expression->depth;

    return result;
}
//////////////////////////////////////////////////////////////////////////
static MoviecValue moviec_expression_product( MoviecExpression * _expression )
{
    MoviecValue result = moviec_expression_unary( _expression );

    while( strcmp( _expression->token, "*" ) == 0 || strcmp( _expression->token, "/" ) == 0 || strcmp( _expression->token, "%" ) == 0 )
    {
        char token = _expression->token[0];
        uint32_t opcode = token == '*' ? AE_MOTION_MUL : token == '/' ? AE_MOTION_DIV
                                                                      : AE_MOTION_MOD;
        moviec_expression_next( _expression );
        MoviecValue arguments[2] = { result, moviec_expression_unary( _expression ) };
        result = moviec_expression_operation( _expression, opcode, arguments, 2 );
    }

    return result;
}
//////////////////////////////////////////////////////////////////////////
static MoviecValue moviec_expression_sum( MoviecExpression * _expression )
{
    MoviecValue result = moviec_expression_product( _expression );

    while( strcmp( _expression->token, "+" ) == 0 || strcmp( _expression->token, "-" ) == 0 )
    {
        uint32_t opcode = _expression->token[0] == '+' ? AE_MOTION_ADD : AE_MOTION_SUB;
        moviec_expression_next( _expression );
        MoviecValue arguments[2] = { result, moviec_expression_product( _expression ) };
        result = moviec_expression_operation( _expression, opcode, arguments, 2 );
    }

    return result;
}
//////////////////////////////////////////////////////////////////////////
static MoviecValue moviec_expression_comparison( MoviecExpression * _expression )
{
    static const char * const TOKENS[] = { "<", "<=", ">", ">=", "==", "!=" };
    static const uint32_t OPCODES[] = { AE_MOTION_LT, AE_MOTION_LE, AE_MOTION_GT, AE_MOTION_GE, AE_MOTION_EQ, AE_MOTION_NE };
    MoviecValue result = moviec_expression_sum( _expression );

    for( ;; )
    {
        size_t index = 0;

        for( ; index != 6; ++index )
        {
            if( strcmp( _expression->token, TOKENS[index] ) == 0 )
            {
                break;
            }
        }

        if( index == 6 )
        {
            break;
        }

        moviec_expression_next( _expression );
        MoviecValue arguments[2] = { result, moviec_expression_sum( _expression ) };
        result = moviec_expression_operation( _expression, OPCODES[index], arguments, 2 );
    }

    return result;
}
//////////////////////////////////////////////////////////////////////////
static MoviecValue moviec_expression_conditional( MoviecExpression * _expression )
{
    moviec_require( _expression->context, ++_expression->conditionalDepth <= 64, "expression nesting exceeds 64" );
    MoviecValue result = moviec_expression_comparison( _expression );

    if( strcmp( _expression->token, "?" ) == 0 )
    {
        moviec_expression_next( _expression );
        MoviecValue yes = moviec_expression_conditional( _expression );
        moviec_expression_expect( _expression, ":" );
        MoviecValue no = moviec_expression_conditional( _expression );
        MoviecValue arguments[3] = { result, yes, no };
        result = moviec_expression_operation( _expression, AE_MOTION_SELECT, arguments, 3 );
    }

    --_expression->conditionalDepth;

    return result;
}
//////////////////////////////////////////////////////////////////////////
void moviec_compile_expression( MoviecContext * _context, MoviecTrack * _track )
{
    MoviecExpression expression = { 0 };
    expression.context = _context;
    expression.track = _track;
    expression.source = _track->expression;
    moviec_expression_next( &expression );
    MoviecValue value = moviec_expression_conditional( &expression );
    moviec_require( _context, expression.token[0] == '\0', "unexpected token: %s", expression.token );
    moviec_require( _context, value.dimension == 1 || value.dimension == _track->dimension, "expression result has the wrong dimension" );
    _track->code = value.components[value.dimension == 1 ? 0 : _track->component];
}
