from movie_type import *
from movie_stream import *

class aeMovieResource(object):
    def __init__(self, type):
        self.type = type
        pass
    pass

class aeMovieResourceSolid(aeMovieResource):
    def __init__(self):
        aeMovieResource.__init__(self, AE_MOVIE_RESOURCE_SOLID)

        self.width = 0.0
        self.height = 0.0
        self.r = 1.0
        self.g = 1.0
        self.b = 1.0
        pass

    def load(self, f):
        self.width = read_float(f)
        self.height = read_float(f)
        self.r = read_float(f)
        self.g = read_float(f)
        self.b = read_float(f)
        pass
    pass

class aeMovieResourceVideo(aeMovieResource):
    def __init__(self):
        aeMovieResource.__init__(self, AE_MOVIE_RESOURCE_VIDEO)

        self.path = ""
        self.width = 0.0
        self.height = 0.0
        self.alpha = False
        self.frameRate = 0.0
        self.duration = 0.0
        pass

    def load(self, f):
        self.path = read_string(f)
        self.width = read_float(f)
        self.height = read_float(f)
        self.alpha = read_uint8_t(f)
        self.frameRate = read_float(f)
        self.duration = read_float(f)
        pass
    pass

class aeMovieResourceSound(aeMovieResource):
    def __init__(self):
        aeMovieResource.__init__(self, AE_MOVIE_RESOURCE_SOUND)

        self.path = ""
        self.duration = 0.0
        pass

    def load(self, f):
        self.path = read_string(f)
        self.duration = read_float(f)
        pass
    pass

class aeMovieResourceImage(aeMovieResource):
    def __init__(self):
        aeMovieResource.__init__(self, AE_MOVIE_RESOURCE_IMAGE)

        self.path = ""
        self.premultiplied = False
        self.base_width = 0.0
        self.base_height = 0.0
        self.trim_width = 0.0
        self.trim_height = 0.0
        self.offset_x = 0.0
        self.offset_y = 0.0
        self.mesh = None
        pass

    def load(self, f):
        self.path = read_string(f)
        self.premultiplied = read_bool(f)

        self.base_width = read_float(f)
        self.base_height = read_float(f)

        is_trim = read_bool(f)

        if is_trim is True:
            self.trim_width = read_float(f)
            self.trim_height = read_float(f)
            self.offset_x = read_float(f)
            self.offset_y = read_float(f)
            pass
        else:
            self.trim_width = self.base_width
            self.trim_height = self.base_height
            self.offset_x = 0.0
            self.offset_y = 0.0
            pass

        is_mesh = read_bool(f)

        if is_mesh is True:
            self.mesh = read_mesh(f)
            pass
        else:
            self.mesh = None
            pass
        pass
    pass

class aeMovieResourceSequence(aeMovieResource):
    def __init__(self):
        aeMovieResource.__init__(self, AE_MOVIE_RESOURCE_SEQUENCE)

        self.frameDurationInv = 0.0
        self.image_count = 0
        self.images = []
        pass

    def load(self, f):
        self.frameDurationInv = read_float(f)
        self.image_count = read_size(f)

        self.images = read_n(f, read_size, self.image_count)
        pass
    pass

class aeMovieResourceParticle(aeMovieResource):
    def __init__(self):
        aeMovieResource.__init__(self, AE_MOVIE_RESOURCE_SEQUENCE)

        self.path = ""
        pass

    def load(self, f):
        self.path = read_string(f)
        pass
    pass