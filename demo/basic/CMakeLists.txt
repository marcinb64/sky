set(MODULE_ID basic_demo)

find_package(spdlog REQUIRED)
find_package(mist REQUIRED)

add_executable(${MODULE_ID}
    main.cpp
    ${COMMON_SOURCES})

target_include_directories(${MODULE_ID} PRIVATE ../common)

target_link_libraries(${MODULE_ID}
    PRIVATE 
        project_warnings
        project_options
        sky
        mist::mist
        spdlog::spdlog)
                                                                             