#include <diy/mpi/communicator.hpp>
#include <diy/master.hpp>
#include <thread>
#include "fmt/format.h"
#include "opts.h"
#include <netcdf.h>
#include <netcdf_par.h>

using communicator  = MPI_Comm;

#define ERR(e) { std::cout << "Error: " << nc_strerror(e) << std::endl; MPI_Abort(MPI_COMM_WORLD, 1); }

int main(int argc, char* argv[])
{
    diy::mpi::environment   env(argc, argv, MPI_THREAD_MULTIPLE);

    // for some reason, local has to be a duplicate of world, not world itself
    diy::mpi::communicator  world;
    communicator            local;
    MPI_Comm_dup(world, &local);
    diy::mpi::communicator local_(local);

    int                     ncid;
    int                     varid;;
    int                     ndims;
    int                     dimids[NC_MAX_VAR_DIMS];
    size_t                  dim_lens[NC_MAX_VAR_DIMS];
    size_t                  total_size = 1;
    int                     retval;
    std::string             infile;
    std::string             dataset;
    bool                    help;

    // get command line arguments
    using namespace opts;
    Options ops;
    ops
        >> Option('f', "infile",    infile,         "input file name")
        >> Option('d', "dataset",   dataset,        "dataset name")
        >> Option('h', "help",      help,           "show help")
        ;

    if (!ops.parse(argc,argv) || help)
    {
        if (local_.rank() == 0)
        {
            std::cout << "Usage: " << argv[0] << " [OPTIONS]\n";
            std::cout << "Reads various datasets.\n";
            std::cout << ops;
        }
        return 1;
    }

    // debug
    int rank, size;
    MPI_Comm_rank(local, &rank);
    MPI_Comm_size(local, &size);
    fmt::print(stderr, "*** consumer before opening file {}: local comm rank {} size {} ***\n", 
            infile.c_str(), rank, size);

    // open file for reading
    retval = nc_open_par(infile.c_str(), NC_NOWRITE, local, MPI_INFO_NULL, &ncid);
    if (retval != NC_NOERR) ERR(retval);

    // debug
    fmt::print(stderr, "*** consumer after opening file\n");

    // get the variable id for "ssh"
    retval = nc_inq_varid(ncid, dataset.c_str(), &varid);
    if (retval != NC_NOERR) ERR(retval);
    fmt::print(stderr, "*** consumer varid for {} = {}\n", dataset, varid);

    // set collective access
    retval = nc_var_par_access(ncid, varid, NC_COLLECTIVE);

    // query dimensionality (rank) and the ids of those dimensions
    retval = nc_inq_varndims(ncid, varid, &ndims);
    if (retval != NC_NOERR) ERR(retval);
    retval = nc_inq_vardimid(ncid, varid, dimids);
    if (retval != NC_NOERR) ERR(retval);

    // debug
    fmt::print(stderr, "ndims {}\n", ndims);

    // query size of each dimension and compute total size
    for (int i = 0; i < ndims; i++)
    {
        retval = nc_inq_dimlen(ncid, dimids[i], &dim_lens[i]);
        if (retval != NC_NOERR) ERR(retval);
        // debug
        fmt::print(stderr, "dimids[{}] {} dim_lens[{}] {}\n", i, dimids[i], i, dim_lens[i]);
        total_size *= dim_lens[i];
    }

    // debug
    fmt::print(stderr, "total_size {}\n", total_size);

    // read dataset
    std::vector<float> v(total_size);
    retval = nc_get_var_float(ncid, varid, &v[0]);
    if (retval != NC_NOERR) ERR(retval);

    // print first few values of dataset
    for (auto i = 0; i < std::min(v.size(), size_t(10)); i++)
        fmt::print(stderr, "v[{}] = {}\n", i, v[i]);

    // clean up
    retval = nc_close(ncid);
    if (retval != NC_NOERR) ERR(retval);

    // debug
    fmt::print(stderr, "*** consumer after closing file ***\n");
}

