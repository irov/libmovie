//////////////////////////////////////////////////////////////////////////
static const char * __opengl_get_error_string( GLenum _err )
{
    switch( _err )
    {
    case GL_INVALID_ENUM:
        return "GL_INVALID_ENUM";
    case GL_INVALID_VALUE:
        return "GL_INVALID_VALUE";
    case GL_INVALID_OPERATION:
        return "GL_INVALID_OPERATION";
    case GL_INVALID_FRAMEBUFFER_OPERATION:
        return "GL_INVALID_FRAMEBUFFER_OPERATION";
    default:
        {
        }
    }

    return "GL_UNKNOWN";
}
//////////////////////////////////////////////////////////////////////////
static void __opengl_error_check( const char * _method, const char * _file, int _line )
{
    GLenum err = glGetError();

    if( err == GL_NO_ERROR )
    {
        return;
    }

    const char * err_str = __opengl_get_error_string( err );

    emscripten_log( EM_LOG_CONSOLE, "opengl error method '%s' file '%s:%d' error '%s:%d'"
        , _method
        , _file
        , _line
        , err_str
        , err
    );
}
//////////////////////////////////////////////////////////////////////////
#   define GLCALL( Method, Args )\
    {\
        Method Args;\
        __opengl_error_check( #Method, __FILE__, __LINE__ );\
    }
//////////////////////////////////////////////////////////////////////////
#   define GLCALLR( R, Method, Args )\
    {\
        R = Method Args;\
        __opengl_error_check( #Method, __FILE__, __LINE__ );\
    }

//////////////////////////////////////////////////////////////////////////
static GLuint __make_opengl_program( const char * _name, uint32_t _version, const char * _vertexShaderSource, const char * _fragmentShaderSource )
{
    GLuint shader_vertex_id;
    GLCALLR( shader_vertex_id, glCreateShader, (GL_VERTEX_SHADER) );

    if( shader_vertex_id == 0 )
    {
        emscripten_log( EM_LOG_CONSOLE, "opengl create vertex shader '%s' version '%d'\n"
            , _name
            , _version
        );

        return 0U;
    }

    GLint vertexShaderSourceSize = (GLint)strlen( _vertexShaderSource );

    GLCALL( glShaderSource, (shader_vertex_id, 1, &_vertexShaderSource, &vertexShaderSourceSize) );
    GLCALL( glCompileShader, (shader_vertex_id) );

    GLint vertexShaderStatus;
    GLCALL( glGetShaderiv, (shader_vertex_id, GL_COMPILE_STATUS, &vertexShaderStatus) );

    if( vertexShaderStatus == GL_FALSE )
    {
        GLchar errorLog[1024];
        GLCALL( glGetShaderInfoLog, (shader_vertex_id, sizeof( errorLog ) - 1, NULL, errorLog) );

        emscripten_log( EM_LOG_CONSOLE, "opengl compilation vertex shader '%s' version '%d' error '%s'\n"
            , _name
            , _version
            , errorLog
        );

        return 0U;
    }

    GLuint shader_fragment_id;
    GLCALLR( shader_fragment_id, glCreateShader, (GL_FRAGMENT_SHADER) );

    if( shader_fragment_id == 0 )
    {
        emscripten_log( EM_LOG_CONSOLE, "opengl create fragment shader '%s' version '%d'\n"
            , _name
            , _version
        );

        return 0U;
    }

    GLint fragmentShaderSourceSize = (GLint)strlen( _fragmentShaderSource );

    GLCALL( glShaderSource, (shader_fragment_id, 1, &_fragmentShaderSource, &fragmentShaderSourceSize) );
    GLCALL( glCompileShader, (shader_fragment_id) );

    GLint fragmentShaderStatus;
    GLCALL( glGetShaderiv, (shader_fragment_id, GL_COMPILE_STATUS, &fragmentShaderStatus) );

    if( fragmentShaderStatus == GL_FALSE )
    {
        GLchar errorLog[1024];
        GLCALL( glGetShaderInfoLog, (shader_fragment_id, sizeof( errorLog ) - 1, NULL, errorLog) );

        emscripten_log( EM_LOG_CONSOLE, "opengl compilation fragment shader '%s' version '%d' error '%s'\n"
            , _name
            , _version
            , errorLog
        );

        return 0U;
    }

    GLuint program_id;
    GLCALLR( program_id, glCreateProgram, () );

    if( program_id == 0 )
    {
        emscripten_log( EM_LOG_CONSOLE, "opengl create program '%s' version '%d'\n"
            , _name
            , _version
        );

        return 0U;
    }

    GLCALL( glAttachShader, (program_id, shader_vertex_id) );
    GLCALL( glAttachShader, (program_id, shader_fragment_id) );

    GLCALL( glLinkProgram, (program_id) );

    GLint programLinkStatus;
    GLCALL( glGetProgramiv, (program_id, GL_LINK_STATUS, &programLinkStatus) );

    if( programLinkStatus == GL_FALSE )
    {
        GLchar errorLog[1024] = { 0 };
        GLCALL( glGetProgramInfoLog, (program_id, sizeof( errorLog ) - 1, NULL, errorLog) );

        emscripten_log( EM_LOG_CONSOLE, "opengl program '%s' version '%d' linking error '%s'\n"
            , _name
            , _version
            , errorLog
        );

        return 0U;
    }

    GLCALL( glDeleteShader, (shader_vertex_id) );
    GLCALL( glDeleteShader, (shader_fragment_id) );

    return program_id;
}
//////////////////////////////////////////////////////////////////////////
static GLuint __make_opengl_texture( uint32_t _width, uint32_t _height, const void * _data )
{
    GLuint texture_id;
    GLCALL( glGenTextures, (1, &texture_id) );
    GLCALL( glBindTexture, (GL_TEXTURE_2D, texture_id) );
    GLCALL( glTexParameteri, (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE) );
    GLCALL( glTexParameteri, (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE) );
    GLCALL( glTexParameteri, (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR) );
    GLCALL( glTexParameteri, (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR) );

    GLCALL( glTexImage2D, (GL_TEXTURE_2D, 0, GL_RGBA, _width, _height, 0, GL_RGBA, GL_UNSIGNED_BYTE, _data) );

    return texture_id;
}