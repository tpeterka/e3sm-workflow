# Instructions for Building E3SM (ocean component) and Running in a Workflow

This repository provides an example of building and running E3SM coupled with a Python analysis task in an HPC in situ workflow.
The E3SM code is an ocean test case, and the Python analysis code simply prints one of the ocean output variables.
The data transfer can be configured to either read/write files or exchange MPI messages through the system interconnect.
The workflow is intended as a template to copy and modify to suit specific cases.

These instructions have been tested on the Perlmutter CPU partition at NERSC.
Other machines will be similar, but not tested.
Instructions that are specific to Perlmutter have been noted.

The recommended compiler is gcc version >= 12. All of the following instructions are for gnu.

The instructions in this README are divided into the following main steps:

- Preliminaries
- Cloning this repository and setting up a spack environment
- Setting up E3SM
- Setting up an ocean test case
- Building E3SM to run in a workflow
- Testing the E3SM build
- Configuring the workflow
- Running the workflow

-----

## Preliminaries

Preliminary steps include setting up your shell environment, loading/unloading
modules, and configuring your spack installation.

### Clone and install Spack if you don't already have it in your own home, project, or scratch directory

All software installation is done through Spack.
We will use the environments feature of Spack (analogous to Conda environments) to manage all the software dependencies.

First, install your own instance of Spack yourself, rather than relying on a system-installed version of Spack, so that you have complete control over all software versions.
If you don't have your own Spack installation, follow the instructions [here](https://spack.readthedocs.io/en/latest/) first.

### (For Perlmutter) Edit `~/.spack/packages.yaml` to use a newer MPI (mpich) than Cray's

This will point Spack to use an external mpich that I previously installed on Perlmutter. You should not install your own mpich. Instead, tell Spack to use mine as follows.

```
packages:
  mpich:
    externals:
    - spec: mpich@4
      prefix: /pscratch/sd/t/tpeterka/software/mpich-4.3.0/install
      extra_attributes:
        environment:
          prepend_path:
            LD_LIBRARY_PATH: /pscratch/sd/t/tpeterka/software/mpich-4.3.0/install/lib:/opt/cray/libfabric/1.22.0/lib64
    buildable: false
```

### (For Perlmutter) Unload Cray programming environment and add my mpich to your path

```
module unload PrgEnv-gnu/8.5.0
export PATH=$HOME/bin:$PSCRATCH/software/mpich-4.3.0/install/bin:$PATH
export LD_LIBRARY_PATH=$PSCRATCH/software/mpich-4.3.0/install/lib:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=/opt/cray/libfabric/1.22.0/lib64:$LD_LIBRARY_PATH
```

-----

## Cloning this repository and setting up a Spack environment for e3sm-workflow

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

Add the esm_watermasses repository to your Spack installation
```
spack repo add /path/to/e3sm-workflow/esm_watermasses
```

### Set up the Spack environment for e3sm-workflow

First time: create and load the Spack environment

```
cd /path/to/e3sm-workflow
source ./create-env.sh             # requires being in the same directory to work properly
source ./load-env.sh
```

Subsequent times: just load the Spack environment. `source /path/to/e3sm-workflow/load-env.sh`

-----

## Setting up E3SM

### Clone the E3SM repository

No Spack environment should be active. Check with `spack env status` and `spack env deactivate` if necessary.

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

### Patch the E3SM machine configuration and cmake

```
cd /path/to/E3SM
patch cime_config/machines/config_machines.xml /path/to/e3sm-workflow/conf_machines.patch
git apply /path/to/e3sm-workflow/E3SM.patch
```

-----

## Setting up an ocean test case (C case)

### Create a new case

The spack environment should have been loaded (`source /path/to/e3sm-workflow/load-env.sh`)

Below, replace `<case>` with the name of your case, eg., `ccase1`.

