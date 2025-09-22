from netCDF4 import Dataset
# import sys

# print("analysis starting up, argv[0]", sys.argv[0], "argv[1]", sys.argv[1], "argv[2]", sys.argv[2])

filename = "/pscratch/sd/t/tpeterka/software/E3SMv3/ccase1/ccase1/run/ccase1.mpaso.rst.0001-01-06_00000.nc"
varname = "xtime"

# rootgrp = Dataset(sys.argv[1])
rootgrp = Dataset(filename)

# print("input file: ", sys.argv[1], " dataset ", sys.argv[2], ":")
# print(rootgrp.variables[sys.argv[2]][:])
print("input file: ", filename, " dataset ", varname, ":")
print(rootgrp.variables[varname][:])

rootgrp.close()
print("analysis completed successfully")
