#include <diy/mpi/communicator.hpp>
#include <diy/master.hpp>
#include <thread>
#include "prod-con.hpp"

#include <netcdf.h>
#include <netcdf_par.h>

#include "fmt/format.h"

#define MAX_DIMS 10

#define ERR {if(err!=NC_NOERR)printf("Error at line=%d: %s\n", __LINE__, nc_strerror(err));}

// This example reads the "ssh" variable written by producer-ssh-netcdf.cpp,
// reproducing how the E3SM mpas-ocean output is structured:
//   float ssh(Time, nCells) ; Time = UNLIMITED (1), nCells = 1791
// The Time (unlimited) dimension is the slowest/first C dimension, and the
// decomposition is over the fast nCells dimension across MPI ranks.

#define N_CELLS 1791    // matches nCells in producer-ssh-netcdf.cpp

int main(int argc, char** argv)
{
    diy::mpi::environment   env(argc, argv, MPI_THREAD_MULTIPLE);

    // for some reason, local has to be a duplicate of world, not world itself
    diy::mpi::communicator      world;
    communicator                local;
    MPI_Comm_dup(world, &local);
    diy::mpi::communicator local_(local);

    int                     ncid;
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
    err = nc_open_par("ssh.nc", NC_NOWRITE, local, MPI_INFO_NULL, &ncid); ERR

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
    // ssh is 2d in the file: (Time, nCells), where Time is unlimited (using 1 record)
    // and nCells is fixed at N_CELLS. The decomposition is over nCells: each rank
    // owns a contiguous range of cells. nCells is not necessarily evenly divisible
    // by the number of ranks, so the last rank picks up the remainder.
    int     base_cells      = N_CELLS / local_.size();
    int     remainder       = N_CELLS % local_.size();
    int     my_cells        = base_cells + (local_.rank() == local_.size() - 1 ? remainder : 0);
    int     my_cell_start   = local_.rank() * base_cells;

    std::vector<size_t> starts(2), counts(2);
    starts[0]   = 0;                // Time record index
    starts[1]   = my_cell_start;    // first cell owned by this rank
    counts[0]   = 1;                // read one Time record
    counts[1]   = my_cells;         // number of cells owned by this rank

    // read the metadata (get variable ID)
    err = nc_inq_varid(ncid, "ssh", &varid); ERR

    // set collective access
    err = nc_var_par_access(ncid, varid, NC_COLLECTIVE); ERR

    // read ssh (single precision, matching the file's float type)
    std::vector<float> ssh(my_cells);
    err = nc_get_vara_float(ncid, varid, &starts[0], &counts[0], &ssh[0]); ERR

    // print ssh
//     for (auto i = 0; i < ssh.size(); i++)
//         fmt::print(stderr, "ssh[{}] = {}\n", my_cell_start + i, ssh[i]);

    // close file
    err = nc_close(ncid); ERR

    // debug
    fmt::print(stderr, "*** consumer after closing file ***\n");
}
