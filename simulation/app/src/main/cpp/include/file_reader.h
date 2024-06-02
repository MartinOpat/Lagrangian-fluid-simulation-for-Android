//
// Created by martin on 08-05-2024.
//

#ifndef LAGRANGIAN_FLUID_SIMULATION_FILE_READER_H
#define LAGRANGIAN_FLUID_SIMULATION_FILE_READER_H

#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include "android_logging.h"

/**
 * @class FileReader
 * @brief This class provides utilities for file reading operations.
 */
class FileReader {
public:

    /**
     * @brief Constructor
     */
    FileReader() {}

    /**
     * @brief Writes content from a file descriptor into a temporary file.
     * @param fd The file descriptor from which to read the content.
     * @param tempFilename The name of the temporary file into which to write the content.
     * @return The path to the created temporary file.
     */
    std::string writeTempFileFromFD(int fd, const std::string& tempFilename);
};


#endif //LAGRANGIAN_FLUID_SIMULATION_FILE_READER_H
