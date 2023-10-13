add_library(draw_catch_main STATIC catch_main.cpp)
target_link_libraries(draw_catch_main PUBLIC CONAN_PKG::catch2)
target_link_libraries(draw_catch_main PRIVATE project_options)

add_executable(
    draw_tests
    view_tests.cpp)

target_link_libraries(
    draw_tests
    PRIVATE
    project_warnings
    project_options
    draw_catch_main
    draw)

add_test(NAME DrawTests COMMAND ${CMAKE_BINARY_DIR}/bin/draw_tests)
