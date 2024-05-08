//
// Created by martin on 08-05-2024.
//

#ifndef LAGRANGIAN_FLUID_SIMULATION_FILE_READER_H
#define LAGRANGIAN_FLUID_SIMULATION_FILE_READER_H

#include <iostream>

class FileReader {
public:
    // Constructor
    FileReader() {}

    // Utility function to write content from a file descriptor into a temporary file
    std::string writeTempFileFromFD(int fd, const std::string& tempFilename);
};


#endif //LAGRANGIAN_FLUID_SIMULATION_FILE_READER_H
