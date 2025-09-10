# Instructions for Building E3SM (ocean component) and Running in a Workflow

This repository provides an example of building and running E3SM coupled with a Python analysis task in an HPC in situ workflow.
The E3SM code is an ocean test case, and the Python analysis code simply prints one of the ocean output variables.
The data transfer can be configured to either read/write files or exchange MPI messages through the system interconnect.
The workflow is intended as a template to copy and modify to suit specific cases.

These instructions have been tested on the Perlmutter CPU partition at NERSC.
Other machines will be similar, but not tested.
Instructions that are specific to Perlmutter have been noted.

Installation is done through Spack.
If you don't have Spack installed or if Spack is new to you, go [here](https://spack.readthedocs.io/en/latest/) first.
The recommended compiler is gcc version 12.

The instructions in this README are divided into the following main steps:

- Preliminaries
- Cloning this repository and setting up a spack environment
- Cloning the E3SM repository and setting up an ocean test case
- Building E3SM to run in a workflow
- Testing the E3SM build
- Configuring the workflow
- Running the workflow

-----

## Preliminaries

Preliminary steps include setting up your shell environment, loading/unloading
modules, and configuring your spack installation.

### Modify your bash profile

(For Perlmutter)

The version of MPI included with the Cray programming environment is too old.
Use mpich built by me, and unload the Cray programming environment module.
Match the compiler that I used to build my mpich.
This is also a good place to initialize your spack installation.

Add to your `~/.bash_profile` or `~/.bashrc`:

```
module unload PrgEnv-gnu/8.5.0
module load gcc-native/12.3
export PATH=/pscratch/sd/t/tpeterka/software/mpich-4.3.0/install/bin:$PATH
export LD_LIBRARY_PATH=/pscratch/sd/t/tpeterkasoftware/mpich-4.3.0/install/lib:$LD_LIBRARY_PATH

source /path/to/spack/share/spack/setup-env.sh
```
### Edit `~/.spack/packages.yaml` to use gcc 12.3.0 and my pre-installed mpich

(For Perlmutter)

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

### Add symlinks for compiler wrappers pointing to my mpich installation

(For Perlmutter)


Add a `bin` directory to your `$PATH` or use a `bin` directory already in your `$PATH`.

Then add the following symlinks in that `bin` directory.

```
cd /path/to/bin
ln -s /pscratch/sd/t/tpeterka/software/mpich-4.3.0/install/bin/mpif90 ftn
ln -s /pscratch/sd/t/tpeterka/software/mpich-4.3.0/install/bin/mpicc cc
ln -s /pscratch/sd/t/tpeterka/software/mpich-4.3.0/install/bin/mpicxx CC
```
Confirm that the symlinks work:
`which ftn`, `which cc`, `which CC`

-----

## Cloning this repository and setting up a Spack environment

### Clone this repository

```
git clone https://github.com/tpeterka/e3sm-workflow
```

### Add the following Spack repositories to your local Spack installation

No environment should be active. Run `spack env status` to be sure.

Add the LowFive repository to your Spack installation (not included with Spack by default).
```
git clone https://github.com/diatomic/LowFive
spack repo add LowFive
```

Add the Wilkins repository to your Spack installation (not included with Spack by default).
```
git clone https://github.com/orcunyildiz/wilkins
spack repo add wilkins
```

Add the Mpas-o-scorpio repository to your Spack installation (not included with Spack by default).
```
spack repo add /path/to/e3sm-workflow/mpas-o-scorpio
```

### Set up the Spack environment

First time: create and load the Spack environment

```
cd /path/to/e3sm-workflow
source ./create-env.sh             # requires being in the same directory to work properly
source ./load-env.sh
```

Subsequent times: just load the Spack environment

```
source /path/to/e3sm-workflow/load-env.sh
```

-----

## Cloning the E3SM repository and setting up an ocean test case

### Clone the E3SM repository

```
git clone https://github.com/E3SM-Project/E3SM
cd E3SM
git submodule update --init --recursive
```
On a new machine, if you are denied permission to execute the `git submodule update --init --recursive` command, you
need to copy your ssh public key to your github account:
```
cd ~/.ssh
ls
```
If a public key doesn't exist:
```
ssh-keygen -t ed25519 -C "<your email address>"
# press enter for all prompts
```
Copy the key to the clipboard, log into your account on github.com, edit your settings, and add the SSH key.

Also first time only for a new git configuration, you may want to do:
```
git config --global user.email "<your email address>"
git config --global user.name "<your name>"
```

-----

<!-- ## Generating an E3SM ocean test case -->

