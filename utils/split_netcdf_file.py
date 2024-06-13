import netCDF4 as nc

def split_netcdf_by_timestep(input_file):
    with nc.Dataset(input_file, 'r') as src:
        time_variable_name = 'time_counter'
        num_timesteps = src.dimensions[time_variable_name].size
        
        # Iterate over each time step
        for i in range(num_timesteps):
            output_file = f"{input_file.split('.nc')[0]}_timestep_{i}.nc"
            
            # Create a new NetCDF file for each time step
            with nc.Dataset(output_file, 'w') as dst:
                # Copy global attributes
                dst.setncatts(src.__dict__)
                
                # Copy dimensions
                for name, dimension in src.dimensions.items():
                    if name == time_variable_name:
                        dst.createDimension(name, 1)
                    else:
                        dst.createDimension(name, len(dimension) if not dimension.isunlimited() else None)
                
                # Copy variables
                for name, variable in src.variables.items():
                    x = dst.createVariable(name, variable.datatype, variable.dimensions)
                    dst[name].setncatts(src[name].__dict__)
                    
                    if time_variable_name in variable.dimensions:
                        time_dim_index = variable.dimensions.index(time_variable_name)
                        if time_dim_index == 0:
                            x[:] = src[name][i:i+1]
                        else:
                            raise NotImplementedError("Time dimension is not first, needs custom slicing.")
                    else:
                        x[:] = src[name][:]

if __name__ == "__main__":
    input_file = "../simulation/app/data/DoubleGyre3D/doublegyreW.nc"
    split_netcdf_by_timestep(input_file)
