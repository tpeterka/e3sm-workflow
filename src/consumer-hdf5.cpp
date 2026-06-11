#include <diy/mpi/communicator.hpp>
#include <diy/master.hpp>
#include <thread>
#include "prod-con.hpp"

#include <hdf5.h>

#include "fmt/format.h"

#define MAX_DIMS 10

herr_t fail_on_hdf5_error(hid_t stack_id, void*)
{
    H5Eprint(stack_id, stderr);
    fprintf(stderr, "An HDF5 error was detected. Terminating.\n");
    exit(1);
}

int main(int argc, char** argv)
{
    diy::mpi::environment   env(argc, argv, MPI_THREAD_MULTIPLE);

    // for some reason, local has to be a duplicate of world, not world itself
    diy::mpi::communicator      world;
    communicator                local;
    MPI_Comm_dup(world, &local);
    diy::mpi::communicator local_(local);

    int                     elements_per_pe;
    std::vector<size_t>     dims(MAX_DIMS);
    herr_t                  status;

    // debug
    fmt::print(stderr, "consumer: local comm rank {} size {}\n", local_.rank(), local_.size());

    // set up file access property list
    hid_t plist = H5Pcreate(H5P_FILE_ACCESS);
    H5Pset_fapl_mpio(plist, local, MPI_INFO_NULL);

    if (!getenv("HDF5_VOL_CONNECTOR"))
    {
        fmt::print(stderr, "Error: HDF5_VOL_CONNECTOR is not set\n");
        abort();
    } else
        fmt::print(stderr, "HDF5_VOL_CONNECTOR is set\n");

    // open file for reading
    hid_t file = H5Fopen("outfile.h5", H5F_ACC_RDONLY, plist);

//     // decomposition
//     // the following dataset, dataspace, and decomposition  are hardcoded for 2d,
//     // where the first dimension is 1 and the second dimension is 128
//     std::vector<size_t> starts(2), counts(2);
//     dims[0]         = 1;
//     dims[1]         = 128;
//     elements_per_pe = dims[1] / local_.size();
//     starts[0]       = 0;
//     starts[1]       = local_.rank() * elements_per_pe;
//     counts[0]       = 1;
//     counts[1]       = elements_per_pe;
//     max_dims[0]     = 1;
//     max_dims[1]     = 128;

    // decomposition
    // the following dataset, dataspace, and decomposition  are hardcoded for 2d,
    // where the first dimension is unlimited but using 1 and the second dimension is 128
    std::vector<size_t> starts(2), counts(2), max_dims(2);
    dims[0]         = 1;
    dims[1]         = 128;
    elements_per_pe = dims[1] / local_.size();
    starts[0]       = 0;
    starts[1]       = local_.rank() * elements_per_pe;
    counts[0]       = 1;
    counts[1]       = elements_per_pe;
    max_dims[0]     = H5S_UNLIMITED;
    max_dims[1]     = 128;

    // filespace is local subset of global domain
    hid_t dset = H5Dopen2(file, "/v1", H5P_DEFAULT);
    hid_t filespace = H5Dget_space(dset);
    H5Sselect_hyperslab(filespace, H5S_SELECT_SET, &starts[0], NULL, &counts[0], NULL);

    // memspace is simple counts
    hid_t memspace = H5Screate_simple (2, &counts[0], &max_dims[0]);

    // read dataset v1
    std::vector<int> v1(elements_per_pe);
    status = H5Dread(dset, H5T_NATIVE_INT, memspace, filespace, H5P_DEFAULT, &v1[0]);
    if (status < 0)
        fmt::print(stderr, "Error reading dataset: status {}\n", status);

    // print v1
    for (auto i = 0; i < v1.size(); i++)
        fmt::print(stderr, "v1[{}] = {}\n", local_.rank() * elements_per_pe + i, v1[i]);

    // clean up
    H5Sclose(memspace);
    H5Sclose(filespace);
    H5Dclose(dset);
    H5Fclose(file);
    H5Pclose(plist);

    // debug
    fmt::print(stderr, "*** consumer after closing file ***\n");
}

