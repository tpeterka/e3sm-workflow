# e3sm-workflow

# Instructions for Building and Running E3SM (ocean component) to Run in a Workflow

Installation is done through Spack.
If you don't have Spack installed or if Spack is new to you, go [here](https://spack.readthedocs.io/en/latest/) first.
The recommended compiler is gcc version 12.

-----

## For Perlmutter:

Add to your `~/.bash_profile`:
The version of mpi included with the Cray programming environment is too old.
Use mpich built by me, and unload the Cray programming environment module.
Match the compiler that I used to build my mpich.
Also recommend to initialize your spack installation.
```
module unload PrgEnv-gnu/8.5.0
module load gcc-native/12.3
source /path/to/spack/share/spack/setup-env.sh
export PATH=/pscratch/sd/t/tpeterka/software/mpich-4.3.0/install/bin:$PATH
export LD_LIBRARY_PATH=/pscratch/sd/t/tpeterkasoftware/mpich-4.3.0/install/lib:$LD_LIBRARY_PATH
```
Edit `~/.spack/packages.yaml` to use gcc 12.3.0 and my pre-installed mpich:
```
packages:
  gcc:
    externals:
    - spec: gcc@12.3.0 languages='c,c++,fortran'
      prefix: /usr
      extra_attributes:
        compilers:
          c: /usr/bin/gcc-12
          cxx: /usr/bin/g++-12
          fortran: /usr/bin/gfortran-12
  mpich:
    externals:
    - spec: mpich@4
      prefix: /pscratch/sd/t/tpeterka/software/mpich-4.3.0/install
      extra_attributes:
        environment:
          prepend_path:
            LD_LIBRARY_PATH: /pscratch/sd/t/tpeterka/software/mpich-4.3.0/install/lib:/opt/cray/libfabric/1.20.1/lib64
    buildable: False
```
-----

## Cloning this repository

```
git clone https://github.com/tpeterka/e3sm-workflow
```

-----

## Adding the following Spack repositories to your local Spack installation

No environment should be active. Run `spack env status` to be sure.

LowFive
```
git clone https://github.com/diatomic/LowFive
spack repo add LowFive
```

Wilkins
```
git clone https://github.com/orcunyildiz/wilkins
spack repo add wilkins
```

Mpas-o-scorpio
```
spack repo add /path_to/e3sm-workflow/mpas-o-scorpio
```

-----

## Setting up Spack environment

### First time: create and load the Spack environment for MPAS-Ocean

```
cd /path_to/e3sm-workflow
source ./create-env.sh             # requires being in the same directory to work properly
source ./load-env.sh
```

### Subsequent times: load the Spack environment for MPAS-Ocean

```
source /path_to/e3sm-workflow/load-env.sh
```

-----


