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

## Generating an E3SM ocean test case

Edit the template in `ccase1.sh` according to the instructions [here](https://docs.e3sm.org/running-e3sm-guide/guide-prior-to-production/)
Set the `MACHINE`, `PROJECT`, `CASE_NAME`, `CASE_ROOT`, `CODE_ROOT`.
For the first time, set the `do_*` flags as follows:
```
do_fetch_code=true
do_create_newcase=true
do_case_setup=true
do_case_build=true
do_case_submit=false
```
Subsequent times, set various flags, eg. `do_fetch_code`, to `false`.

Run the script:
```
./run.ccase1.sh
```

-----

# create case manually

```
/path/to/E3SMv3/code/latest/cime/scripts/create_newcase --case ccase1 --output-root "${CASEDIR}" --handle-preexisting-dirs u --compset CMPASO-JRA1p4 --res TL319_IcoswISC30E3r5 --machine pm-cpu --compiler gnu

```

-----

# Modify environment for building E3SM

Edit `<CASE_ROOT>/tests/<run>/case_scripts/.env_mach_specific (where
CASE_ROOT and run (eg. XS_1x10_ndays) were provided in the run script above):

TBD

-----

# Rebuild E3SM in the spack environment

```
source /path/to/e3sm-workflow/load-env.sh
cd <CASE_ROOT>/tests/<run>/case_scripts
./case_build --clean-all
./case_build
```
The build log and executable are located in `<CASE_ROOT>/build`.

