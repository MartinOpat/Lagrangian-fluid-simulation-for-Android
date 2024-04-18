//
// Created by martin on 18-04-2024.
//

#ifndef LAGRANGIAN_FLUID_SIMULATION_TEST_READ_NC_H
#define LAGRANGIAN_FLUID_SIMULATION_TEST_READ_NC_H

#ifdef __cplusplus
extern "C" {
#endif

void print_nc_vars(const char *filepath);
void print_nc_vars_from_asset(AAssetManager *assetManager, const char *filename);

#ifdef __cplusplus
}
#endif

#endif //LAGRANGIAN_FLUID_SIMULATION_TEST_READ_NC_H
