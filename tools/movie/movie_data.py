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

        if version != AE_MOVIE_VERSION:
            return False
            pass

        self.movie_name = read_string(f)

        resource_count = read_size(f)

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

        return True
        pass


    def get_images(self):
        images = [resource.path for resource in self.resources if resource.type == AE_MOVIE_RESOURCE_IMAGE]

        return images
        pass

    def get_composition(self, compositionName):
        for composition in self.compositions:
            if composition.name == compositionName:
                return composition
                pass
            pass

        return None
        pass

    def __collect_composition_images(self, composition, images):
        for layer in composition.layers:
            if layer.type == AE_MOVIE_LAYER_TYPE_SEQUENCE:
                resource = self.resources[layer.resource]
                for resource_image_index in resource.images:
                    resource_image = self.resources[resource_image_index]

                    path = resource_image.path

                    if path in images:
                        continue
                        pass

                    images.append(path)
                    pass
                pass
            elif layer.type == AE_MOVIE_LAYER_TYPE_IMAGE:
                resource_image = self.resources[layer.resource]

                path = resource_image.path

                if path in images:
                    continue
                    pass

                images.append(path)
                pass
            elif layer.type == AE_MOVIE_LAYER_TYPE_MOVIE:
                sub_composition = self.compositions[layer.sub_composition]

                self.__collect_composition_images(sub_composition, images)
            pass
        pass

    def get_composition_images(self, compositionName):
        composition = self.get_composition(compositionName)

        if composition is None:
            return None
            pass

        images = []

        self.__collect_composition_images(composition, images)

        return images
        pass

    def get_master_compositions(self):
        compositions = [composition.name for composition in self.compositions if composition.master is True]

        return compositions
        pass
    pass
