file(GLOB aether_openweathermap_sources *.cpp)
add_library(aether-openweathermap ${aether_openweathermap_sources})
target_link_libraries(
    aether-openweathermap
    aether-db
    atlas-com-http
    ${CMAKE_THREAD_LIBS_INIT}
    ${Boost_LIBRARIES}
    )

file(
    GLOB
    aether_openweathermap_test_sources
    test/*.cpp
    )
add_executable(aether-openweathermap-test ${aether_openweathermap_test_sources})
add_test(aether-openweathermap-test aether-openweathermap-test)
target_link_libraries(
    aether-openweathermap-test
    aether-openweathermap
    atlas-com-common
    styx
    hades
    ${CMAKE_THREAD_LIBS_INIT}
    ${Boost_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    )

