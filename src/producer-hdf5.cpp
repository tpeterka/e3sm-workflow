#include <diy/mpi/communicator.hpp>
#include <diy/master.hpp>

#include "prod-con.hpp"

#include <hdf5.h>

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

    // debug
    fmt::print(stderr, "producer: local comm rank {} size {}\n", local_.rank(), local_.size());

    // set up file access property list
    hid_t plist = H5Pcreate(H5P_FILE_ACCESS);
    H5Pset_fapl_mpio(plist, local, MPI_INFO_NULL);

    if (!getenv("HDF5_VOL_CONNECTOR"))
    {
        fmt::print(stderr, "Error: HDF5_VOL_CONNECTOR is not set\n");
        abort();
    } else
        fmt::print(stderr, "HDF5_VOL_CONNECTOR is set\n");

    // create a new file and group using default properties
    hid_t file = H5Fcreate("outfile.h5", H5F_ACC_TRUNC, H5P_DEFAULT, plist);

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
    std::vector<size_t> starts(2), counts(2), max_dims(2), cur_dims(2), chunk_dims(2);
    dims[0]         = 1;
    dims[1]         = 128;
    elements_per_pe = dims[1] / local_.size();
    starts[0]       = 0;
    starts[1]       = local_.rank() * elements_per_pe;
    counts[0]       = 1;
    counts[1]       = elements_per_pe;
    max_dims[0]     = H5S_UNLIMITED;
    max_dims[1]     = 128;
    cur_dims[0]     = 0;
    cur_dims[1]     = 128;
    chunk_dims[0]   = 1;
    chunk_dims[1]   = 128;

    // generate dataset v1
    std::vector<int> v1(elements_per_pe);
    for (int i = 0; i < elements_per_pe; i++)
        v1[i] = local_.rank() * elements_per_pe + i;

    // --- fixed size of 1 in first dimension, does not grow ---

//     // filespace is local subset of global domain
//     hid_t filespace = H5Screate_simple(2, &dims[0], &max_dims[0]);
//     H5Sselect_hyperslab(filespace, H5S_SELECT_SET, &starts[0], NULL, &counts[0], NULL);
// 
//     // memspace is simple counts
//     hid_t memspace = H5Screate_simple(2, &counts[0], &max_dims[0]);
// 
//     // write dataset v1
//     hid_t dset = H5Dcreate2(file, "v1", H5T_NATIVE_INT, filespace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
//     H5Dwrite(dset, H5T_NATIVE_INT, memspace, filespace, H5P_DEFAULT, &v1[0]);
// 
//     // debug: get extent of filespace and memspace
//     std::vector<size_t> extent_dims(2), extent_max_dims(2);
//     H5Sget_simple_extent_dims(filespace, &extent_dims[0], &extent_max_dims[0]);
//     fmt::print(stderr, "filespace extent_dims [{},{}] extent_maxdims[{},{}]\n",
//             extent_dims[0], extent_dims[1], extent_max_dims[0], extent_max_dims[1]);
//     H5Sget_simple_extent_dims(memspace, &extent_dims[0], &extent_max_dims[0]);
//     fmt::print(stderr, "memspace extent_dims [{},{}] extent_maxdims[{},{}]\n",
//             extent_dims[0], extent_dims[1], extent_max_dims[0], extent_max_dims[1]);

    // --- grow dataset from 0 to 1 in the first dimension ----

    // filespace is local subset of global domain
    hid_t filespace = H5Screate_simple(2, &cur_dims[0], &max_dims[0]);

    // dataset creation property list (chunking is required for unlimited dimensions)
    hid_t property_list = H5Pcreate(H5P_DATASET_CREATE);
    H5Pset_chunk(property_list, 2, &chunk_dims[0]);

    // create empty dataset v1
    hid_t dset = H5Dcreate2(file, "v1", H5T_NATIVE_INT, filespace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

    // extend the dataset from 0 rows to 1 row
    cur_dims[0] = 1;
    H5Dset_extent(dset, &cur_dims[0]);

    // get the updated filespace from the extended dataset
    filespace = H5Dget_space(dset);

    // select a hyperslab from the filespace
    H5Sselect_hyperslab(filespace, H5S_SELECT_SET, &starts[0], NULL, &counts[0], NULL);

    // memspace is simple counts
    hid_t memspace = H5Screate_simple(2, &counts[0], &max_dims[0]);

    // write the data
    H5Dwrite(dset, H5T_NATIVE_INT, memspace, filespace, H5P_DEFAULT, &v1[0]);

    // debug: print v1
//     for (auto i = 0; i < v1.size(); i++)
//         fmt::print(stderr, "v1[{}] = {}\n", local_.rank() * elements_per_pe + i, v1[i]);

    // clean up
    H5Dclose(dset);
    H5Sclose(memspace);
    H5Sclose(filespace);
    H5Fclose(file);
    H5Pclose(plist);

    // debug
    fmt::print(stderr, "*** producer after closing file ***\n");
}
