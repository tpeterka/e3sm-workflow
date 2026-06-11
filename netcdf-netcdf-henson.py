#!/usr/bin/env python3

from mpi4py import MPI
import pyhenson as h
import lowfive
from pathlib import Path
import os

world = MPI.COMM_WORLD.Dup()
size = world.Get_size()

passthru = False
consumer_procs = int(size / 2)

pm = h.ProcMap(world, [("producer", size - consumer_procs), ("consumer", consumer_procs)])
nm = h.NameMap()

if pm.group() == "producer":
    tag = 0
#     lowfive.create_logger("trace")
    vol = lowfive.create_DistMetadataVOL(pm.local(), pm.intercomm("consumer", tag))
    if passthru:
        vol.set_passthru("*", "*")
    else:
        vol.set_memory("*", "*")
#     vol.set_intercomm("*", "*", 0)

    # set the following path to point to your installation of producer
    prod = h.Puppet("/global/homes/t/tpeterka/software/e3sm-workflow/build/src/producer-netcdf.so", [], pm, nm)

    prod.proceed()

    if passthru:
        h.to_mpi4py(pm.intercomm("consumer", tag)).barrier()
else:
    tag = 0
#     lowfive.create_logger("trace")
    vol = lowfive.create_DistMetadataVOL(pm.local(), pm.intercomm("producer", tag))
    if passthru:
        vol.set_passthru("*", "*")
    else:
        vol.set_memory("*", "*")
    vol.set_intercomm("*", "*", 0)

    # set the following path to point to your installation of consumer
    cons = h.Puppet("/global/homes/t/tpeterka/software/e3sm-workflow/build/src/consumer-netcdf.so", [], pm, nm)

    if passthru:
        h.to_mpi4py(pm.intercomm("producer", tag)).barrier()

    cons.proceed()

