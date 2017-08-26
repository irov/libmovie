/*
==================================================
	EXAMPLE #1

Load movie file & give basic info about it.
No resource or node providers.
==================================================
*/

#include "common.h"

int main( int argc, char *argv[] ) {
	//
	// Initialize library instance, movie & composition names.
	//

	ex_init( "f86464bbdebf0fe3e684b03ec263d049d079e6f1", "../../../examples/resources/ui/ui.aem", "LOBBY" );

	//
	// Set pointer to resource loader callback, in this example the empty one.
	//

	ex.resource_provider = &ex_callback_resource_provider_empty;

	//
	// Load movie data structure, which in particular contains info about all compositions in the movie.
	//

	ex_load_movie_data();

	//
	// Usually we should set composition provider callbacks here,
	// but for the current example we are using NULL ones.
	//

	//
	// Select the composition & load its data (nodes, cameras etc.).
	//

	ex_set_composition();

	//
	// Usually it can be played at this moment.
	//

	EX_LOG("The movie composition has loaded successfully. See further examples.\n");

	return 0;
}
