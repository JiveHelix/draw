
from conans import ConanFile, CMake, tools


class DrawConan(ConanFile):
    name = "draw"
    version = "0.1.0"

    scm = {
        "type": "git",
        "url": "https://github.com/JiveHelix/draw.git",
        "revision": "auto",
        "submodule": "recursive"}

    license = "MIT"
    author = "Jive Helix (jivehelix@gmail.com)"
    url = "https://github.com/JiveHelix/draw"
    description = "Drawing tools"
    topics = ("Vector Drawing", "Graphics", "C++")

    settings = "os", "compiler", "build_type", "arch"

    generators = "cmake"

    no_copy_source = True

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = ["draw"]

        if self.settings.os == 'Windows' \
                and self.settings.build_type != 'Debug':
            self.cpp_info.defines.append('NDEBUG')

    def build_requirements(self):
        self.test_requires("catch2/2.13.8")

    def requirements(self):
        self.requires("jive/[~1.1]")
        self.requires("fields/[~1.3]")
        self.requires("tau/[~1.9]")
        self.requires("pex/[>=0.9.2]")
        self.requires("wxpex/[~0.8]")
