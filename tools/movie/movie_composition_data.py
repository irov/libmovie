from movie_stream import *
from movie_layer_data import *

class aeMovieCompositionData(object):
    def __init__(self):
        self.name = ""
        self.width = 0.0
        self.height = 0.0
        self.duration = 0.0
        self.frameDuration = 0.0
        self.frameDurationInv = 0.0
        self.loop_segment = None
        self.anchor_point = None
        self.offset_point = None
        self.bounds = None
        self.has_threeD = False
        self.cameraZoom = 0.0
        pass

    def load(self, f):
        self.name = read_string(f)

        self.width = read_float(f)
        self.height = read_float(f)
        self.duration = read_float(f)
        self.frameDuration = read_float(f)
        self.frameDurationInv = read_float(f)

        while True:
            flag = read_uint8_t(f)

            if flag == 0:
                break
                pass
            elif flag == 1:
                self.loop_segment = read_vec2f(f)
                pass
            elif flag == 2:
                self.anchor_point = read_vec3f(f)
                pass
            elif flag == 3:
                self.offset_point = read_vec3f(f)
                pass
            elif flag == 4:
                self.bounds = read_vec4f(f)
                pass
            else:
                return False
                pass
            pass

        self.has_threeD = read_bool(f)

        if self.has_threeD is True:
            self.cameraZoom = read_float(f)
            pass

        layer_count = read_size(f)

        self.layers = [aeMovieLayerData() for index in xrange(layer_count)]

        for layer in self.layers:
            if layer.load(f) is False:
                return False
                pass
            pass
        pass
    pass