add_library(
    aether-task
    retrieve_forecast.cpp
    )
target_link_libraries(
    aether-task
    atlas-task
    aether-db
    aether-openweathermap
    )

