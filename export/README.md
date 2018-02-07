# Scripts

# Atlas export script
```sh
scripts/ae_movie_atlas.bat
```
arguments:
* image_count
* in_path
* log_path
* atlas_path

results:
* [image] - struct [name; atlas_path; aw; ah; ox; oy; ow; oh; fw; fh; x0; y0; x1; y1; x2; y2; x3; y3]

# Astralax export
```sh
scripts/ae_movie_astralax_export.bat
```
arguments:
* in_file
* out_file
* write_name
* write_path

results:
* write_name
* write_path
* codec
* atlas_count
* [atlas_names] array [name]
* [atlas_paths] array [path]
* [atlas_codecs] array [codec]
* [atlas_premultiplies] array [premultiplie]
* [atlas_widths] array [width]
* [atlas_heights] array [height]

# Report export 
```sh
scripts/ae_movie_export_report.bat
```
arguments:
* log_path

# Sound export
```sh
scripts/ae_movie_sound_export.bat
```
arguments:
* in_path
* out_path
* write_name
* write_path

results:
* write_name
* write_path
* codec

# Image export
```sh
scripts/ae_movie_image_export.bat
```
arguments:
* in_path
* out_path
* write_name
* write_path

results:
* write_name
* write_path
* codec
* premultiplied
* base_width
* base_height
* trim_width
* trim_height
* offset_x
* offset_y

# Video export
```sh
scripts/ae_movie_video_export.bat
```
arguments:
* in_path
* out_path
* write_name
* write_path
* alpha

results:
* write_name
* write_path
* codec
* alpha
* frame_rate
* base_width
* base_height
* trim_width
* trim_height
* offset_x
* offset_y

# Image polygonize
```sh
scripts/ae_movie_image_polygonize.bat
```
arguments:
* image_path
* offset_x
* offset_y
* width
* height

results:
* vertex_count
* indices_count
* positions
* uvs
* indices

# Polygon triangulate
```sh
scripts/ae_movie_polygon_triangulate.bat
```
arguments:
* bb
* base_size - struct [width; height]
* trim_size - struct [width; height]
* trim_offset - struct [x; y]
* subtract
* points

results:
* data - struct [vertices_count, indices_count, [x, y], [u, v], [index]]

# Multiexport
```sh
scripts/ae_movie_multiexport_script.bat
```
arguments:
* export_config

results:
* atlas
* polygonize_sprite
* polygonize_square
* show_complete
* break_if_failed
* [in] filepath
* [out] filepath

# Settings
```sh
scripts/ae_movie_settings_script.bat
```
arguments:
* project_path
* project_name
* movie_name
* preview

results:
* api_key
* tool_folder
* export_path
* export_name
* atlas
* polygonize_sprite
* polygonize_square
* show_complete


