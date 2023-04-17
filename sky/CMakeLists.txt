set(MODULE_ID sky)

find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(spdlog REQUIRED)
find_package(mist REQUIRED)

add_library(
    ${MODULE_ID}
    sky.cpp
    Color.cpp
    skyui.cpp
    tiles.cpp)

target_include_directories(
    ${MODULE_ID} PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
                        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")

target_link_libraries(
    ${MODULE_ID}
    PRIVATE project_warnings project_options spdlog::spdlog
    PUBLIC SDL2::SDL2
           SDL2_ttf::SDL2_ttf
           SDL2_image::SDL2_image
           mist::mist)

if (ENABLE_UTEST_NEVER)
    find_package(Catch2 REQUIRED)

    add_executable(utest_${MODULE_ID} test/utest_core.cpp)

    target_include_directories(utest_${MODULE_ID} PRIVATE ".")

    target_link_libraries(
        utest_${MODULE_ID}
        PRIVATE project_warnings
                project_options
                Catch2::Catch2WithMain
                ${MODULE_ID})

    include(CTest)
    include(Catch)
    catch_discover_tests(utest_${MODULE_ID} TEST_PREFIX "${MODULE_ID}@")
endif ()
