#include "tile_demo/tiledemo.h"
#include "mist/MistDemo.h"
#include <spdlog/spdlog.h>

/* -------------------------------------------------------------------------- */

int main()
{
    spdlog::set_level(spdlog::level::debug);

    try {
        //demo::tileeditor();
        demo::mistDemo();
    }

    catch (std::exception &e) {
        spdlog::critical(e.what());
    }

    return 0;
}
