from movie_type import *
from movie_stream import *
from movie_resource import *
from movie_composition_data import *

class aeMovieData(object):
    def __init__(self):
        self.movie_name = ""
        self.resources = []
        self.compositions = []
        pass

    def load(self, f):
        if read_char(f) != 'A' or read_char(f) != 'E' or read_char(f) != 'M' or read_char(f) != '1':
            return False
            pass

        version = read_uint32_t(f)

        print version, AE_MOVIE_VERSION

        if version != AE_MOVIE_VERSION:
            return False
            pass

        self.movie_name = read_string(f)

        print "movie_name", self.movie_name

        resource_count = read_size(f)

        print "resource_count", resource_count

        resource_factory = {}
        resource_factory[AE_MOVIE_RESOURCE_SOLID] = aeMovieResourceSolid
        resource_factory[AE_MOVIE_RESOURCE_VIDEO] = aeMovieResourceVideo
        resource_factory[AE_MOVIE_RESOURCE_SOUND] = aeMovieResourceSound
        resource_factory[AE_MOVIE_RESOURCE_IMAGE] = aeMovieResourceImage
        resource_factory[AE_MOVIE_RESOURCE_SEQUENCE] = aeMovieResourceSequence
        resource_factory[AE_MOVIE_RESOURCE_PARTICLE] = aeMovieResourceParticle

        for index_resource in range(resource_count):
            type = read_uint8_t(f)

            generator = resource_factory.get(type)

            if generator is None:
                return False
                pass

            resource = generator()

            resource.load(f)
            self.resources.append(resource)
            pass

        composition_count = read_size(f)

        self.compositions = [aeMovieCompositionData() for index in xrange(composition_count)]

        for composition in self.compositions:
            if composition.load(f) is False:
                return False
                pass
            pass

        print "successful"
        pass


    def get_images(self):
        images = []
        for resource in self.resources:
            type = resource.type
            if type == AE_MOVIE_RESOURCE_IMAGE:
                images.append(resource.path)
                pass
            pass

        return images
        pass
    pass
