#include <diy/mpi/communicator.hpp>
#include <diy/master.hpp>

#include "prod-con.hpp"

#include <netcdf.h>
#include <netcdf_par.h>

#define MAX_DIMS 10

#define ERR {if(err!=NC_NOERR)printf("Error at line=%d: %s\n", __LINE__, nc_strerror(err));}

int main(int argc, char** argv)
{
    diy::mpi::environment   env(argc, argv, MPI_THREAD_MULTIPLE);

    // for some reason, local has to be a duplicate of world, not world itself
    diy::mpi::communicator      world;
    communicator                local;
    MPI_Comm_dup(world, &local);
    diy::mpi::communicator local_(local);

    int                     ncid;
    int                     elements_per_pe;
    std::vector<size_t>     dims(MAX_DIMS);
    std::vector<int>        dimids(MAX_DIMS);
    int                     varid;
    int                     err;

    // debug
    fmt::print(stderr, "producer: local comm rank {} size {}\n", local_.rank(), local_.size());

    if (!getenv("HDF5_VOL_CONNECTOR"))
    {
        fmt::print(stderr, "Error: HDF5_VOL_CONNECTOR is not set\n");
        abort();
    } else
        fmt::print(stderr, "HDF5_VOL_CONNECTOR is set\n");

    // create file
    err = nc_create_par("outfile.nc", NC_NETCDF4 | NC_CLOBBER | NC_NODIMSCALE_ATTACH,
            local, MPI_INFO_NULL, &ncid); ERR

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

    // generate dataset v1
    std::vector<int> v1(elements_per_pe);
    for (int i = 0; i < elements_per_pe; i++)
        v1[i] = local_.rank() * elements_per_pe + i;

    // define dimensions
    err = nc_def_dim(ncid, "dim0", NC_UNLIMITED, &dimids[0]); ERR
    err = nc_def_dim(ncid, "dim1", dims[1], &dimids[1]); ERR

    // define variable v1
    err = nc_def_var(ncid, "v1", NC_INT, 2, &dimids[0], &varid); ERR

    // end define mode
    err = nc_enddef(ncid); ERR

    // set collective access
    err = nc_var_par_access(ncid, varid, NC_COLLECTIVE); ERR

    // write variable
    // netcdf automatically extends unlimited dimension
    err = nc_put_vara_int(ncid, varid, &starts[0], &counts[0], &v1[0]); ERR

    // close file
    err = nc_close(ncid); ERR

    // debug: print v1
//     for (auto i = 0; i < v1.size(); i++)
//         fmt::print(stderr, "v1[{}] = {}\n", local_.rank() * elements_per_pe + i, v1[i]);

    // debug
    fmt::print(stderr, "*** producer after closing file ***\n");
}
