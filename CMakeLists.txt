cmake_minimum_required(VERSION 3.14)

# Project definition
project(
    Sky
    VERSION 0.1
    LANGUAGES CXX)

# Build customization
option(WARNINGS_AS_ERRORS "Treat compiler warnings as errors" On)
option(ENABLE_CLANG_TIDY "Run clang-tidy during build" Off)
option(ENABLE_SANITIZERS "Enable sanitizers" Off)

# Helpers
include(CompilerWarnings.cmake)
include(Sanitizers.cmake)
include(StaticAnalysis.cmake)

# Project-specific settings
add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE cxx_std_20)
add_sanitizers(project_options)

# Pseudo-library for adding compiler warning flags to targets
add_library(project_warnings INTERFACE)
set_project_warnings(project_warnings)

# Misc options
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(CTest)

# Project structure
add_subdirectory(app)
add_subdirectory(sky)
