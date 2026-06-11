#include <diy/mpi/communicator.hpp>
#include <diy/master.hpp>
#include <thread>
#include "prod-con.hpp"

#include <netcdf.h>
#include <netcdf_par.h>

#include "fmt/format.h"

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
    int                     ndims;
    std::vector<size_t>     dims(MAX_DIMS);
    std::vector<int>        dimids(MAX_DIMS);
    int                     varid;
    int                     nvars;                  // number of variables
    int                     ngatts;                 // number of global attributes
    int                     unlimdimid;             // id of unlimited dim
    int                     err;

    // debug
    fmt::print(stderr, "consumer: local comm rank {} size {}\n", local_.rank(), local_.size());

    if (!getenv("HDF5_VOL_CONNECTOR"))
    {
        fmt::print(stderr, "Error: HDF5_VOL_CONNECTOR is not set\n");
        abort();
    } else
        fmt::print(stderr, "HDF5_VOL_CONNECTOR is set\n");

    // open file for reading
    err = nc_open_par("outfile.nc", NC_NOWRITE, local, MPI_INFO_NULL, &ncid); ERR

    // read the metadata

    // global metadata
    err = nc_inq(ncid, &ndims, &nvars, &ngatts, &unlimdimid); ERR
    fmt::print(stderr, "*** consumer metadata: ndims {} nvars {} ngatts {} unlimdimid {} ***\n",
            ndims, nvars, ngatts, unlimdimid);

    // dimensions
    char dimname[256];
    size_t dimlen;
    for (int d = 0; d < ndims; d++)
    {
        err = nc_inq_dim(ncid, d, dimname, &dimlen); ERR
        dims[d] = dimlen;
        fmt::print(stderr, "*** consumer dim {} dim_name {} dimlen {} ***\n", d, dimname, dimlen);
    }

    // variable info
    char varname[256];
    int                     natts;                  // number of variable attributes
    nc_type                 dtype;                  // netCDF data type of this variable
    err = nc_inq_var(ncid, 0, varname, &dtype, &ndims, &dimids[0], &natts); ERR
    fmt::print(stderr, "*** consumer varname {} dtype {} ndims {} natts {}\n", varname, dtype, ndims, natts);

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

    // read the metadata (get variable ID)
    nc_inq_varid(ncid, "v1", &varid);

    // set collective access
    err = nc_var_par_access(ncid, varid, NC_COLLECTIVE); ERR

    // read v1
    std::vector<int> v1(elements_per_pe);
    err = nc_get_vara_int(ncid, varid, &starts[0], &counts[0], &v1[0]); ERR

    // print v1
    for (auto i = 0; i < v1.size(); i++)
        fmt::print(stderr, "v1[{}] = {}\n", local_.rank() * elements_per_pe + i, v1[i]);

    // close file
    err = nc_close(ncid); ERR

    // debug
    fmt::print(stderr, "*** consumer after closing file ***\n");
}

