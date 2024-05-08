//
// Created by martin on 08-05-2024.
//

#include "file_reader.h"
#include "android_logging.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

std::string FileReader::writeTempFileFromFD(int fd, const std::string& tempFilename) {
    // Generate path for the temporary file in the app's internal storage
    std::string tempFilePath = "/data/data/com.example.lagrangianfluidsimulation/tmp/" + tempFilename;

    // Ensure the directory exists
    mkdir("/data/data/com.example.lagrangianfluidsimulation/tmp/", 0777);

    // Create and open the temporary file
    int tempFd = open(tempFilePath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (tempFd == -1) {
        __android_log_print(ANDROID_LOG_ERROR, "native-lib", "Failed to open temporary file for writing");
        return "";
    }

    // Rewind the source descriptor to ensure it's read from the start
    lseek(fd, 0, SEEK_SET);

    // Copy data from the file descriptor to the temporary file
    char buffer[1024];
    ssize_t bytesRead;
    while ((bytesRead = read(fd, buffer, sizeof(buffer))) > 0) {
        if (write(tempFd, buffer, bytesRead) != bytesRead) {
            __android_log_print(ANDROID_LOG_ERROR, "native-lib", "Failed to write all bytes to temporary file");
            close(tempFd);
            return "";
        }
    }

    close(tempFd);
    return tempFilePath;
}