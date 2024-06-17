//
// Created by martin on 28-5-/24.
//

#ifndef NETCDFREADER_H
#define NETCDFREADER_H

#include "file_reader.h"

#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <netcdf/netcdf>
#include <string>
#include <vector>

/**
 * @class NetCDFReader
 * @brief This class provides utilities for reading NetCDF files.
 */
class NetCDFReader : public FileReader {
public:
    /**
     * @brief Constructor.
     */
    NetCDFReader();

    /**
     * @brief Loads the asset manager.
     *
     * @param assetManager A pointer to the asset manager.
     */
    void loadAssetManager(AAssetManager* assetManager);

    /**
     * @brief Loads a NetCDF file.
     *
     * @param filename The name of the file to load from asset manager.
     */
    void loadFile(const std::string& filename);

    /**
     * @brief Prints the names of the variables in the NetCDF file into std::out.
     */
    void printVariableNames() const;

    /**
     * @brief Getter for the names of the variables in the NetCDF file.
     *
     * @return A reference to the vector of variable names.
     */
    const std::vector<std::string>& getVariableNames() const;

private:
    AAssetManager* mAssetManager;

    // he name of the loaded file.
    std::string mFilename;

    // The names of the variables in the loaded NetCDF file.
    std::vector<std::string> variableNames;
};

#endif // NETCDFREADER_H
