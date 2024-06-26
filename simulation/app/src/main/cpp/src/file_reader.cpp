//
// Created by martin on 08-05-2024.
//

#include "include/file_reader.h"

std::string FileReader::writeTempFileFromFD(int fd, const std::string& tempFilename) {
    // Generate path for the temporary file in the app's internal storage

    // TODO: Fix this mess

    // main app
//    std::string tempFilePath = "/data/data/com.rug.lagrangianfluidsimulation/tmp/" + tempFilename;
//    mkdir("/data/data/com.rug.lagrangianfluidsimulation/tmp/", 0777);

    // Double gyre
//    std::string tempFilePath = "/data/data/com.rug.lagrangianfluidsimulation.doublegyre/tmp/" + tempFilename;
//    mkdir("/data/data/com.rug.lagrangianfluidsimulation.doublegyre/tmp/", 0777);

    // Perlin
    std::string tempFilePath = "/data/data/com.rug.lagrangianfluidsimulation.perlin/tmp/" + tempFilename;
    mkdir("/data/data/com.rug.lagrangianfluidsimulation.perlin/tmp/", 0777);

    // Ensure the directory exists

    // Create and open the temporary file
    int tempFd = open(tempFilePath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (tempFd == -1) {
        LOGE("file_reader", "Failed to open temporary file for writing");
        return "";
    }

    // Rewind the source descriptor
    lseek(fd, 0, SEEK_SET);

    // Copy data from the file descriptor to the temporary file
    char buffer[1024];
    ssize_t bytesRead;
    while ((bytesRead = read(fd, buffer, sizeof(buffer))) > 0) {
        if (write(tempFd, buffer, bytesRead) != bytesRead) {
            LOGE("file_reader", "Failed to write all bytes to temporary file");
            close(tempFd);
            return "";
        }
    }

    close(tempFd);

    return tempFilePath;
}