<!-- Edit the template in `run.ccase1.sh` according to the instructions [here](https://docs.e3sm.org/running-e3sm-guide/guide-prior-to-production/) -->
<!-- Set the `MACHINE`, `PROJECT`, `CASE_NAME`, `CASE_ROOT`, `CODE_ROOT`. -->
<!-- For the first time, set the `do_*` flags as follows: -->
<!-- ``` -->
<!-- do_fetch_code=true -->
<!-- do_create_newcase=true -->
<!-- do_case_setup=true -->
<!-- do_case_build=true -->
<!-- do_case_submit=false -->
<!-- ``` -->
<!-- Subsequent times, set various flags, eg. `do_fetch_code`, to `false`. -->

<!-- Run the script: -->
<!-- ``` -->
<!-- ./run.ccase1.sh -->
<!-- ``` -->

<!-- ----- -->

### Create the ocean test case (ccase)

The spack environment should have been loaded (`source /path/to/e3sm-workflow/load-env.sh`)

```
cd /path/to/E3SM/code/latest/cime/scripts
./create_newcase --case <case> --output-root "/path/to/E3SM/<case>" --handle-preexisting-dirs u --compset CMPASO-JRA1p4 --res TL319_IcoswISC30E3r5 --machine pm-cpu --compiler gnu
```

Note: (For Perlmutter) `pm-cpu` above is perlmutter-cpu. Other machines supported by E3SM are also available.

The case will be created in `/path/to/E3SM/<case>`. Subsequent instructions refer to this location.

### Patch the environment xml file

(For Perlmutter)

```
cd /path/to/E3SM/<case>
patch env_mach_specific.xml /path/to/e3sm-workflow/env_mach_specific.patch
```

### Set up the case

The spack environment should have been loaded (`source /path/to/e3sm-workflow/load-env.sh`)

```
cd /path/to/E3SM/<case>
./case.setup
```

-----

## Building E3SM to run in a workflow

The spack environment should have been loaded (`source /path/to/e3sm-workflow/load-env.sh`)

### The first time, patch the E3SM cmake files

```
cd /path/to/E3SM
git apply /path/to/e3sm-workflow/E3SM.patch
cd /path/to/E3SM/<case>
patch cmake_macros/universal.cmake /path/to/e3sm-workflow/universal.cmake.patch
```

### Build E3SM

The original standalone executable, `e3sm.exe`, is built along with the new
shared object, `e3sm_shared.so`, which is what the workflow will run.
The original executable is built so that it can be run standalone if desired.
Because two targets are built that use the same object files, the `case.build`
script, which uses `-j` to parallelize the build, fails. We start the build
using the script so that the build is configured, and after it fails, we build
sequentially using `make`.

```
cd /path/to/E3SM/<case>
./case.build --clean-all                    # optional, if rebuilding
./case.build                                # build in parallel until it fails
make -C ccase1/bld/cmake-bld clean          # then clean and make sequentially
make -C ccase1/bld/cmake-bld VERBOSE=1      # VERBOSE=1 is optional
mv <case>/bld/cmake-bld/cmake/cpl/e3sm_shared.so <case>/bld
```
The build logs, executable, and shared object are located in `/path/to/E3SM/<case>/<case>/bld`.

-----

## Testing the E3SM build

The spack environment should have been loaded (`source /path/to/e3sm-workflow/load-env.sh`)

### Test that `e3sm.exe` was built correctly by running it standalone

```
unset HDF5_VOL_CONNECTOR
unset HDF5_PLUGIN_PATH
cd /path/to/E3SM/<case>/<case>/run
mkdir timing/checkpoints      # first time only
(For Perlmutter)
salloc --nodes 1 --qos interactive --time 30:00 --constraint cpu --account=<your-account>
srun  --label  -n 128 -N 1 -c 2  --cpu_bind=cores  -m plane=128 /path/to/E3SM/<case>/<case>/bld/e3sm.exe 2>&1 | tee e3sm-run-log.txt
```

### Test that `e3sm_shared.so` was built correctly by running it using a driver utility

```
unset HDF5_VOL_CONNECTOR
unset HDF5_PLUGIN_PATH
cd /path/to/E3SM/<case>/<case>/run
mkdir timing/checkpoints      # first time only
(For Perlmutter)
salloc --nodes 1 --qos interactive --time 30:00 --constraint cpu --account=<your-account>
srun  --label  -n 128 -N 1 -c 2  --cpu_bind=cores  -m plane=128 $HENSON/bin/henson-exec -- /path/to/E3SM/<case>/<case>/bld/e3sm_shared.so 2>&1 | tee e3sm-run-log.txt
```

-----

## Configuring the workflow

Edit line 2 of `/path/to/e3sm-workflow/wilkins-config.yaml` to the `path/to/E3SM/<case>/<case>/bld/e3sm_shared.so` on your machine.

Edit line 10 of `/path/to/e3sm-workflow/wilkins-config.yaml` to the `path/to/e3sm-workflow/analysis.py` on your machine.

Edit line 12 of `/path/to/e3sm-workflow/wilkins-config.yaml`, the first argument to the `path/to/E3SM/<case>/<case>/run/<your_file.nc>` on your machine, and the second argument to the variable you wish to print.

Edit line 6 of `/path/to/e3sm-workflow/wilkins-run.sh` to the `path/to/e3sm-workflow/wilkins-config.yaml` on your machine.

To switch between file mode and MPI mode for data transfers:
Change the settings of `passthru` and `metadata` on lines 8, 9, and 17, 18 of
`path/to/e3sm-workflow/wilkins-config.yaml` as follows:

file transfer:
```
passthru: 1
metadata: 0
```

MPI message transfer:
```
passthru: 0
metadata: 1
```

Because of the way NetCDF works, even for MPI data transfers, there needs to be a valid netCDF file on disk of the same name being read by
the analysis code, otherwise the analysis code will fail. For the first execution, use file mode
so that a file is produced on disk, and then leave the file
there. Afterwards you may use MPI mode.  Alternatively, you may copy the
blank netcdf file `blank.nc` from the top level of the e3sm-workflow repository to the run directory
and rename `blank.nc` to name of the file given in the first argument to the analysis code. Then you can use MPI mode immediately.

-----

## Running the workflow

The spack environment should have been loaded (`source /path/to/e3sm-workflow/load-env.sh`)

```
cd /path/to/E3SM/<case>/<case>/run
mkdir timing/checkpoints      # first time only
(For Perlmutter)
salloc --nodes 1 --qos interactive --time 30:00 --constraint cpu --account=<your-account>
/path/to/e3sm-workflow/wilkins-run.sh
```
-----
