# Copyright Spack Project Developers. See COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)

from spack_repo.builtin.build_systems.python import PythonPackage

from spack.package import *


class EsmWatermasses(Package):
    """A water mass analysis package for gridded ocean and atmospheric data and Earth system model output"""

    homepage = "https://github.com/SciDAC-ImPACTS/esm_watermasses"
    url = "https://github.com/SciDAC-ImPACTS/esm_watermasses.git"
    git = "https://github.com/SciDAC-ImPACTS/esm_watermasses.git"

    license("BSD-3")

    version('main', branch='main')

    def install(self, spec, prefix):
        # install_tree copies all files from source stage to prefix
        install_tree('.', prefix)
