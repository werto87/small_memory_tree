from conan import ConanFile


class Project(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"

    def configure(self):
        self.options["catch2"].with_main = True
        self.options["catch2"].with_benchmark = True

    def requirements(self):
        self.requires("boost/1.84.0", force=True)
        self.requires("st_tree/1.2.1")
        self.requires("catch2/2.13.7")
        self.requires("confu_algorithm/0.0.1")
