AEMovie["create_player"] = cwrap('em_create_player', 'number', ['string', 'string', 'number', 'number', 'number'])
AEMovie["delete_player"] = cwrap('em_delete_player', 'void', ['number'])

AEMovie["create_movie_data"] = cwrap('em_create_movie_data', 'number', ['number', 'number'])
AEMovie["delete_movie_data"] = cwrap('em_delete_movie_data', 'void', ['number'])

AEMovie["create_movie_composition"] = cwrap('em_create_movie_composition', 'number', ['number', 'number', 'string'])
AEMovie["delete_movie_composition"] = cwrap('em_delete_movie_composition', 'void', ['number'])

AEMovie["set_movie_composition_loop"] = cwrap('em_set_movie_composition_loop', 'void', ['number', 'number'])
AEMovie["get_movie_composition_loop"] = cwrap('em_get_movie_composition_loop', 'number', ['number'])

AEMovie["set_movie_composition_work_area"] = cwrap('em_set_movie_composition_work_area', 'void', ['number'], ['number'], ['number'])
AEMovie["remove_movie_composition_work_area"] = cwrap('em_remove_movie_composition_work_area', 'void', ['number'])

AEMovie["play_movie_composition"] = cwrap('em_play_movie_composition', 'void', ['number', 'number'])
AEMovie["stop_movie_composition"] = cwrap('em_stop_movie_composition', 'void', ['number'])
AEMovie["pause_movie_composition"] = cwrap('em_pause_movie_composition', 'void', ['number'])
AEMovie["resume_movie_composition"] = cwrap('em_resume_movie_composition', 'void', ['number'])
AEMovie["interrupt_movie_composition"] = cwrap('em_interrupt_movie_composition', 'void', ['number', 'number'])

AEMovie["set_movie_composition_time"] = cwrap('em_set_movie_composition_time', 'void', ['number', 'number'])
AEMovie["get_movie_composition_time"] = cwrap('em_get_movie_composition_time', 'number', ['number'])

AEMovie["get_movie_composition_duration"] = cwrap('em_get_movie_composition_duration', 'number', ['number'])
AEMovie["get_movie_composition_width"] = cwrap('em_get_movie_composition_width', 'number', ['number'])
AEMovie["get_movie_composition_height"] = cwrap('em_get_movie_composition_height', 'number', ['number'])
AEMovie["get_movie_composition_frame_count"] = cwrap('em_get_movie_composition_frame_count', 'number', ['number'])

AEMovie["get_movie_composition_in_loop"] = cwrap('em_get_movie_composition_in_loop', 'number', ['number'])
AEMovie["get_movie_composition_out_loop"] = cwrap('em_get_movie_composition_out_loop', 'number', ['number'])

AEMovie["has_movie_composition_node"] = cwrap('em_has_movie_composition_node', 'number', ['number', 'string'])

AEMovie["get_movie_composition_node_in_time"] = cwrap('em_get_movie_composition_node_in_time', 'number', ['number', 'string'])
AEMovie["em_get_movie_composition_node_out_time"] = cwrap('em_get_movie_composition_node_out_time', 'number', ['number', 'string'])

AEMovie["set_movie_composition_node_enable"] = cwrap('em_set_movie_composition_node_enable', 'void', ['number', 'string', 'number'])
AEMovie["get_movie_composition_node_enable"] = cwrap('em_get_movie_composition_node_enable', 'number', ['number', 'string'])

AEMovie["update_movie_composition"] = cwrap('em_update_movie_composition', 'void', ['number', 'number', 'number'])
AEMovie["render_movie_composition"] = cwrap('em_render_movie_composition', 'void', ['number', 'number'])

AEMovie["utils_opengl_create_texture"] = cwrap('em_utils_opengl_create_texture', 'void', ['number', 'number', 'number', 'number'])
AEMovie["set_movie_composition_wm"] = cwrap('em_set_movie_composition_wm', 'void', ['number', 'number', 'number', 'number', 'number', 'number', 'number', 'number'])

AEMovie["utils_opengl_initialize"] = function(canvas)
{    
    try 
    {
        var context = canvas.getContext("webgl");
        
        if( context == null )
        {
            context = canvas.getContext("experimental-webgl");
        }
     
        if( context == null) 
        {
            console.log("Could not initialize WebGL")
            
            return false
        }
        
        var webGLContextAttributes = {}
        webGLContextAttributes['majorVersion'] = 2;
        webGLContextAttributes['minorVersion'] = 0;
        
        var glctxhandle = GL.registerContext(context, webGLContextAttributes);
        GL.makeContextCurrent(glctxhandle)

        return context
    } 
    catch(e) 
    {
        console.log(e)
    }
    
    return false
}

AEMovie["utils_opengl_load_texure_from_image"] = function(imageloadercanvas, id, image)
{
    imageloadercanvas.setAttribute('width', '512px');
    imageloadercanvas.setAttribute('height', '512px');
    
    context = imageloadercanvas.getContext('2d');
    
    var width = 512
    var height = 512
    
    context.drawImage(image,0,0, image.width, image.height, 0, 0, width, height);
    
    var image_data = context.getImageData(0, 0, width, height);
    
    var ptr = AEMovie["utils_malloc_arraybuffer"](image_data.data)

    AEMovie["utils_opengl_create_texture"](id, width, height, ptr)
    
    AEMovie["utils_free_arraybuffer"](ptr);
}

AEMovie["utils_malloc_arraybuffer"] = function(arraybuffer)
{
    var numBytes = arraybuffer.byteLength;
        
    var ptr = AEMovie["_malloc"](numBytes);
    var heapBytes= new Uint8Array(AEMovie["HEAPU8"].buffer, ptr, numBytes);
    heapBytes.set(new Uint8Array(arraybuffer));
    
    var byteOffset = heapBytes.byteOffset
    
    return byteOffset
}

AEMovie["utils_free_arraybuffer"] = function(ptr)
{
    AEMovie["_free"](ptr);
}