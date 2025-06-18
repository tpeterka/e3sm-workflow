# Instructions for Building E3SM (ocean component) and running in a Workflow

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

### First time: create and load the Spack environment

```
cd /path_to/e3sm-workflow
source ./create-env.sh             # requires being in the same directory to work properly
source ./load-env.sh
```

### Subsequent times: load the Spack environment

```
source /path_to/e3sm-workflow/load-env.sh
```

-----

## Clone E3SM repository

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

<!-- Edit the template in `ccase1.sh` according to the instructions [here](https://docs.e3sm.org/running-e3sm-guide/guide-prior-to-production/) -->
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

## Creating ocean test case (ccase)

The spack environment should have been loaded (`source /path_to/e3sm-workflow/load-env.sh`)

```
/path/to/E3SM/code/latest/cime/scripts/create_newcase --case <case> --output-root "/path/to/E3SM/<case>" --handle-preexisting-dirs u --compset CMPASO-JRA1p4 --res TL319_IcoswISC30E3r5 --machine pm-cpu --compiler gnu

```

Note: `pm-cpu` above is perlmutter-cpu. Other machines supported by E3SM are also available.

-----

# (For Perlmutter) Modify environment for building E3SM

Patch the environment xml file:

```
cd /path/to/E3SM/<case>
patch env_mach_specific.xml /path/to/e3sm-workflow/env_mach_specific.patch 
```

-----

## Set up the case

The spack environment should have been loaded (`source /path_to/e3sm-workflow/load-env.sh`)

```
cd /path/to/E3SM/<case>
./case.setup
```

-----

## (For Perlmutter) Add symlinks for compiler wrappers pointing to my mpich installation

Add or use a `bin` directory to or in your `$PATH`
Then add symlinks:

```
cd /path/to/bin
ln -s /pscratch/sd/t/tpeterka/software/mpich-4.3.0/install/bin/mpif90 ftn
ln -s /pscratch/sd/t/tpeterka/software/mpich-4.3.0/install/bin/mpicc cc
ln -s /pscratch/sd/t/tpeterka/software/mpich-4.3.0/install/bin/mpicxx CC
```
Confirm that the symlinks work:
`which ftn`, `which cc`, `which CC`

-----

## Build E3SM

The spack environment should have been loaded (`source /path_to/e3sm-workflow/load-env.sh`)

The first time, patch the E3SM makefiles:

```
cd /path/to/E3SM
git apply /path/to/e3sm-workflow/E3SM.patch
```

Then proceed to build E3SM:

```
cd /path/to/E3SM/<case>
./case_build --clean-all   # optional, if rebuilding
./case_build
```
The build logs and executable are located in `/path/to/E3SM/<case>/<case>/bld`.

-----

# Run E3SM standalone (without a workflow) as a test

salloc --nodes 1 --qos interactive --time 30:00 --constraint cpu --account=<your-account>

srun  --label  -n 128 -N 1 -c 2  --cpu_bind=cores   -m plane=128 /path/to/E3SM/<case>/<case>/bld/e3sm.exe 2>&1 | tee e3sm-run-log.txt 

-----
