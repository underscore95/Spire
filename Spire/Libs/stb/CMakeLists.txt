cmake_minimum_required(VERSION 3.31)

function(include_stb_to_target target_name)
    set(stb_dir "${LIBS_DIRECTORY}/stb/stb")
    target_include_directories(${target_name} PRIVATE "${stb_dir}")

    file(GLOB_RECURSE stb_headers "${stb_dir}/*.h")
    target_sources(${target_name} PRIVATE ${stb_headers})
endfunction()
