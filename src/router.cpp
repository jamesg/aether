#include "aether/router.hpp"

#include "aether/db.hpp"
#include "atlas/http/server/static_string.hpp"

#define AETHER_DECLARE_STATIC_STRING(NAME) ATLAS_DECLARE_STATIC_STRING(aether, NAME)
#define AETHER_STATIC_STD_STRING(NAME) ATLAS_STATIC_STD_STRING(aether, NAME)

AETHER_DECLARE_STATIC_STRING(index_html)
AETHER_DECLARE_STATIC_STRING(index_js)
AETHER_DECLARE_STATIC_STRING(models_js)

aether::router::router(hades::connection& conn)
{
    install_static_text("/", "html", AETHER_STATIC_STD_STRING(index_html));
    install_static_text("/index.html", AETHER_STATIC_STD_STRING(index_html));
    install_static_text("/index.js", AETHER_STATIC_STD_STRING(index_js));
    install_static_text("/models.js", AETHER_STATIC_STD_STRING(models_js));
}

