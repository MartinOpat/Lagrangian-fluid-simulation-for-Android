#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <string>
#include <vector>
#include "H5Cpp.h"

class HDF5Reader {
public:
    HDF5Reader();
    void loadAssetManager(AAssetManager* assetManager);
    void loadFile(const std::string& filename);
    void printDatasetNames() const;
    const std::vector<std::string>& getDatasetNames() const;
    std::string writeTempFileFromFD(int fd, const std::string& tempFilename);

private:
    AAssetManager* mAssetManager;
    std::string mFilename;
    std::vector<std::string> datasetNames;
    H5::H5File* file;
};