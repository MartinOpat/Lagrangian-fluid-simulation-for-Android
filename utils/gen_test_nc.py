import netCDF4 as nc
import numpy as np

# Create a new .nc file
ncfile = nc.Dataset('test.nc', mode='w', format='NETCDF4')

# Create dimensions
ncfile.createDimension('dim', 10)

# Create variables
x = ncfile.createVariable('x', np.float32, ('dim',))
y = ncfile.createVariable('y', np.float32, ('dim',))

# Generate random data for variables
x[:] = np.random.random(10)
y[:] = np.random.random(10)

# Close the file
ncfile.close()

print("NetCDF file 'test.nc' created successfully.")
