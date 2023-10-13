cmake_minimum_required(VERSION 3.15)

project(draw CXX)

include(${CMAKE_CURRENT_LIST_DIR}/cmake_includes/setup_project.cmake)
setup_project()


add_subdirectory(draw)


if (${PROJECT_IS_TOP_LEVEL})
    option(
        BUILD_EXAMPLES
        "Build the example targets"
        ${ENABLES_DEFAULT})

endif ()


if (${PROJECT_IS_TOP_LEVEL})

    if (${BUILD_EXAMPLES})
        add_subdirectory(examples)
    endif ()

endif ()


if (ENABLE_TESTING)
    if (${PROJECT_IS_TOP_LEVEL})
        add_subdirectory(test)
    endif ()
endif ()
