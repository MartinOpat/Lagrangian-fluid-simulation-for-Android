// NetCDFReader.cpp
#include <iostream>
#include <fstream>
#include <vector>
#include <cstdio>
#include <fcntl.h>
#include <unistd.h>
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>


#include "netcdf_reader.h"
#include "android_logging.h"


NetCDFReader::NetCDFReader() {
    mAssetManager = nullptr;
    mFilename = "";
    variableNames = {};
}

void NetCDFReader::loadAssetManager(AAssetManager* assetManager) {
    mAssetManager = assetManager;
}

void NetCDFReader::loadFile(const std::string& filename) {
    if (mAssetManager == nullptr) {
        LOGE("netcdf-reader", "Asset manager not loaded");
        return;
    }

    AAsset* asset = AAssetManager_open(mAssetManager, mFilename.c_str(), AASSET_MODE_UNKNOWN);
    if (!asset) {
        std::cerr << "Failed to open asset: " << mFilename << std::endl;
        return;
    }

    off_t length = AAsset_getLength(asset);
    char* buffer = new char[length];
    AAsset_read(asset, buffer, length);
    AAsset_close(asset);

    std::string tempFilename = "/tmp/tempfile.nc";
    std::ofstream outFile(tempFilename, std::ios::binary);
    outFile.write(buffer, length);
    outFile.close();
    delete[] buffer;

    try {
        netCDF::NcFile dataFile(tempFilename, netCDF::NcFile::read);
        auto vars = dataFile.getVars();
        for (const auto& var : vars) {
            variableNames.push_back(var.first); // Store the name of each variable
        }
    } catch (netCDF::exceptions::NcException& e) {
        std::cerr << e.what() << std::endl;
    }

    std::remove(tempFilename.c_str());
}

void NetCDFReader::printVariableNames() const {

    std::cout << "Variables in the NetCDF file:" << std::endl;
    if (variableNames.empty()) {
        LOGE("netcdf-reader", "No variables found or no file loaded!");
        return;
    }
    for (const auto& varName : variableNames) {
        std::cout << varName << std::endl;
    }
}

const std::vector<std::string>& NetCDFReader::getVariableNames() const {
    return variableNames;
}

