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
export PIO=`spack location -i mpas-o-scorpio`
export HDF5=`spack location -i hdf5`
export LOWFIVE=`spack location -i lowfive`
export HENSON=`spack location -i henson`
export USE_PIO2=true
export OPENMP=false
export HDF5_USE_FILE_LOCKING=FALSE
export MPAS_SHELL=/bin/bash
export CORE=ocean
export SHAREDLIB=true
export PROFILE_PRELIB="-L$HENSON/lib -lhenson-pmpi"

# set E3SM case setup env vars
export NETCDF_PATH=$NETCDF
export PNETCDF_PATH=$PNETCDF
export HDF5_ROOT=$HDF5
# export PIO_VERSION_MAJOR=2
# export PIO_LIBDIR="$PIO/lib"
# export PIO_INCDIR="$PIO/include"
# export PIO_TYPENAME_VALID_VALUES="netcdf"
export CC=mpicc
export CXX=mpicxx
export FC=mpif90
echo "environment variables are set for building E3SM"

# set load library paths for running
set LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$NETCDF/lib:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$NETCDF/lib64:$LD_LIBRARY_PATH  # for perlmutter
export LD_LIBRARY_PATH=$PNETCDF/lib:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$NETCDFF/lib:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$HDF5/lib:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$PIO/lib:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$LOWFIVE/lib:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$HENSON/lib:$LD_LIBRARY_PATH
echo "library paths are set for running E3SM"

# set python path
export SPACK=`spack location -r`
export PYTHONPATH=$SPACK/var/spack/environments/e3sm-env/.spack-env/view/lib/python3.11/site-packages:/pscratch/sd/t/tpeterka/software/spack/var/spack/environments/mpas/.spack-env/view/lib:$PYTHONPATH
export PATH=$SPACK/var/spack/environments/e3sm-env/.spack-env/view/bin:$PATH

# enable VOL plugin
unset HDF5_PLUGIN_PATH
unset HDF5_VOL_CONNECTOR
lowfive_module=`python3 -c "import lowfive; print(lowfive._lowfive.__file__)"`
lowfive_library=`ldd $lowfive_module | awk 'NF == 4 {print $3}; NF == 2 {print $1}' | grep lowfive`
export HDF5_PLUGIN_PATH=`dirname $lowfive_library`
export HDF5_VOL_CONNECTOR="lowfive under_vol=0;under_info={};"
echo "environment variables are set for running LowFive"

# give openMP 1 core for now to prevent using all cores for threading
# could set a more reasonable number to distribute cores between mpi + openMP
export OMP_NUM_THREADS=1

