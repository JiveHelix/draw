add_subdirectory(common)


if (MSVC)
    set(windowed WIN32)
elseif (APPLE)
    set(windowed MACOSX_BUNDLE)
endif ()


add_executable(polygon_demo polygon_demo.cpp)

target_link_libraries(
    polygon_demo
    PRIVATE
    project_warnings
    project_options
    common)


add_executable(poly_demo poly_demo.cpp)

target_link_libraries(
    poly_demo
    PRIVATE
    project_warnings
    project_options
    common)


add_executable(ellipse_demo ellipse_demo.cpp)

target_link_libraries(
    ellipse_demo
    PRIVATE
    project_warnings
    project_options
    common)


add_executable(segments_demo segments_demo.cpp)

target_link_libraries(
    segments_demo
    PRIVATE
    project_warnings
    project_options
    common)


add_executable(quad_demo quad_demo.cpp)

target_link_libraries(
    quad_demo
    PRIVATE
    project_warnings
    project_options
    common)


add_executable(waveform_demo waveform_demo.cpp)

target_link_libraries(
    waveform_demo
    PRIVATE
    project_warnings
    project_options
    common)


add_executable(png_demo png_demo.cpp)

target_link_libraries(
    png_demo
    PRIVATE
    project_warnings
    project_options
    common)
