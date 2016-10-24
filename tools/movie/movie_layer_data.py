from movie_stream import *

class aeMovieLayerData(object):
    def __init__(self):
        self.name = ""
        self.index = 0
        self.is_track_matte = False
        self.has_track_matte = False
        self.type = 0
        self.frame_count = 0

        self.times = None
        self.immutable_mesh = None
        self.mesh = None
        self.immutable_bezier_warp = None
        self.bezier_warps = None
        self.immutable_color_vertex_r = 1.0
        self.color_vertex_r = None
        self.immutable_color_vertex_g = 1.0
        self.color_vertex_g = None
        self.immutable_color_vertex_b = 1.0
        self.color_vertex_b = None
        self.immutable_polygon = None
        self.polygons = None

        self.parent_index = -1
        self.reverse_time = False
        self.start_time = 0.0
        self.in_time = 0.0
        self.out_time = 0.0

        self.resource = None
        self.sub_composition = None

        self.blend_mode = 0
        self.threeD = False
        self.params = 0

        self.play_count = 0
        self.stretch = 1.0
        pass

    def load(self, f):
        self.layer_name = read_string(f)

        self.layer_index = read_size(f)

        self.is_track_matte = read_bool(f)
        self.has_track_matte = read_bool(f)

        self.type = read_uint8_t(f)

        self.frame_count = read_size(f)

        while True:
            extension = read_uint8_t(f)

            if extension == 0:
                break
                pass
            elif extension == 1:
                timeremap_frame_count = read_size(f)

                self.times = read_n(f, read_float, timeremap_frame_count)
                pass
            elif extension == 2:
                mesh_immutable = read_bool(f)

                if mesh_immutable is True:
                    self.immutable_mesh = read_mesh(f)
                    pass
                else:
                    mesh_frame_count = read_size(f)

                    for index_mesh in xrange(mesh_frame_count):
                        self.mesh = read_mesh(f)
                        pass
                    pass
                pass
            elif extension == 3:
                bezier_warp_immutable = read_bool(f)

                if bezier_warp_immutable is True:
                    corners = read_n(f, read_vec2f, 4)
                    beziers = read_n(f, read_vec2f, 8)

                    self.immutable_bezier_warp = aeMovieBezierWarp(corners, beziers)
                    pass
                else:
                    bezier_warp_frame_count = read_size(f)

                    self.bezier_warps = []

                    for index_bezier_warp in xrange(bezier_warp_frame_count):
                        corners = read_n(f, read_vec2f, 4)
                        beziers = read_n(f, read_vec2f, 8)

                        bezier_warp = aeMovieBezierWarp(corners, beziers)
                        self.bezier_warps.append(bezier_warp)
                        pass
                    pass
                pass
            elif extension == 4:
                color_vertex_immutable_r = read_bool(f)

                if color_vertex_immutable_r is True:
                    self.immutable_color_vertex_r = read_float(f)
                    pass
                else:
                    color_vertex_frame_count = read_size(f)

                    self.color_vertex_r = read_n(f, read_float, color_vertex_frame_count)
                    pass

                color_vertex_immutable_g = read_bool(f)

                if color_vertex_immutable_g is True:
                    self.immutable_color_vertex_g = read_float(f)
                    pass
                else:
                    color_vertex_frame_count = read_size(f)

                    self.color_vertex_g = read_n(f, read_float, color_vertex_frame_count)
                    pass

                color_vertex_immutable_b = read_bool(f)

                if color_vertex_immutable_b is True:
                    self.immutable_color_vertex_b = read_float(f)
                    pass
                else:
                    color_vertex_frame_count = read_size(f)

                    self.color_vertex_b = read_n(f, read_float, color_vertex_frame_count)
                    pass
                pass
            elif extension == 5:
                polygon_immutable = read_bool(f)

                if polygon_immutable is True:
                    self.immutable_polygon = read_polygon(f)
                    pass
                else:
                    polygon_polygon_count = read_size(f)

                    self.polygons = read_n(f, read_polygon, polygon_polygon_count)
                    pass
                pass
            else:
                return False
                pass
            pass
        pass

        is_resource_or_composition = read_bool(f)

        if is_resource_or_composition is True:
            resource_index = read_size(f)

            if resource_index == 0:
                self.resource = None
                pass
            else:
                self.resource = resource_index - 1
                pass

            self.sub_composition = None
            pass
        else:
            composition_index = read_size(f)

            self.sub_composition = composition_index

            self.resource = None
            pass

        self.parent_index = read_size(f)

        self.reverse_time = read_bool(f)

        self.start_time = read_float(f)
        self.in_time = read_float(f)
        self.out_time = read_float(f)

        self.blend_mode = read_uint8_t(f)
        self.threeD = read_bool(f)
        self.params = read_uint32_t(f)

        self.play_count = read_size(f)
        self.stretch = read_float(f)

        immutable_property_mask = read_uint32_t(f)

        for mask in [AE_MOVIE_IMMUTABLE_ANCHOR_POINT_X, AE_MOVIE_IMMUTABLE_ANCHOR_POINT_Y,
                     AE_MOVIE_IMMUTABLE_ANCHOR_POINT_Z,
                     AE_MOVIE_IMMUTABLE_POSITION_X, AE_MOVIE_IMMUTABLE_POSITION_Y, AE_MOVIE_IMMUTABLE_POSITION_Z,
                     AE_MOVIE_IMMUTABLE_SCALE_X, AE_MOVIE_IMMUTABLE_SCALE_Y, AE_MOVIE_IMMUTABLE_SCALE_Z,
                     AE_MOVIE_IMMUTABLE_QUATERNION_X, AE_MOVIE_IMMUTABLE_QUATERNION_Y,
                     AE_MOVIE_IMMUTABLE_QUATERNION_Z, AE_MOVIE_IMMUTABLE_QUATERNION_W,
                     AE_MOVIE_IMMUTABLE_OPACITY]:
            if read_property(f, immutable_property_mask & mask) is False:
                return False
                pass
            pass

        return True
        pass
    pass