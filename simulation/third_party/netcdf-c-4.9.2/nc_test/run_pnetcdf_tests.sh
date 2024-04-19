#!/bin/sh

# This script runs some PnetCDF I/O tests

set -e

echo
echo "Testing file created with PnetCDF is modifiable with netCDF..."
./tst_pnetcdf

echo "Testing file created with PnetCDF works when adding variables..."
./tst_addvar tst_pnetcdf.nc

# We assume a min of at least 2 processors is available
mpiexec -n 2 ./tst_parallel2

# These tests work in either serial or parallel builds.
mpiexec -n 1 ./t_nc
mpiexec -n 1 ./tst_atts3
mpiexec -n 1 ./tst_nofill
mpiexec -n 1 ./nc_test
mpiexec -n 1 ./tst_default_format_pnetcdf
mpiexec -n 1 ./tst_small
mpiexec -n 1 ./tst_formatx_pnetcdf
mpiexec -n 1 ./tst_cdf5format


