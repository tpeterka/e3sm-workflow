#include <diy/mpi/communicator.hpp>
#include <diy/master.hpp>
#include <thread>
#include <dlfcn.h>
#include "fmt/format.h"
#include "opts.h"
#include <netcdf.h>
#include <netcdf_par.h>

using communicator  = MPI_Comm;

#define ERR(e) { std::cout << "Error: " << nc_strerror(e) << std::endl; MPI_Abort(MPI_COMM_WORLD, 1); }

int main(int argc, char* argv[])
{
    diy::mpi::environment   env(argc, argv, MPI_THREAD_MULTIPLE);
    diy::mpi::communicator  world;
    communicator            mpi_world       = MPI_COMM_WORLD;
    int                     ncid;
    int                     varid;;
    int                     ndims;
    int                     retval;
    std::string             infile;
    bool                    help;

    // get command line arguments
    using namespace opts;
    Options ops;
    ops
        >> Option('f', "infile",    infile,         "input file name")
        >> Option('h', "help",      help,           "show help")
        ;

    if (!ops.parse(argc,argv) || help)
    {
        if (world.rank() == 0)
        {
            std::cout << "Usage: " << argv[0] << " [OPTIONS]\n";
            std::cout << "Reads various datasets.\n";
            std::cout << ops;
        }
        return 1;
    }

    // debug
    int rank, size;
    MPI_Comm_rank(mpi_world, &rank);
    MPI_Comm_size(mpi_world, &size);
    fmt::print(stderr, "*** consumer before opening file {}: local comm rank {} size {} ***\n", 
            infile.c_str(), rank, size);

    // open file for reading
    retval = nc_open_par(infile.c_str(), NC_NOWRITE, mpi_world, MPI_INFO_NULL, &ncid);
    if (retval != NC_NOERR) ERR(retval);

    // debug
    fmt::print(stderr, "*** consumer after opening file\n");

    // get the variable id for "xtime"
    retval = nc_inq_varid(ncid, "xtime", &varid);
    if (retval != NC_NOERR) ERR(retval);
    fmt::print(stderr, "*** consumer varid for xtime = {}\n", varid);

    // clean up
    retval = nc_close(ncid);
    if (retval != NC_NOERR) ERR(retval);

    // debug
    fmt::print(stderr, "*** consumer after closing file ***\n");
}

