from movie_type import *
from movie_stream import *
from movie_resource import *

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

        for index_resource in range(resource_count):
            type = read_uint8_t(f)

            if type == AE_MOVIE_RESOURCE_SOLID:
                width = read_float(f)
                height = read_float(f)
                r = read_float(f)
                g = read_float(f)
                b = read_float(f)

                resource = aeMovieResourceSolid(width, height, r, g, b)
                self.resources.append(resource)
                pass
            elif type == AE_MOVIE_RESOURCE_VIDEO:
                path = read_string(f)
                width = read_float(f)
                height = read_float(f)
                alpha = read_uint8_t(f)
                frameRate = read_float(f)
                duration = read_float(f)

                resource = aeMovieResourceVideo(path, width, height, alpha, frameRate, duration)
                self.resources.append(resource)
                pass
            elif type == AE_MOVIE_RESOURCE_SOUND:
                path = read_string(f)
                duration = read_float(f)

                resource = aeMovieResourceSound(path, duration)
                self.resources.append(resource)
                pass
            elif type == AE_MOVIE_RESOURCE_IMAGE:
                path = read_string(f)
                premultiplied = read_bool(f)

                base_width = read_float(f)
                base_height = read_float(f)

                is_trim = read_bool(f)

                if is_trim is True:
                    trim_width = read_float(f)
                    trim_height = read_float(f)
                    offset_x = read_float(f)
                    offset_y = read_float(f)
                    pass
                else:
                    trim_width = base_width
                    trim_height = base_height
                    offset_x = 0.0
                    offset_y = 0.0
                    pass

                is_mesh = read_bool(f)

                if is_mesh is True:
                    mesh = read_mesh(f)
                    pass
                else:
                    mesh = None
                    pass

                resource = aeMovieResourceImage(path, premultiplied, base_width, base_height, trim_width, trim_height, offset_x, offset_y, mesh)
                self.resources.append(resource)
                pass
            elif type == AE_MOVIE_RESOURCE_SEQUENCE:

                frameDurationInv = read_float(f)
                image_count = read_size(f)

                images = []
                for index_image in xrange(image_count):
                    resource_id = read_size(f)

                    resource = self.resources[resource_id]
                    images.append(resource)
                    pass

                resource = aeMovieResourceSequence(frameDurationInv, image_count, images)
                self.resources.append(resource)
                pass
            elif type == AE_MOVIE_RESOURCE_PARTICLE:
                path = read_size(f)

                resource = aeMovieResourceParticle(path)
                self.resources.append(resource)
                pass
            pass

        composition_count = read_size(f)

        for index_composition in range(composition_count):
            composition_name = read_string(f)

            composition_width = read_float(f)
            composition_height = read_float(f)
            composition_duration = read_float(f)
            composition_frameDuration = read_float(f)
            composition_frameDurationInv = read_float(f)

            while True:
                flag = read_uint8_t(f)

                if flag == 0:
                    break
                    pass
                elif flag == 1:
                    loop_segment = read_vec2f(f)
                    pass
                elif flag == 2:
                    anchor_point = read_vec3f(f)
                    pass
                elif flag == 3:
                    offset_point = read_vec3f(f)
                    pass
                elif flag == 4:
                    bounds = read_vec4f(f)
                    pass
                else:
                    return False
                    pass
                pass

            has_threeD = read_bool(f)

            if has_threeD is True:
                cameraZoom = read_float(f)
                pass

            layer_count = read_size(f)

            for index_layer in xrange(layer_count):
                layer_name = read_string(f)

                layer_index = read_size(f)

                is_track_matte = read_bool(f)
                has_track_matte = read_bool(f)

                layer_type = read_uint8_t(f)

                layer_frame_count = read_size(f)

                while True:
                    extension = read_uint8_t(f)

                    if extension == 0:
                        break
                        pass
                    elif extension == 1:
                        timeremap_frame_count = read_size(f)

                        timeremap_times = read_n(f, read_float, timeremap_frame_count)
                        pass
                    elif extension == 2:
                        mesh_immutable = read_bool(f)

                        if mesh_immutable is True:
                            mesh_immutable_mesh = read_mesh(f)
                            pass
                        else:
                            mesh_frame_count = read_size(f)

                            for index_mesh in xrange(mesh_frame_count):
                                mesh_mesh = read_mesh(f)
                                pass
                            pass
                        pass
                    elif extension == 3:
                        bezier_warp_immutable = read_bool(f)

                        if bezier_warp_immutable is True:
                            bezier_warp_corners = read_n(f, read_vec2f, 4)
                            bezier_warp_beziers = read_n(f, read_vec2f, 8)
                            pass
                        else:
                            bezier_warp_frame_count = read_size(f)

                            for index_bezier_warp in xrange(bezier_warp_frame_count):
                                bezier_warp_corners = read_n(f, read_vec2f, 4)
                                bezier_warp_beziers = read_n(f, read_vec2f, 8)
                                pass
                        pass
                    elif extension == 4:
                        color_vertex_immutable_r = read_bool(f)

                        if color_vertex_immutable_r is True:
                            color_vertex_immutable_color_vertex_r = read_float(f)
                            pass
                        else:
                            color_vertex_frame_count = read_size(f)

                            color_vertex_color_vertex_r = read_n(f, read_float, color_vertex_frame_count)
                            pass

                        color_vertex_immutable_g = read_bool(f)

                        if color_vertex_immutable_g is True:
                            color_vertex_immutable_color_vertex_g = read_float(f)
                            pass
                        else:
                            color_vertex_frame_count = read_size(f)

                            color_vertex_color_vertex_g = read_n(f, read_float, color_vertex_frame_count)
                            pass

                        color_vertex_immutable_b = read_bool(f)

                        if color_vertex_immutable_b is True:
                            color_vertex_immutable_color_vertex_b = read_float(f)
                            pass
                        else:
                            color_vertex_frame_count = read_size(f)

                            color_vertex_color_vertex_b = read_n(f, read_float, color_vertex_frame_count)
                            pass
                        pass
                    elif extension == 5:
                        polygon_immutable = read_bool(f)

                        if polygon_immutable is True:
                            polygon_immutable_polygon = read_polygon(f)
                            pass
                        else:
                            polygon_polygon_count = read_size(f)

                            polygon_polygons = read_n(f, read_polygon, polygon_polygon_count)
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
                    pass
                else:
                    composition_index = read_size(f)
                    pass

                parent_index = read_size(f)

                reverse_time = read_bool(f)

                start_time = read_float(f)
                in_time = read_float(f)
                out_time = read_float(f)

                blend_mode = read_uint8_t(f)
                threeD = read_bool(f)
                params = read_uint32_t(f)

                play_count = read_size(f)
                stretch = read_float(f)

                immutable_property_mask = read_uint32_t(f)

                for mask in [AE_MOVIE_IMMUTABLE_ANCHOR_POINT_X, AE_MOVIE_IMMUTABLE_ANCHOR_POINT_Y, AE_MOVIE_IMMUTABLE_ANCHOR_POINT_Z,
                             AE_MOVIE_IMMUTABLE_POSITION_X, AE_MOVIE_IMMUTABLE_POSITION_Y, AE_MOVIE_IMMUTABLE_POSITION_Z,
                             AE_MOVIE_IMMUTABLE_SCALE_X, AE_MOVIE_IMMUTABLE_SCALE_Y, AE_MOVIE_IMMUTABLE_SCALE_Z,
                             AE_MOVIE_IMMUTABLE_QUATERNION_X, AE_MOVIE_IMMUTABLE_QUATERNION_Y, AE_MOVIE_IMMUTABLE_QUATERNION_Z, AE_MOVIE_IMMUTABLE_QUATERNION_W,
                             AE_MOVIE_IMMUTABLE_OPACITY]:
                    if read_property(f, immutable_property_mask & mask) is False:
                        return False
                        pass
                    pass
                pass
            pass

        print "successful"
        pass

m = aeMovieData()

f = open("ui/ui.aem", "rb")

with f:
    if m.load(f) is False:
        print "failed"
        pass
    pass

