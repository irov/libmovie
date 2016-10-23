from movie_type import *

class aeMovieResource(object):
    def __init__(self, type):
        self.type = type
        pass
    pass

class aeMovieResourceSolid(aeMovieResource):
    def __init__(self, width, height, r, g, b):
        aeMovieResource.__init__(self, AE_MOVIE_RESOURCE_SOLID)

        self.width = width
        self.height = height
        self.r = r
        self.g = g
        self.b = b
        pass
    pass

class aeMovieResourceVideo(aeMovieResource):
    def __init__(self, path, width, height, alpha, frameRate, duration):
        aeMovieResource.__init__(self, AE_MOVIE_RESOURCE_VIDEO)

        self.path = path
        self.width = width
        self.height = height
        self.alpha = alpha
        self.frameRate = frameRate
        self.duration = duration
        pass
    pass

class aeMovieResourceSound(aeMovieResource):
    def __init__(self, path, duration):
        aeMovieResource.__init__(self, AE_MOVIE_RESOURCE_SOUND)

        self.path = path
        self.duration = duration
        pass
    pass

class aeMovieResourceImage(aeMovieResource):
    def __init__(self, path, premultiplied, base_width, base_height, trim_width, trim_height, offset_x, offset_y, mesh):
        aeMovieResource.__init__(self, AE_MOVIE_RESOURCE_IMAGE)

        self.path = path
        self.premultiplied = premultiplied
        self.base_width = base_width
        self.base_height = base_height
        self.trim_width = trim_width
        self.trim_height = trim_height
        self.offset_x = offset_x
        self.offset_y = offset_y
        self.mesh = mesh
        pass
    pass

class aeMovieResourceSequence(aeMovieResource):
    def __init__(self, frameDurationInv, image_count, images):
        aeMovieResource.__init__(self, AE_MOVIE_RESOURCE_SEQUENCE)

        self.frameDurationInv = frameDurationInv
        self.image_count = image_count
        self.images = images
        pass
    pass

class aeMovieResourceParticle(aeMovieResource):
    def __init__(self, path):
        aeMovieResource.__init__(self, AE_MOVIE_RESOURCE_SEQUENCE)

        self.path = path
        pass
    pass