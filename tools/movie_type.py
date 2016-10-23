AE_MOVIE_VERSION = 6

AE_MOVIE_RESOURCE_NONE = 0
AE_MOVIE_RESOURCE_SOLID = 4
AE_MOVIE_RESOURCE_VIDEO = 5
AE_MOVIE_RESOURCE_SOUND = 6
AE_MOVIE_RESOURCE_IMAGE = 7
AE_MOVIE_RESOURCE_SEQUENCE = 8
AE_MOVIE_RESOURCE_PARTICLE = 9

AE_MOVIE_IMMUTABLE_ANCHOR_POINT_X = 0x00000001
AE_MOVIE_IMMUTABLE_ANCHOR_POINT_Y = 0x00000002
AE_MOVIE_IMMUTABLE_ANCHOR_POINT_Z = 0x00000004
AE_MOVIE_IMMUTABLE_POSITION_X = 0x00000010
AE_MOVIE_IMMUTABLE_POSITION_Y = 0x00000020
AE_MOVIE_IMMUTABLE_POSITION_Z = 0x00000040
AE_MOVIE_IMMUTABLE_SCALE_X = 0x00000100
AE_MOVIE_IMMUTABLE_SCALE_Y = 0x00000200
AE_MOVIE_IMMUTABLE_SCALE_Z = 0x00000400
AE_MOVIE_IMMUTABLE_QUATERNION_X = 0x00001000
AE_MOVIE_IMMUTABLE_QUATERNION_Y = 0x00002000
AE_MOVIE_IMMUTABLE_QUATERNION_Z = 0x00004000
AE_MOVIE_IMMUTABLE_QUATERNION_W = 0x00008000
AE_MOVIE_IMMUTABLE_OPACITY = 0x00010000

class aeMovieMesh(object):
    def __init__(self, positions, uvs, indices):
        self.positions = positions
        self.uvs = uvs
        self.indices = indices
        pass
    pass

class aeMoviePolygon(object):
    def __init__(self, points):
        self.points = points
        pass
    pass