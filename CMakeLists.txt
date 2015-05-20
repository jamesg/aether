add_subdirectory(src)
add_subdirectory(web)

add_dependencies(aether-server-lib aether-web)

add_executable(aether src/main/aether.cpp)
target_link_libraries(
    aether
    aether-server-lib
    atlas-web
    )

