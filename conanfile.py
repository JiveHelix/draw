
from conans import ConanFile, CMake, tools


class DrawConan(ConanFile):
    name = "draw"
    version = "0.2.4"

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

    def build_requirements(self):
        self.test_requires("catch2/2.13.8")

    def requirements(self):
        self.requires("jive/[>=1.1.3 <1.2]")
        self.requires("fields/[~1.3]")
        self.requires("pex/[>=0.9.4 <0.10]")
        self.requires("tau/[~1.10]")
        self.requires("wxpex/[>=0.9.6 <0.10]")
