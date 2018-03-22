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

#ifndef MOVIE_MOVIE_RESOURCE_H_
#define MOVIE_MOVIE_RESOURCE_H_

#include "movie/movie_type.h"

typedef struct aeMovieResourceSolid
{
    AE_MOVIE_RESOURCE_BASE();

    ae_float_t width;
    ae_float_t height;

    ae_color_t color;

} aeMovieResourceSolid;

typedef struct aeMovieResourceVideo
{
    AE_MOVIE_RESOURCE_BASE();

    ae_string_t path;
    ae_uint32_t codec;

    ae_float_t width;
    ae_float_t height;

    ae_uint32_t alpha;
    ae_time_t frameRate; //No TIMESCALE
    ae_time_t duration; //No TIMESCALE

} aeMovieResourceVideo;

typedef struct aeMovieResourceSound
{
    AE_MOVIE_RESOURCE_BASE();

    ae_string_t path;
    ae_uint32_t codec;

    ae_time_t duration; //No TIMESCALE

} aeMovieResourceSound;

typedef struct aeMovieResourceImage
{
    AE_MOVIE_RESOURCE_BASE();

    ae_string_t path;
    ae_uint32_t codec;

    ae_bool_t premultiplied;

    ae_float_t base_width;
    ae_float_t base_height;
    ae_float_t trim_width;
    ae_float_t trim_height;
    ae_float_t offset_x;
    ae_float_t offset_y;

    const ae_vector2_t * uv;
    const ae_mesh_t * mesh;

} aeMovieResourceImage;

typedef struct aeMovieResourceSequence
{
    AE_MOVIE_RESOURCE_BASE();

    ae_time_t frameDurationInv; //No TIMESCALE

    ae_uint32_t image_count;
    const aeMovieResourceImage * const * images;

} aeMovieResourceSequence;

typedef struct aeMovieResourceParticle
{
    AE_MOVIE_RESOURCE_BASE();

    ae_string_t path;
    ae_uint32_t codec;

    ae_uint32_t image_count;
    const aeMovieResourceImage * const * images;

} aeMovieResourceParticle;

typedef struct aeMovieResourceSlot
{
    AE_MOVIE_RESOURCE_BASE();

    ae_float_t width;
    ae_float_t height;

} aeMovieResourceSlot;

#endif