file(GLOB server_sources *.cpp)
add_library(aether-server-lib ${server_sources})
target_link_libraries(
    aether-server-lib
    atlas-com-http-server
    hades
    ${CMAKE_CURRENT_BINARY_DIR}/../web/static.a
    ${CMAKE_THREAD_LIBS_INIT}
    ${Boost_LIBRARIES}
    )

