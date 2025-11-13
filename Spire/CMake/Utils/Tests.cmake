function(AddTests TestExecutable TestedLibrary)
    enable_testing()

    target_include_directories(${TestExecutable} PRIVATE
            ${LIBS_DIRECTORY}/googletest/googletest/include
    )

    target_link_libraries(${TestExecutable} PRIVATE
            ${BUILD_MODE_DIR}/gtest.lib
            ${BUILD_MODE_DIR}/gtest_main.lib
    )

    include(GoogleTest)
    gtest_discover_tests(${TestExecutable})

    add_custom_target(AutomaticallyRun${TestExecutable}OnBuild ALL
            COMMAND $<TARGET_FILE:${TestExecutable}>
            DEPENDS ${TestedLibrary} ${TestExecutable}
            COMMENT "Running ${TestExecutable} after build..."
    )
endfunction()
