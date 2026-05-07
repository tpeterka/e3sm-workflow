from netCDF4 import Dataset
import sys
import argparse

def main(raw_args=None):

    print("analysis starting up")

    parser = argparse.ArgumentParser()
    parser.add_argument('--infile', dest='infile')
    parser.add_argument('--dataset', dest='dataset')
    config = parser.parse_args(raw_args)

    infile = config.infile
    dataset = config.dataset

    print("infile = ", infile)
    print("dataset = ", dataset)

    rootgrp = Dataset(infile)

    print(rootgrp.variables[dataset][:])

    rootgrp.close()
    print("analysis completed successfully")

if __name__ == "__main__":
    main()
