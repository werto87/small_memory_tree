from conan import ConanFile
from conan.tools.cmake import CMakeToolchain

class Project(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators =  "CMakeDeps"


    def generate(self):
        tc = CMakeToolchain(self)
        tc.user_presets_path = False #workaround because this leads to useless options in cmake-tools configure
        tc.generate()

    def configure(self):
        self.options["catch2"].with_main = True
        self.options["catch2"].with_benchmark = True
        self.options["boost"].header_only = True

    def requirements(self):
        self.requires("boost/1.84.0", force=True)
        self.requires("st_tree/1.2.2")
        self.requires("catch2/2.13.7")
        self.requires("confu_algorithm/1.1.1")
        self.requires("stlplus/3.16.0")
        
