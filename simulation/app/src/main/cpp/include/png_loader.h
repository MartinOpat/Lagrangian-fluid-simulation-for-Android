//
// Created by martin on 16-04-2024.
//

#ifndef LAGRANGIAN_FLUID_SIMULATION_PNG_LOADER_H
#define LAGRANGIAN_FLUID_SIMULATION_PNG_LOADER_H

#include <fstream>
#include <vector>
#include <stdexcept>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/log.h>
#include "android_logging.h"




struct ImageData {
    int width;
    int height;
    std::vector<unsigned char> data; // RGBA format
};

ImageData loadSimpleTGA(AAssetManager* assetManager, const char* filename);

#endif //LAGRANGIAN_FLUID_SIMULATION_PNG_LOADER_H