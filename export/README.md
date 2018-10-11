# Scripts

## Settings
```sh
scripts/ae_movie_settings.py
```
arguments:
*  project_path
*  project_name
*  movie_name
*  preview

results:
*  api_key
*  tool_folder
*  export_path
*  export_name
*  allow_atlas
*  atlas
*  image_polygonize
*  image_polygonize_tolerance
*  image_polygonize_minimum_square
*  allow_image_premultiplied
*  image_premultiplied
*  allow_sha1
*  use_sha1
*  show_complete
*  disable_script_export_astralax
*  disable_script_export_image
*  disable_script_export_sound
*  disable_script_export_video

## Image export
```sh
scripts/ae_movie_export_image.py
```
arguments:
*  in_path
*  out_path
*  project_name
*  write_name
*  write_path
*  trim
*  in_premultiplied
*  out_premultiplied

results:
*  out_path
*  write_name
*  write_path
*  codec
*  premultiplied
*  base_width
*  base_height
*  trim_width
*  trim_height
*  offset_x
*  offset_y

## Sound export
```sh
scripts/ae_movie_export_sound.py
```
arguments:
*  in_path
*  out_path
*  out_work
*  project_name
*  write_name
*  write_path
*  use_sha1

results:
*  write_name
*  write_path
*  codec

## Video export
```sh
scripts/ae_movie_export_video.py
```
arguments:
*  in_path
*  out_path
*  out_work
*  project_name
*  write_name
*  write_path
*  alpha
*  use_sha1

results:
*  write_name
*  write_path
*  codec
*  alpha
*  frame_rate
*  base_width
*  base_height
*  trim_width
*  trim_height
*  offset_x
*  offset_y

## Astralax export
```sh
scripts/ae_movie_export_astralax.py
```
arguments:
*  in_path
*  out_path
*  out_work
*  project_name
*  write_name
*  write_path
*  use_sha1

results:
*  write_name
*  write_path
*  codec
*  atlas_count
*  [atlas_names] array [name]
*  [atlas_paths] array [path]
*  [atlas_codecs] array [codec]
*  [atlas_premultiplies] array [premultiplie]
*  [atlas_widths] array [width]
*  [atlas_heights] array [height]

## Image polygonize
```sh
scripts/ae_movie_image_polygonize.py
```
arguments:
*  in_path
*  offset_x
*  offset_y
*  width
*  height

results:
*  vertex_count
*  index_count
*  [positions] array [position]
*  [uvs] array [uv]
*  [indices] array [index]

## Polygon triangulate
```sh
scripts/ae_movie_polygon_triangulate.py
```
arguments:
*  bb
*  base_width
*  base_height
*  trim_width
*  trim_height
*  trim_offset_x
*  trim_offset_y
*  subtract
*  [points] array [point]

results:
*  vertex_count
*  index_count
*  [positions] array [position] 
*  [uvs] array [uv]
*  [indices] array [index]

## Atlas export
```sh
scripts/ae_movie_atlas.py
```
arguments:
*  in_path
*  out_path
*  image_count
*  premultiply

results:
* [image_data] array ostruct [name; atlas_path; aw; ah; ox; oy; ow; oh; fw; fh; x0; y0; x1; y1; x2; y2; x3; y3; rotate]

## Prebuild
```sh
scripts/ae_movie_prebuild.py
```
arguments:
*  movie_folder
*  movie_name
*  project_name
*  preview

results:

## Postbuild
```sh
scripts/ae_movie_postbuild.py
```
arguments:
*  movie_folder
*  movie_name
*  project_name
*  hash_crc
*  preview
*  successful

results:

## Preview
```sh
scripts/ae_movie_preview.py
```
arguments:
*  project_path
*  movie_name
*  project_name
*  composition_name
*  hash_crc
*  work_area

results:

## Multiexport
```sh
scripts/ae_movie_multiexport.py
```
arguments:
*  cfg_path

results:
*  atlas
*  image_premultiplied
*  image_polygonize
*  image_polygonize_square
*  show_complete
*  break_if_failed
*  [in] filepath
*  [out] filepath

## Multiexport prebuild
```sh
scripts/ae_movie_multiexport_prebuild.py
```
arguments:
*  image_polygonize

results:

## Multiexport postbuild
```sh
scripts/ae_movie_multiexport_postbuild.py
```
arguments:
*  successful

results:

## Multiexport report
```sh
scripts/ae_movie_multiexport_report.py
```
arguments:
*  log_path

results:

## Multiexport report
```sh
scripts/ae_movie_check_settings.py
```
arguments:

results:
*  check_root_file
*  check_time




















