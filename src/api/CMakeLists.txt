include_directories("../../include/")
aux_source_directory(. src)
add_library(api ${src})