import netCDF4 as nc

path = "../simulation/data/DoubleGyre2D"
file = path + "/" + "doublegyreU.nc"

ds = nc.Dataset(file, "r")
print(ds)

for var_name, var in ds.variables.items():
    print(f" - {var_name}")
    print(f"   - Dimensions: {var.dimensions}")
    print(f"   - Data type: {var.dtype}")
    for attr_name in var.ncattrs():
        print(f"   - {attr_name}: {var.getncattr(attr_name)}")
    print()