On Perlmutter, a larger C case with 128 MPI processses:
```
cd /path/to/E3SM
cime/scripts/create_newcase --case <case> --output-root /path/to/E3SM/<case> --handle-preexisting-dirs u --compset CMPASO-JRA1p4 --res TL319_IcoswISC30E3r5 --machine pm-cpu-generic --compiler gnu
```

On Perlmutter, a smaller C case with 1, 2, or 4 MPI processses:
```
cime/scripts/create_newcase --case <case> --output-root /path/to/E3SM/<case> --handle-preexisting-dirs u --res T62_oQU480 --compset CMPASO-NYF --machine pm-cpu-generic --compiler gnu
```

Note: `pm-cpu-generic` above is perlmutter-cpu with minimal other settings.

On smaller workstations (eg., ANL's GCE machines), a smaller C case with 1, 2, or 4  MPI processes:

```
cd /path/to/E3SM
cime/scripts/create_newcase --case <case> --output-root /path/to/E3SM/<case> --handle-preexisting-dirs u --res T62_oQU480 --compset CMPASO-NYF --machine anlgce-ub22 --compiler gnu
```

Note: `anlgce-ub22` above is ANL GCE with Ubuntu22.

The case will be created in `/path/to/E3SM/<case>`. Subsequent instructions refer to this location.

### Set up the case

The spack environment should have been loaded (`source /path/to/e3sm-workflow/load-env.sh`)

```
cd /path/to/E3SM/<case>

./xmlchange NTASKS=<num_procs>          # num_procs = 128 for the larger case, 1, 2, 4 for the smaller case
./xmlchange PIO_NUMTASKS=<num_procs>
./xmlchange PIO_STRIDE=1
./xmlchange PIO_TYPENAME=netcdf4p

./case.setup --reset                    # --reset is optional, if case was setup before

patch cmake_macros/universal.cmake /path/to/e3sm-workflow/universal.cmake.patch

# for ANL GCE
patch env_mach_specific.xml /path/to/e3sm-workflow/anlgce-ub22_env_mach_specific.patch

./preview_run
```

-----

## Building E3SM to run in a workflow

The spack environment should have been loaded (`source /path/to/e3sm-workflow/load-env.sh`)

### Build E3SM

The original standalone executable, `e3sm.exe`, is built along with the new
shared object, `e3sm_shared.so`, which is what the workflow will run.
The original executable is built so that it can be run standalone if desired.

```
cd /path/to/E3SM/<case>
./case.build --clean-all                    # optional, if rebuilding
./case.build
mv <case>/bld/cmake-bld/cmake/cpl/e3sm_shared.so <case>/bld
```
The build logs, executable, and shared object are located in `/path/to/E3SM/<case>/<case>/bld`.

In rare circumstances, if you need to reset the case, you would need to re-patch
the files as follows:

```
cd /path/to/E3SM/<case>
./case.setup --reset
patch cmake_macros/universal.cmake /path/to/e3sm-workflow/universal.cmake.patch
```

Then proceed with the rest of the build steps above. However, it should
rarely be necessary to reset the case, usually only for debugging and
development.

-----

## Editing `streams.ocean`

Edit `/path/to/E3SM/<case>/<case>/run/streams.ocean` to change any instances of `clobber_mode="append"` to
`clobber_mode="truncate"`.

-----

## Testing the E3SM build

The spack environment should have been loaded (`source /path/to/e3sm-workflow/load-env.sh`)

### Test that `e3sm.exe` was built correctly by running it standalone

```
unset HDF5_VOL_CONNECTOR
unset HDF5_PLUGIN_PATH
cd /path/to/E3SM/<case>/<case>/run
mkdir timing                  # first time only
mkdir timing/checkpoints      # first time only
(For Perlmutter)
salloc --nodes 1 --qos interactive --time 30:00 --constraint cpu --account=<your-account>
srun -l -n <num_procs> -N 1 -c 2  --cpu_bind=cores  -m plane=128 /path/to/E3SM/<case>/<case>/bld/e3sm.exe 2>&1 | tee e3sm-run-log.txt
```

### Test that `e3sm_shared.so` was built correctly by running it using a driver utility

```
unset HDF5_VOL_CONNECTOR
unset HDF5_PLUGIN_PATH
cd /path/to/E3SM/<case>/<case>/run
mkdir timing/checkpoints      # first time only
(For Perlmutter)
salloc --nodes 1 --qos interactive --time 30:00 --constraint cpu --account=<your-account>
srun -l -n <num_procs> -N 1 -c 2  --cpu_bind=cores  -m plane=128 $HENSON/bin/henson-exec -- /path/to/E3SM/<case>/<case>/bld/e3sm_shared.so 2>&1 | tee e3sm-run-log.txt
```

### Test that `e3sm_shared.so` can run with LowFive and Wilkins

Edit line 2 of `/path/to/e3sm-workflow/wilkins-config-prod-only.yaml` to the `path/to/E3SM/<case>/<case>/bld/e3sm_shared.so` on your machine.

Edit line 4 of `/path/to/e3sm-workflow/wilkins-config-prod-only.yaml` to the number of MPI processes for the case being run

Edit lines 10, 19 of `/path/to/e3sm-workflow/wilkins-run-prod-only.sh` to the `path/to/e3sm-workflow/wilkins-config-prod-only.yaml` on your machine.

Comment/uncomment the appropriate block of `/path/to/e3sm-workflow/wilkins-run-prod-only.sh` depending on the number of MPI processes being run


```
cd /path/to/E3SM/<case>/<case>/run
mkdir timing/checkpoints      # first time only
(For Perlmutter)
salloc --nodes 1 --qos interactive --time 30:00 --constraint cpu --account=<your-account>
/path/to/e3sm-workflow/wilkins-run-prod-only.sh
```
-----

## Configuring the workflow

Edit line 2 of `/path/to/e3sm-workflow/wilkins-config.yaml` to the `path/to/E3SM/<case>/<case>/bld/e3sm_shared.so` on your machine.

Edit line 4 of `/path/to/e3sm-workflow/wilkins-config.yaml` to the number of MPI processes for the case being run

Edit line 10 of `/path/to/e3sm-workflow/wilkins-config.yaml` to the `path/to/e3sm-workflow/analysis.py` on your machine.

Edit line 12 of `/path/to/e3sm-workflow/wilkins-config.yaml`, the first argument to the `path/to/E3SM/<case>/<case>/run/<your_file.nc>` on your machine, and the second argument to the variable you wish to print.

Edit lines 10, 19 of `/path/to/e3sm-workflow/wilkins-run.sh` to the `path/to/e3sm-workflow/wilkins-config.yaml` on your machine.

Comment/uncomment the appropriate block of `/path/to/e3sm-workflow/wilkins-run.sh` depending on the number of MPI processes being run

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

## Running the E3SM->consumer workflow

The spack environment should have been loaded (`source /path/to/e3sm-workflow/load-env.sh`)

```
cd /path/to/E3SM/<case>/<case>/run
mkdir timing/checkpoints      # first time only
(For Perlmutter, num_nodes = 2 for larger case, 1 for smaller case)
salloc --nodes <num_nodes> --qos interactive --time 30:00 --constraint cpu --account=<your-account>
/path/to/e3sm-workflow/wilkins-run.sh
```
-----

## Running the esm_watermasses only workflow

For Perlmutter, use a special load script that does not activate the spack environment because of a lua conflict with slurm:
```
source /path/to/e3sm-workflow/load-env-watermasses.sh
```

Edit the spack-installed `watermasses.py` to add command line arguments to `main()`:
- Change line 15 from `def main():` to `def main(raw_args=None):`
- Change line 38 from `args = parser.parse_args()` to `args = parser.parse_args(raw_args)`

Run the workflow:
```
cd /global/cfs/cdirs/m4259/esm_watermasses
salloc --nodes <num_nodes> --qos interactive --time 30:00 --constraint cpu --account=<your-account>
/path/to/e3sm-workflow/wilkins-run-watermasses-only.sh
```


