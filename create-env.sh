#!/bin/bash

unset PROFILE_PRELIB

export SPACKENV=e3sm-env
export YAML=$PWD/env.yaml

# create spack environment
echo "creating spack environment $SPACKENV"
spack env deactivate > /dev/null 2>&1
spack env remove -y $SPACKENV > /dev/null 2>&1
spack env create $SPACKENV $YAML

# activate environment
echo "activating spack environment"
spack env activate $SPACKENV

spack add mpich
spack add hdf5@1.14+hl+mpi
# spack develop lowfive
spack add lowfive
spack add wilkins ^python@3.13
# spack develop henson+python+mpi-wrappers
spack add henson+python+mpi-wrappers ^python@3.13
spack add netcdf-c@4.9+mpi
spack add parallel-netcdf
spack add netcdf-fortran@4.5.3
spack add py-netcdf4+mpi ^python@3.13
# following are optional for testing c++ consumer
spack add diy@master
spack add fmt

# following are for esm_watermasses
spack add py-numpy ^python@3.13
spack add py-scipy ^python@3.13
spack add py-pyyaml ^python@3.13
spack add py-xarray ^python@3.13
spack add py-tqdm ^python@3.13
spack add nco ^python@3.13
spack add py-pip ^python@3.13
spack add esm_watermasses

# install everything in environment
echo "installing dependencies in environment"
spack install henson        # install henson so that henson path is set
export HENSON=`spack location -i henson`
spack install

spack env deactivate
