/******************************************************************************
* libMOVIE Software License v1.0
*
* Copyright (c) 2016-2017, Levchenko Yuriy <irov13@mail.ru>
* All rights reserved.
*
* You are granted a perpetual, non-exclusive, non-sublicensable, and
* non-transferable license to use, install, execute, and perform the libMOVIE
* software and derivative works solely for personal or internal
* use. Without the written permission of Levchenko Yuriy, you may not (a) modify, translate,
* adapt, or develop new applications using the libMOVIE or otherwise
* create derivative works or improvements of the libMOVIE or (b) remove,
* delete, alter, or obscure any trademarks or any copyright, trademark, patent,
* or other intellectual property or proprietary rights notices on or in the
* Software, including any copy thereof. Redistributions in binary or source
* form must include this license and terms.
*
* THIS SOFTWARE IS PROVIDED BY LEVCHENKO YURIY "AS IS" AND ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
* EVENT SHALL LEVCHENKO YURIY BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES, BUSINESS INTERRUPTION,
* OR LOSS OF USE, DATA, OR PROFITS) HOWEVER CAUSED AND ON ANY THEORY OF
* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
* OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************/

#ifndef MOVIE_MOVIE_RESOURCE_H_
#define MOVIE_MOVIE_RESOURCE_H_

#	include "movie/movie_type.h"

typedef struct aeMovieResourceSolid
{
	AE_MOVIE_RESOURCE_BASE();

	float width;
	float height;
	float r;
	float g;
	float b;

} aeMovieResourceSolid;

typedef struct aeMovieResourceVideo
{
	AE_MOVIE_RESOURCE_BASE();

	ae_string_t path;

	float width;
	float height;

	uint32_t alpha;
	float frameRate;
	float duration;

} aeMovieResourceVideo;

typedef struct aeMovieResourceSound
{
	AE_MOVIE_RESOURCE_BASE();

	ae_string_t path;

	float duration;

} aeMovieResourceSound;

typedef struct aeMovieResourceImage
{
	AE_MOVIE_RESOURCE_BASE();

	ae_string_t path;

	ae_bool_t premultiplied;

	float base_width;
	float base_height;
	float trim_width;
	float trim_height;
	float offset_x;
	float offset_y;

	const ae_vector2_t * uv;
	const aeMovieMesh * mesh;

} aeMovieResourceImage;

typedef struct aeMovieResourceSequence
{
	AE_MOVIE_RESOURCE_BASE();

	float frameDurationInv;

	uint32_t image_count;
	const aeMovieResourceImage * const * images;

} aeMovieResourceSequence;

typedef struct aeMovieResourceParticle
{
	AE_MOVIE_RESOURCE_BASE();

	ae_string_t path;

} aeMovieResourceParticle;

typedef struct aeMovieResourceSlot
{
	AE_MOVIE_RESOURCE_BASE();

	float width;
	float height;

} aeMovieResourceSlot;

#	undef AE_MOVIE_RESOURCE_BASE

#endif