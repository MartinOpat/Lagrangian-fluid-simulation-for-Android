#include <stdio.h>
#include <netcdf.h>
#include <android/log.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>


void print_nc_vars(const char *filepath) {
    int ncid, retval;
    int nvars, varid;

    // Open the NetCDF file for reading.
    if ((retval = nc_open(filepath, NC_NOWRITE, &ncid))) {
        fprintf(stderr, "Error: Failed to open file %s\n", filepath);
        __android_log_print(ANDROID_LOG_ERROR, "native-lib", "Error: Failed to open file %s\n", filepath);
        return;
    }

    // Get the number of variables in the file.
    if ((retval = nc_inq_nvars(ncid, &nvars))) {
        fprintf(stderr, "Error: Unable to get number of variables\n");
        __android_log_print(ANDROID_LOG_ERROR, "native-lib", "Error: Unable to get number of variables\n");
        nc_close(ncid);
        return;
    }

    printf("Variables in the file %s:\n", filepath);
    __android_log_print(ANDROID_LOG_INFO, "native-lib", "Variables in the file %s:\n", filepath);

    // Iterate over all variables to get their names and print them.
    char var_name[NC_MAX_NAME + 1]; // +1 for null terminator
    for (varid = 0; varid < nvars; varid++) {
        if ((retval = nc_inq_varname(ncid, varid, var_name))) {
            fprintf(stderr, "Error: Unable to get the name of variable %d\n", varid);
            __android_log_print(ANDROID_LOG_ERROR, "native-lib", "Error: Unable to get the name of variable %d\n", varid);
            continue;
        }
        printf("Variable %d: %s\n", varid, var_name);
        __android_log_print(ANDROID_LOG_INFO, "native-lib", "Variable %d: %s\n", varid, var_name);
    }

    // Close the NetCDF file.
    if ((retval = nc_close(ncid))) {
        fprintf(stderr, "Error: Failed to close file %s\n", filepath);
        __android_log_print(ANDROID_LOG_ERROR, "native-lib", "Error: Failed to close file %s\n", filepath);
        return;
    }
}


void print_nc_vars_from_asset(AAssetManager *assetManager, const char *filename) {
    AAsset* asset = AAssetManager_open(assetManager, filename, AASSET_MODE_STREAMING);
    if (!asset) {
        __android_log_print(ANDROID_LOG_ERROR, "native-lib", "Failed to open asset %s", filename);
        return;
    }

    // Create a temporary file to copy the asset into
    char tmpFilename[] = "/data/data/lagrangianfluidsimulation/tempfile.nc"; // Change your.package.name to your actual package name
    FILE* out = fopen(tmpFilename, "wb");
    if (!out) {
        __android_log_print(ANDROID_LOG_ERROR, "native-lib", "Failed to open temporary file for writing");
        AAsset_close(asset);
        return;
    }

    // Read data from the asset and write it to the temporary file
    const size_t bufSize = 1024;
    char buf[bufSize];
    int bytesRead;
    while ((bytesRead = AAsset_read(asset, buf, bufSize)) > 0) {
        fwrite(buf, sizeof(char), bytesRead, out);
    }
    fclose(out);
    AAsset_close(asset);

    // Open the temporary NetCDF file
    int ncid, retval;
    if ((retval = nc_open(tmpFilename, NC_NOWRITE, &ncid))) {
        __android_log_print(ANDROID_LOG_ERROR, "native-lib", "Error: Failed to open file %s\n", tmpFilename);
        remove(tmpFilename); // Clean up the temporary file
        return;
    }

    // Read and print variables
    int nvars, varid;
    if ((retval = nc_inq_nvars(ncid, &nvars))) {
        __android_log_print(ANDROID_LOG_ERROR, "native-lib", "Error: Unable to get number of variables");
    } else {
        __android_log_print(ANDROID_LOG_INFO, "native-lib", "Variables in the file %s:", tmpFilename);
        char var_name[NC_MAX_NAME + 1];
        for (varid = 0; varid < nvars; varid++) {
            if ((retval = nc_inq_varname(ncid, varid, var_name)) == NC_NOERR) {
                __android_log_print(ANDROID_LOG_INFO, "native-lib", "Variable %d: %s", varid, var_name);
            } else {
                __android_log_print(ANDROID_LOG_ERROR, "native-lib", "Error: Unable to get the name of variable %d", varid);
            }
        }
    }

    // Close the NetCDF file and clean up
    nc_close(ncid);
    remove(tmpFilename); // Delete the temporary file
}
