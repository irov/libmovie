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

#ifndef __AEMovieDataEx_H__
#define __AEMovieDataEx_H__

#include "AEConfig.h"

#include "AESound.h"

NS_CC_EXT_BEGIN;

class AEMovieData : public cocos2d::Ref
{
public:
    AEMovieData();
    virtual ~AEMovieData();

    // path must end with '/'
	virtual bool initWithFileAndFramesFolder( aeMovieInstance * instance, const std::string & filepath, const std::string & framesFolder );

    const aeMovieData * getData() const { return _data; }

protected:
    static void * callbackResourceProvider( const aeMovieResource * _resource, ae_voidptr_t _data );
    static void callbackResourceDeleter( aeMovieResourceTypeEnum _type, ae_voidptr_t _data, ae_voidptr_t _ud );

    cocos2d::Ref * createImage( const std::string & path, int width, int height );

    AESound * createSound( const std::string & path );


    // relative path to folder with movie data ending with '/' (eg. 'AEM/example/', where the .aem file & its referred resources are contained)
    std::string _path;

    // data received from the movie library
    aeMovieData * _data;

    typedef std::vector<AESound *> TVectorSound;
    TVectorSound _sounds;

private:
    CC_DISALLOW_COPY_AND_ASSIGN( AEMovieData );
};

NS_CC_EXT_END;

#endif
