
from cmake_includes.conan import LibraryConanFile


class DrawConan(LibraryConanFile):
    name = "draw"
    version = "0.3.0"

    license = "MIT"
    author = "Jive Helix (jivehelix@gmail.com)"
    url = "https://github.com/JiveHelix/draw"
    description = "Drawing tools"
    topics = ("Vector Drawing", "Graphics", "C++")


    def build_requirements(self):
        self.test_requires("catch2/2.13.8")

    def requirements(self):
        self.requires("jive/[~1.3]", transitive_headers=False)
        self.requires("fields/[~1.4]", transitive_headers=False)
        self.requires("pex/[~1.0]", transitive_headers=False)
        self.requires("tau/[~1.11]", transitive_headers=False)
        self.requires("wxpex/[~1.0]", transitive_headers=False)
        self.requires("libpng/[~1.6]")
