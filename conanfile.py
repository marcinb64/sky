from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout


class HelloConan(ConanFile):
    name = "sky"
    version = "0.1.1"

    # Optional metadata
    license = "MIT"
    author = "marcinb64@gmail.com"
    url = "https://github.com/marcinb64/sky"
    description = "2D game dev library based on libSDL, with C++ interface."
    topics = ("game")

    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False]}
    default_options = {"shared": False, "fPIC": True}

    # Sources are located in the same place as this recipe, copy them to the recipe
    exports_sources = "CMakeLists.txt", "*.cmake", "sky/*", "demo/*", "starter/*"
    generators = "CMakeDeps"

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def requirements(self):
        self.requires("mist/0.1.7")
        self.requires("sdl/2.26.5")
        self.requires("sdl_ttf/2.20.1")
        self.requires("sdl_image/2.0.5")
        self.requires("libpng/1.6.40")
        self.requires("spdlog/1.10.0")

    def layout(self):
        cmake_layout(self)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure(variables={ "ENABLE_SANITIZERS" : "Off" })
        cmake.build(target='sky')

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = ["sky"]
