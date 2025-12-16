from netCDF4 import Dataset
import sys
import argparse

def main(raw_args=None):

    print("analysis starting up")

    parser = argparse.ArgumentParser()
    parser.add_argument('--filename', dest='filename')
    parser.add_argument('--dataset', dest='dataset')
    config = parser.parse_args(raw_args)

    filename = config.filename
    varname = config.dataset

    print("filename = ", filename)
    print("varname = ", varname)

    rootgrp = Dataset(filename)

    print("input file: ", filename, " dataset ", varname, ":")
    print(rootgrp.variables[varname][:])

    rootgrp.close()
    print("analysis completed successfully")

if __name__ == "__main__":
    main()
