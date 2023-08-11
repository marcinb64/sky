set(MODULE_ID sky)

find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(spdlog REQUIRED)
find_package(mist REQUIRED)

add_library(
    ${MODULE_ID}
    src/Sky.cpp
    src/Color.cpp
    src/SkyUi.cpp
    src/Tiles.cpp)

file(GLOB HEADER_FILES src/*.h)

target_include_directories(
    ${MODULE_ID} PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/src/"
                        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>/sky/")

target_link_libraries(
    ${MODULE_ID}
    PRIVATE project_warnings project_options spdlog::spdlog
    PUBLIC SDL2::SDL2
           sdl_ttf::sdl_ttf
           SDL2_image::SDL2_image
           mist::mist)

install(TARGETS ${MODULE_ID})
install(FILES ${HEADER_FILES} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/mist/)

if (BUILD_TESTING)
    find_package(Catch2 REQUIRED)

    add_executable(utest_${MODULE_ID} test/utest_dummy.cpp)

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
