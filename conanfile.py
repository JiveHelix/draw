
from conan import ConanFile


class DrawConan(ConanFile):
    name = "draw"
    version = "0.4.0"

    python_requires = "boiler/0.2"
    python_requires_extend = "boiler.LibraryConanFile"

    license = "MIT"
    author = "Jive Helix (jivehelix@gmail.com)"
    url = "https://github.com/JiveHelix/draw"
    description = "Drawing tools"
    topics = ("Vector Drawing", "Graphics", "C++")


    def build_requirements(self):
        self.test_requires("catch2/2.13.8")

    def requirements(self):
        self.requires("jive/[>=1.7 <2]", transitive_headers=False)
        self.requires("fields/[>=1.8 <2]", transitive_headers=False)
        self.requires("pex/[>=1.4 <2]", transitive_headers=False)
        self.requires("tau/[>=1.16 <2]", transitive_headers=False)
        self.requires("wxpex/[>=1.0 <2]", transitive_headers=False)
        self.requires("libpng/[~1.6]", transitive_headers=True)
