#!/usr/bin/env python3

from mpi4py import MPI
import pyhenson as h
import lowfive
import os
import importlib
import time

world = MPI.COMM_WORLD.Dup()
size = world.Get_size()

passthru = True
consumer_procs = 1

pm = h.ProcMap(world, [("producer", size - consumer_procs), ("consumer", consumer_procs)])
nm = h.NameMap()

if pm.group() == "producer":
    tag = 0
    lowfive.create_logger("trace")
    vol = lowfive.create_DistMetadataVOL(pm.local(), pm.intercomm("consumer", tag))
#     vol = lowfive.create_VOLBase()
    if passthru:
        vol.set_passthru("*", "*")
    else:
        vol.set_memory("*", "*")
    vol.set_intercomm("*", "*", 0)

    # set the following path to your installation of the producer task
    prod = h.Puppet("/pscratch/sd/t/tpeterka/software/E3SMv3/ccase1/ccase1/bld/e3sm_shared.so", [], pm, nm)

    prod.proceed()

    if passthru:
        h.to_mpi4py(pm.intercomm("consumer", tag)).barrier()
else:
    tag = 0
    lowfive.create_logger("trace")
    vol = lowfive.create_DistMetadataVOL(pm.local(), pm.intercomm("producer", tag))
#     vol = lowfive.create_VOLBase()
    if passthru:
        vol.set_passthru("*", "*")
    else:
        vol.set_memory("*", "*")
    vol.set_intercomm("*", "*", 0)

    if passthru:
        h.to_mpi4py(pm.intercomm("producer", tag)).barrier()

    # set the following path to your installation of the analysis task
    importlib.import_module("/global/homes/t/tpeterka/software/e3sm-workflow/analysis.py")
