from netCDF4 import Dataset
import sys

print("analysis starting up")

rootgrp = Dataset(sys.argv[1])

print("input file: ", sys.argv[1], " dataset ", sys.argv[2], ":")
print(rootgrp.variables[sys.argv[2]][:])

rootgrp.close()
print("analysis completed successfully")
