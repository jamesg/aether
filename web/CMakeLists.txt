atlas_add_web_library(
    PREFIX aether
    TARGET aether-web
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/static.a
    SOURCES
        aether.css
        application.js
        board.html
        board.js
        chart.js
        index.html
        index.js
        knowledgebase.html
        knowledgebase.js
        knowledgebase_editor.html
        knowledgebase_editor.js
        models.js
        palette.html
        palette.js
        sensors.html
        sensors.js
        settings.html
        settings.js
        weather.html
        weather.js
        )
