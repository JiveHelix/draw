add_library(draw)

if (${fPIC})
    set_property(TARGET draw PROPERTY POSITION_INDEPENDENT_CODE ON)
endif ()

find_package(Jive REQUIRED)
find_package(Fields REQUIRED)
find_package(Pex REQUIRED)
find_package(Tau REQUIRED)
find_package(Wxpex REQUIRED)
find_package(PNG REQUIRED)
find_package(Fmt REQUIRED)
find_package(Nlohmann_json REQUIRED)

# Projects that include this project must #include "draw/<header-name>"
target_include_directories(draw PUBLIC ${PROJECT_SOURCE_DIR})

target_link_libraries(
    draw
    PUBLIC
    wxpex::wxpex
    tau::tau
    pex::pex
    fields::fields
    jive::jive
    PNG::PNG
    fmt::fmt
    nlohmann_json::nlohmann_json)

target_sources(
    draw
    PRIVATE
    bitmap.h
    cross.h
    cross_shape.h
    drag.h
    draw_context.h
    draw_lines.h
    draw_segments.h
    draw_spline.h
    edge.h
    edge_shape.h
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
    regular_polygon.h
    regular_polygon_shape.h
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
    shape_creator.cpp
    shape_editor.cpp
    size.h
    waveform.h
    waveform_generator.h
    waveform_settings.h
    detail/png_image.h
    detail/poly_shape_id.h
    views/affine_view.h
    views/bitmap_canvas.h
    views/bitmap_view.h
    views/canvas.h
    views/canvas_settings.h
    views/canvas_view.h
    views/color_map_settings_view.h
    views/cross_shape_view.h
    views/cross_view.h
    views/ellipse_shape_view.h
    views/ellipse_view.h
    views/font_look_view.h
    views/lines_shape_view.h
    views/look_view.h
    views/node_settings_view.h
    views/order_view.h
    views/pixel_canvas.h
    views/pixel_view.h
    views/pixel_view_settings.h
    views/points_shape_view.h
    views/point_view.h
    views/polygon_view.h
    views/regular_polygon_view.h
    views/quad_view.h
    views/size_view.h
    views/view_settings.h
    views/waveform_settings_view.h
    views/waveform_view.h
    bitmap.cpp
    cross.cpp
    cross_shape.cpp
    draw_context.cpp
    draw_segments.cpp
    draw_spline.cpp
    edge.cpp
    edge_shape.cpp
    ellipse.cpp
    oddeven.cpp
    font_look.cpp
    lines_shape.cpp
    look.cpp
    points_shape.cpp
    polygon.cpp
    regular_polygon.cpp
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
    views/bitmap_canvas.cpp
    views/bitmap_view.cpp
    views/canvas_settings.cpp
    views/cross_shape_view.cpp
    views/cross_view.cpp
    views/ellipse_shape_view.cpp
    views/ellipse_view.cpp
    views/font_look_view.cpp
    views/lines_shape_view.cpp
    views/look_view.cpp
    views/node_settings_view.cpp
    views/order_view.cpp
    views/pixel_canvas.cpp
    views/pixel_view.cpp
    views/pixel_view_settings.cpp
    views/points_shape_view.cpp
    views/polygon_view.cpp
    views/regular_polygon_view.cpp
    views/quad_view.cpp
    views/view_settings.cpp
    views/waveform_settings_view.cpp
    views/waveform_view.cpp)


install(TARGETS draw DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(
    DIRECTORY ${PROJECT_SOURCE_DIR}/draw
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
