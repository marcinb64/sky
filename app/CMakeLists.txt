set(MODULE_ID sandbox)

find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(spdlog REQUIRED)
find_package(mist REQUIRED)

add_executable(${MODULE_ID}
  main.cpp
  tile_demo/tiledemo.cpp
  tile_demo/TileEditor.cpp
  tile_demo/Chunk.cpp
  mist/MistDemo.cpp
  mist/Terraform.cpp
  )

target_link_libraries(${MODULE_ID} PRIVATE
  project_warnings
  project_options
  sky
  mist::mist
  spdlog::spdlog
  )
