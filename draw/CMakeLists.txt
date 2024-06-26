add_library(draw)

if (${fPIC})
    set_property(TARGET draw PROPERTY POSITION_INDEPENDENT_CODE ON)
endif ()

# Projects that include this project must #include "draw/<header-name>"
target_include_directories(draw PUBLIC ${PROJECT_SOURCE_DIR})

target_link_libraries(
    draw
    PUBLIC
    CONAN_PKG::jive
    CONAN_PKG::fields
    CONAN_PKG::tau
    CONAN_PKG::pex
    CONAN_PKG::libpng
    CONAN_PKG::wxpex
    CONAN_PKG::fmt
    CONAN_PKG::nlohmann_json)

target_sources(
    draw
    PRIVATE
    cross.h
    cross_shape.h
    drag.h
    draw_ellipse.h
    draw_lines.h
    draw_segments.h
    ellipse.h
    ellipse_shape.h
    error.h
    font_look.h
    lines_shape.h
    look.h
    oddeven.h
    pixels.h
    planar.h
    png.h
    point.h
    points_shape.h
    polygon.h
    polygon_brain.h
    polygon_lines.h
    polygon_shape.h
    quad.h
    quad_brain.h
    quad_lines.h
    quad_shape.h
    scale.h
    segments_shape.h
    shapes.h
    shape_edit.cpp
    size.h
    waveform.h
    waveform_generator.h
    waveform_settings.h
    detail/png_image.h
    detail/poly_shape_id.h
    views/affine_view.h
    views/cross_shape_view.h
    views/cross_view.h
    views/ellipse_shape_view.h
    views/ellipse_view.h
    views/font_look_view.h
    views/lines_shape_view.h
    views/look_view.h
    views/node_settings_view.h
    views/pixel_canvas.h
    views/pixel_view.h
    views/pixel_view_settings.h
    views/points_shape_view.h
    views/point_view.h
    views/polygon_view.h
    views/quad_view.h
    views/size_view.h
    views/view_settings.h
    views/waveform_settings_view.h
    views/waveform_view.h
    cross.cpp
    cross_shape.cpp
    draw_segments.cpp
    ellipse.cpp
    ellipse_shape.cpp
    oddeven.cpp
    font_look.cpp
    lines_shape.cpp
    look.cpp
    points_shape.cpp
    polygon.cpp
    polygon_lines.cpp
    png.cpp
    quad.cpp
    quad_brain.cpp
    quad_lines.cpp
    segments_shape.cpp
    shapes.cpp
    waveform.cpp
    waveform_generator.cpp
    waveform_settings.cpp
    detail/png_image.cpp
    detail/poly_shape_id.cpp
    views/cross_shape_view.cpp
    views/cross_view.cpp
    views/ellipse_shape_view.cpp
    views/ellipse_view.cpp
    views/font_look_view.cpp
    views/lines_shape_view.cpp
    views/look_view.cpp
    views/node_settings_view.cpp
    views/pixel_canvas.cpp
    views/pixel_view.cpp
    views/pixel_view_settings.cpp
    views/points_shape_view.cpp
    views/polygon_view.cpp
    views/quad_view.cpp
    views/view_settings.cpp
    views/waveform_settings_view.cpp
    views/waveform_view.cpp)


install(TARGETS draw DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(
    DIRECTORY ${PROJECT_SOURCE_DIR}/draw
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
