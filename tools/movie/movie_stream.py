import struct
from movie_type import *

def read_char(f):
    data = f.read(1)
    value = struct.unpack('c', data)[0]
    return value
    pass

def read_uint8_t(f):
    data = f.read(1)
    value = struct.unpack('B', data)[0]
    return value
    pass

def read_bool(f):
    value = read_uint8_t(f)

    return value != 0
    pass

def read_uint16_t(f):
    data = f.read(2)
    value = struct.unpack('H', data)[0]
    return value
    pass

def read_uint32_t(f):
    data = f.read(4)
    value = struct.unpack('I', data)[0]
    return value
    pass

def read_float(f):
    data = f.read(4)
    value = struct.unpack('f', data)[0]
    return value
    pass

def read_size(f):
    value_255 = read_uint8_t(f)

    if value_255 != 255:
        return value_255
        pass

    value_65535 = read_uint16_t(f)

    if value_65535 != 65535:
        return value_65535
        pass

    value = read_uint32_t(f)

    return value
    pass

def read_string(f):
    size = read_size(f)

    data = f.read(size)
    value = struct.unpack('%ds'%size, data)[0]

    return value
    pass

def read_vec2f(f):
    data = f.read(8)
    value = struct.unpack('ff', data)
    return value
    pass

def read_vec3f(f):
    data = f.read(12)
    value = struct.unpack('fff', data)
    return value
    pass

def read_vec4f(f):
    data = f.read(16)
    value = struct.unpack('ffff', data)
    return value
    pass

def read_n(f, read, size):
    n = []
    for index in xrange(size):
        v = read(f)
        n.append(v)
        pass

    return n
    pass

def read_mesh(f):
    vertex_count = read_size(f)

    if vertex_count == 0:
        indices_count = 0
        positions = []
        uvs = []
        indices = []
        pass
    else:
        indices_count = read_size(f)

        positions = read_n(f, read_vec2f, vertex_count)
        uvs = read_n(f, read_vec2f, vertex_count)
        indices = read_n(f, read_uint16_t, indices_count)
        pass

    mesh = aeMovieMesh(positions, uvs, indices)

    return mesh
    pass

def read_polygon(f):
    point_count = read_size(f)

    if point_count == 0:
        points = []
        pass
    else:
        points = read_n(f, read_vec2f, point_count)
        pass

    polygon = aeMoviePolygon(points)

    return polygon
    pass

def read_property(f, immnutable_mask):
    if immnutable_mask != 0:
        value = read_float(f)
        pass
    else:
        block_count = read_size(f)

        for index_block in xrange(block_count):
            block_type = read_uint8_t(f)

            block_value_count = read_size(f)

            if block_type == 0:
                block_value = read_float(f)
                pass
            elif block_type == 1:
                block_begin = read_float(f)
                block_end = read_float(f)
                pass
            elif block_type == 3:
                block_values = read_n(f, read_float, block_value_count)
                pass
            else:
                return False
                pass
            pass
        pass
    pass