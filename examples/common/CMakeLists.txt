add_library(common)

if (${fPIC})
    set_property(TARGET common PROPERTY POSITION_INDEPENDENT_CODE ON)
endif ()

# Projects that include this project must #include "common/<header-name>"
target_include_directories(common PUBLIC ${PROJECT_SOURCE_DIR})

target_link_libraries(
    common
    PUBLIC
    draw)

target_sources(
    common
    PRIVATE
    about_window.cpp
    user.cpp
    display_error.cpp)


add_version_header(common)
