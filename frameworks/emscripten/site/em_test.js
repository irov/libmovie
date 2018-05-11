var movie = AEMovie()

var resources = {}
var resource_enumerator = 0

function create_resource_id()
{
    resource_enumerator += 1
    
    return resource_enumerator
}
    
function em_player_resource_sound_provider(ud, path, codec, duration)
{
    console.log("em_player_resource_sound_provider")
        
    var id = create_resource_id()

    var a = new Audio();
    a.src = path
    
    resources[id] = a
    
    return id
}

function em_player_resource_sound_deleter(ud, id)
{
    resources[id] = undefined
}

function em_player_node_sound_play(ud, id)
{
    var a = resources[id]
    a.play()
}

function em_player_node_sound_stop(ud, id)
{
    var a = resources[id]    
    //a.stop()?
}

function em_player_node_sound_pause(ud, id)
{
    var a = resources[id]    
    //a.pause()?
}

function em_player_node_sound_resume(ud, id)
{
    var a = resources[id]    
    //a.resume()?
}

var loaders = {}

function __onload_image(id, image, path)
{
    var imageloadercanvas = document.getElementById("imageloader");
    
    movie.utils_opengl_load_texure_from_image(imageloadercanvas, id, image)
    
    delete loaders[path]
}

function em_player_resource_image_provider(ud, id, path, codec, premultiplied)
{
    loaders[path] = 1

    var image = new Image();
    image.onload = function() { __onload_image(id, image, path); }
    image.src = path;
    image.setAttribute('crossOrigin', '');
}

function onload_movie(canvas, gl, response, em_player, composition_name)
{
    var ptr = movie.utils_malloc_arraybuffer(response);
        
    var em_movie_data = movie.create_movie_data(em_player, ptr)
    
    movie.utils_free_arraybuffer(ptr);
    
    if( em_movie_data == 0 )
    {
        console.log("invalid create movie data")
    
        return
    }
    
    var em_movie_composition = movie.create_movie_composition(em_player, em_movie_data, composition_name)
    
    if( em_movie_composition == 0 )
    {
        console.log("invalid create movie composition")
        
        return
    }
       
    movie.set_movie_composition_wm(em_movie_composition, 0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 0.0)
        
    movie.play_movie_composition(em_movie_composition, 0.0)
    movie.set_movie_composition_loop(em_movie_composition, 1)
        
    function wait_resource_load(timestamp2)
    {
        var loaders_length = Object.keys(loaders).length
        
        if( loaders_length != 0 )
        {
            requestAnimationFrame(wait_resource_load);
            return
        }
        
        var start = performance.now();
        
        function tick(timestamp)
        {
            requestAnimationFrame(tick);
            
            var progressMs = timestamp - start;
            var fpsInterval = 70.0
            
            if( progressMs > fpsInterval )
            {
                start = timestamp - (progressMs % fpsInterval)
                
                gl.clear(gl.COLOR_BUFFER_BIT);
                
                movie.update_movie_composition(em_player, em_movie_composition, progressMs * 0.001)
                
                movie.render_movie_composition(em_player, em_movie_composition)
            }
        }
        
        requestAnimationFrame(tick)
    }
    
    requestAnimationFrame(wait_resource_load)
};

function libMOVIE_test(rendercanvas, movie_hash, movie_path, movie_aem, composition_name)
{
    var canvas = document.getElementById(rendercanvas);    
    var gl = movie.utils_opengl_initialize(canvas);
        
    gl.enable(gl.BLEND)
    gl.clearColor(0.0, 0.0, 0.0, 1.0);
    
    var em_player = movie.create_player(movie_hash, movie_path, canvas.width, canvas.height, 0)
    
    if( em_player == 0 )
    {
        console.log("invalid create player")
        
        return
    }
    
    var xhr = new XMLHttpRequest();
    xhr.open('get', movie_path + movie_aem, true); 
    xhr.responseType = "arraybuffer"
    xhr.onload = function(){ onload_movie(canvas, gl, xhr.response, em_player, composition_name) }
    xhr.send();    
 }