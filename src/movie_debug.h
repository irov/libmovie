/******************************************************************************
* libMOVIE Software License v1.0
*
* Copyright (c) 2016-2023, Yuriy Levchenko <irov13@mail.ru>
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

#ifndef MOVIE_DEBUG_H_
#define MOVIE_DEBUG_H_

#include "movie/movie_type.h"

AE_INTERNAL ae_void_t __movie_break_point( ae_void_t )
{
    //Breakpoint
    ae_uint32_t breakpoint_this = 0;
    (ae_void_t)breakpoint_this;
}

#ifdef AE_MOVIE_DEBUG
#   define AE_RETURN_ERROR_RESULT(Result) __movie_break_point(); return Result
#else
#   define AE_RETURN_ERROR_RESULT(Result) return Result
#endif

#ifdef AE_MOVIE_DEBUG
#   define AE_MOVIE_ASSERTION_VOID(Instance, Condition, ...) {if(!(Condition)) {(Instance)->logger((Instance)->instance_userdata, AE_ERROR_CRITICAL, __VA_ARGS__); __movie_break_point(); return;}}
#   define AE_MOVIE_ASSERTION_RESULT(Instance, Condition, Result, ...) {if(!(Condition)) {(Instance)->logger((Instance)->instance_userdata, AE_ERROR_CRITICAL, __VA_ARGS__); __movie_break_point(); return Result;}}
#else
#   define AE_MOVIE_ASSERTION_VOID(Instance, Condition, ...) AE_UNUSED(Instance)
#   define AE_MOVIE_ASSERTION_RESULT(Instance, Condition, Result, ...) AE_UNUSED(Instance)
#endif

#ifdef AE_MOVIE_DEBUG
#   define AE_MOVIE_PANIC_MEMORY(Memory, Result) {if(Memory == AE_NULLPTR) {__movie_break_point(); return Result;}}
#else
#   define AE_MOVIE_PANIC_MEMORY(Memory, Result)
#endif

#endif