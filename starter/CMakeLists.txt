set(MODULE_ID starter)

find_package(spdlog REQUIRED)
find_package(mist REQUIRED)

add_executable(${MODULE_ID}
    src/main.cpp)

target_link_libraries(${MODULE_ID} 
    PRIVATE
        project_warnings
        project_options
        sky
        mist::mist
        spdlog::spdlog)
