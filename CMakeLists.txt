add_subdirectory(src)
add_subdirectory(web)

add_dependencies(aether-server-lib aether-web)

add_executable(aether src/main/aether.cpp)
target_link_libraries(
    aether
    aether-server-lib
    atlas-web
    )

add_executable(aether-kb-export src/main/export_knowledgebase.cpp)
target_link_libraries(
    aether-kb-export
    aether-server-lib
    )

add_executable(aether-kb-import src/main/import_knowledgebase.cpp)
target_link_libraries(
    aether-kb-import
    aether-server-lib
    )
