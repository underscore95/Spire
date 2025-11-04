
# Defines ASSETS_DIRECTORY macro
# In debug mode, this is an absolute path to the asset directory in the project
# In release mode, the project assets are copied into a directory in this structure:
# bin/
# - Game.exe
# - Assets/
# - - player.png
# and ASSETS_DIRECTORY would be a relative path to the assets directory from the default working directory

function(setup_assets_directory TARGET_NAME ASSETS_DIRECTORY_NAME)
    # Macro
    if (CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_definitions(ASSETS_DIRECTORY="${ASSETS_DIRECTORY_NAME}")
    else()
        add_compile_definitions(ASSETS_DIRECTORY="${CMAKE_CURRENT_SOURCE_DIR}/${ASSETS_DIRECTORY_NAME}")
    endif()

    # Copy to build folder in release
    if (CMAKE_BUILD_TYPE STREQUAL "Release")
        file(REMOVE_RECURSE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${ASSETS_DIRECTORY_NAME})
        add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_CURRENT_SOURCE_DIR}/${ASSETS_DIRECTORY_NAME}
                ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${ASSETS_DIRECTORY_NAME}
        )
    endif()
endfunction()
