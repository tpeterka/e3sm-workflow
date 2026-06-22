#include <diy/mpi/communicator.hpp>
#include <diy/master.hpp>

#include "prod-con.hpp"

#include <netcdf.h>
#include <netcdf_par.h>

#define ERR {if(err!=NC_NOERR)printf("Error at line=%d: %s\n", __LINE__, nc_strerror(err));}

// This example reproduces the way the E3SM mpas-ocean component writes the
// "ssh" (sea surface height) variable through the SCORPIO/PIO -> NetCDF-4 stack.
//
// In the MPAS Registry (components/mpas-ocean/src/Registry.xml) ssh is declared:
//   <var name="ssh" type="real" dimensions="nCells Time" units="m"
//        description="sea surface height"/>
//
// The Registry lists dimensions in Fortran (column-major) order "nCells Time".
// When written through PIO to a NetCDF-4 file (C, row-major), the order is
// reversed to (Time, nCells), with Time as the UNLIMITED record dimension and
// nCells as the fixed, contiguous dimension that is decomposed across MPI ranks.
//
// The resulting variable in the file is:
//   float ssh(Time, nCells) ; Time = UNLIMITED (currently 1), nCells = 1791
//   ssh:units = "m" ;
//   ssh:long_name = "sea surface height" ;

#define N_CELLS 1791    // matches nCells in the reference E3SM output file

int main(int argc, char** argv)
{
    diy::mpi::environment   env(argc, argv, MPI_THREAD_MULTIPLE);

    // for some reason, local has to be a duplicate of world, not world itself
    diy::mpi::communicator      world;
    communicator                local;
    MPI_Comm_dup(world, &local);
    diy::mpi::communicator local_(local);

    int                     ncid;
    int                     dimids[2];      // Time, nCells (C order)
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
    err = nc_create_par("ssh.nc", NC_NETCDF4 | NC_CLOBBER | NC_NODIMSCALE_ATTACH,
            local, MPI_INFO_NULL, &ncid); ERR

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
    counts[0]   = 1;                // write one Time record
    counts[1]   = my_cells;         // number of cells owned by this rank

    // generate ssh data (single precision, matching the file's float type)
    // synthetic per-cell values keyed on the global cell index for debugging
    std::vector<float> ssh(my_cells);
    for (int i = 0; i < my_cells; i++)
        ssh[i] = static_cast<float>(my_cell_start + i);

    // define dimensions (C order: Time is the slowest/first dimension and unlimited)
    err = nc_def_dim(ncid, "Time", NC_UNLIMITED, &dimids[0]); ERR
    err = nc_def_dim(ncid, "nCells", N_CELLS, &dimids[1]); ERR

    // define variable ssh as float with dims (Time, nCells)
    err = nc_def_var(ncid, "ssh", NC_FLOAT, 2, &dimids[0], &varid); ERR

    // attributes matching the E3SM output file
    err = nc_put_att_text(ncid, varid, "units", 1, "m"); ERR
    err = nc_put_att_text(ncid, varid, "long_name", 18, "sea surface height"); ERR

    // end define mode
    err = nc_enddef(ncid); ERR

    // set collective access
    err = nc_var_par_access(ncid, varid, NC_COLLECTIVE); ERR

    // write variable
    // netcdf automatically extends the unlimited Time dimension
    err = nc_put_vara_float(ncid, varid, &starts[0], &counts[0], &ssh[0]); ERR

    // close file
    err = nc_close(ncid); ERR

    // debug: print ssh
//     for (int i = 0; i < my_cells; i++)
//         fmt::print(stderr, "ssh[{}] = {}\n", my_cell_start + i, ssh[i]);

    // debug
    fmt::print(stderr, "*** producer after closing file ***\n");
}
