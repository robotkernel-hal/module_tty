from conans import ConanFile, tools
import os

class MainProject(ConanFile):
    python_requires = "conan_template_ln_generator/[~=5 >=5.0.7]@robotkernel/stable"
    python_requires_extend = "conan_template_ln_generator.RobotkernelLNGeneratorConanFile"
    
    name = "module_tty"
    description = "robotkernel-5 is a modular, easy configurable hardware abstraction framework"
    exports_sources = ["*", "!.gitignore"] + ["!%s" % x for x in tools.Git().excluded_files()]
    requires = (
            "robotkernel/[~=5]@robotkernel/stable",
            "service_provider_process_data_inspection/[~=5]@robotkernel/stable",
            "service_provider_memory_inspection/[~=5]@robotkernel/stable" )

