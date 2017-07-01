/**
@defgroup instance Library Instance
@brief Instance is the top element of the library API. Generally only one instance per application should be used.
*/

/**
@defgroup data Movie Data
@brief aeMovieData structure holds information about the movie and loads its resources through providers.
There should be only one aeMovieData per movie file.
Multiple user widgets that load and play compositions from the same movie should re-use the same aeMovieData structure for resource references.
*/

/**
@defgroup compositions Movie Compositions
@brief Compositions are the playable elements of a movie. A single movie can contain multiple compositions, which in turn consist of renderable and utility elements (images, videos, sounds, slots etc.).
@{
*/
	/**
	@defgroup layers Layers
	@brief Layers are composition building blocks.
	@{
	*/

		/**
		@defgroup slots Slots
		@brief Slots are points inside composition to which various data elements can be attached. Slots provide a transformation matrix for attached elements.
		*/

		/**
		@defgroup sockets Sockets
		@brief Sockets are sockets.
		*/

		/**
		@defgroup subcompositions Sub-compositions
		@brief Sub-composition is a composition inside the main one.
		*/

	// layers
	/// @}

// compositions
/// @}

/**
@defgroup data_types Data Types
@brief libmovie internal data types.
*/
