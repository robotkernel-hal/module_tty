import os
import shutil

from conan import ConanFile
from conan.tools.files import copy
from conan.tools.build import can_run

class TestTestConan(ConanFile):
    test_type = "explicit"
    settings = "os", "compiler", "build_type", "arch"
    exports_sources = "mod_test.rkc"

    generators = "VirtualRunEnv"

    def requirements(self):
        self.requires(self.tested_reference_str)

    def test(self):
        if self.source_folder != self.build_folder:
            copy(self, 'mod_test.rkc', self.source_folder, self.build_folder)

        if can_run(self):
            self.run("robotkernel --test-run --config .%smod_test.rkc" % os.sep, env="conanrun")
        else:
            self.output.warn("Skipping run cross built package")

