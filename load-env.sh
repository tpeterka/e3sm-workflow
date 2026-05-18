#!/bin/bash

# activate the environment
export SPACKENV=e3sm-env
spack env deactivate > /dev/null 2>&1
spack env activate $SPACKENV
echo "activated spack environment $SPACKENV"

# set spack locations and vars for building
export MPAS_EXTERNAL_LIBS=""
export MPAS_EXTERNAL_LIBS="${MPAS_EXTERNAL_LIBS} -lgomp"
export NETCDF=`spack location -i netcdf-c`
export NETCDFF=`spack location -i netcdf-fortran`
export PNETCDF=`spack location -i parallel-netcdf`
export HDF5=`spack location -i hdf5`
export LOWFIVE=`spack location -i lowfive`
export HENSON=`spack location -i henson`
export WILKINS=`spack location -i wilkins`
export USE_PIO2=true
export OPENMP=false
export HDF5_USE_FILE_LOCKING=FALSE
export MPAS_SHELL=/bin/bash
export CORE=ocean
export SHAREDLIB=true
export PROFILE_PRELIB="-L$HENSON/lib -lhenson-pmpi"

# optional when DIY and fmt are installed
export DIY_PATH=`spack location -i diy`
export FMT_PATH=`spack location -i fmt`

# set E3SM case setup env vars
export NETCDF_C_PATH=$NETCDF
export NETCDF_FORTRAN_PATH=$NETCDFF
export PNETCDF_PATH=$PNETCDF
export HDF5_ROOT=$HDF5
echo "environment variables are set for building E3SM"

# set load library paths for running
set LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$NETCDF/lib:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$NETCDF/lib64:$LD_LIBRARY_PATH  # for perlmutter
export LD_LIBRARY_PATH=$PNETCDF/lib:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$NETCDFF/lib:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$HDF5/lib:$LD_LIBRARY_PATH
# export LD_LIBRARY_PATH=$PIO/lib:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$LOWFIVE/lib:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$HENSON/lib:$LD_LIBRARY_PATH
echo "library paths are set for running E3SM"

# enable VOL plugin
unset HDF5_PLUGIN_PATH
unset HDF5_VOL_CONNECTOR
export HDF5_PLUGIN_PATH=$LOWFIVE/lib
export HDF5_VOL_CONNECTOR="lowfive under_vol=0;under_info={};"
echo "environment variables are set for running LowFive"

# give openMP 1 core for now to prevent using all cores for threading
# could set a more reasonable number to distribute cores between mpi + openMP
export OMP_NUM_THREADS=1

# load the spack installed python in the spack environment
spack load python

# following are for esm_watermasses
echo "installing python packages for esm_watermasses"
pip3 install fastjmd95
pip3 install pop-tools
export ESM_WATERMASSES=`spack location -i esm_watermasses`
# python3 -m pip install --no-deps --no-build-isolation -e $ESM_WATERMASSES


