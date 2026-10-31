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
