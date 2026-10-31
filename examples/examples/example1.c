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

/*
==================================================
    EXAMPLE #1

Load movie file & give basic info about it.
No resource or node providers.
==================================================
*/

#include "common.h"

static const ae_char_t * ex_example_movie_path = "../../examples/resources/Knight/Knight.aem";
static const ae_char_t * ex_example_composition_name = "Knight";

int main( int argc, char * argv[] )
{
    AE_UNUSED( argc );
    AE_UNUSED( argv );
    //
    // Initialize library instance, movie & composition names.
    //

    ex_init( AE_HASHKEY_EMPTY, ex_example_movie_path, ex_example_composition_name );

    //
    // Set pointer to resource loader callback, in this example the empty one.
    //

    ex.resource_provider = &ex_callback_resource_provider_empty;
    ex.resource_deleter = &ex_callback_resource_deleter_empty;

    //
    // Load movie data structure, which in particular contains info about all compositions in the movie.
    //

    ex_load_movie_data();

    //
    // Usually we should set composition provider callbacks here,
    // but for the current example we are using AE_NULL ones.
    //

    //
    // Select the composition & load its data (nodes, cameras etc.).
    //

    ex_set_composition();

    //
    // Usually it can be played at this moment.
    //

    EX_LOG( "The movie composition has loaded successfully. See further examples.\n" );

    //
    // Shutdown.
    //

    ex_shutdown();

    return EXIT_SUCCESS;
}
