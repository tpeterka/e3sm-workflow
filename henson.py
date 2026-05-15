#!/usr/bin/env python3

from mpi4py import MPI
import pyhenson as h
import lowfive
import os
import importlib
import time

world = MPI.COMM_WORLD.Dup()
size = world.Get_size()

passthru = False
consumer_procs = 1

pm = h.ProcMap(world, [("producer", size - consumer_procs), ("consumer", consumer_procs)])
nm = h.NameMap()

if pm.group() == "producer":
    tag = 0
    lowfive.create_logger("trace")
    vol = lowfive.create_DistMetadataVOL(pm.local(), pm.intercomm("consumer", tag))
#     vol = lowfive.create_VOLBase()
    vol.set_passthru("*", "*")
    if passthru:
        vol.set_passthru("ccase2.mpaso.hist.am.highFrequencyOutput.0001-01-01_00.00.00.nc", "*")
    else:
        vol.set_memory("ccase2.mpaso.hist.am.highFrequencyOutput.0001-01-01_00.00.00.nc", "*")
        vol.set_intercomm("ccase2.mpaso.hist.am.highFrequencyOutput.0001-01-01_00.00.00.nc", "*", 0)

    prod = h.Puppet("/pscratch/sd/t/tpeterka/software/E3SM/ccase2/ccase2/bld/e3sm_shared.so", [], pm, nm)

    prod.proceed()

    if passthru:
        h.to_mpi4py(pm.intercomm("consumer", tag)).barrier()
else:
    tag = 0
    lowfive.create_logger("trace")
    vol = lowfive.create_DistMetadataVOL(pm.local(), pm.intercomm("producer", tag))
#     vol = lowfive.create_VOLBase()
    if passthru:
        vol.set_passthru("ccase2.mpaso.hist.am.highFrequencyOutput.0001-01-01_00.00.00.nc", "*")
    else:
        vol.set_memory("ccase2.mpaso.hist.am.highFrequencyOutput.0001-01-01_00.00.00.nc", "*")
        vol.set_intercomm("ccase2.mpaso.hist.am.highFrequencyOutput.0001-01-01_00.00.00.nc", "*", 0)

    cons = h.Puppet("/global/homes/t/tpeterka/software/e3sm-workflow/install/bin/consumer.so",
                    ["--infile", "ccase2.mpaso.hist.am.highFrequencyOutput.0001-01-01_00.00.00.nc",
                     "--dataset", "ssh"], pm, nm)

    if passthru:
        h.to_mpi4py(pm.intercomm("producer", tag)).barrier()
    cons.proceed()
