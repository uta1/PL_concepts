/*! \file */

#pragma once

#include <sys/stat.h>
#include <fstream>

/*! ---------------------------------------
* Check validity of argument (must be file)
* 
* @param [in] path        Path to checking file
*
* @returns True if valid and false else
*
* -----------------------------------------
*/
bool isValidFile(const char* path) {
    struct stat fileStat = {0};
    
    stat(path, &fileStat);
    if (S_ISREG(fileStat.st_mode)) {
        return true;
    }
    
    return false;
}

/*! ---------------------------------------
* Gets size if file
* 
* @param [in] path        Path to file
*
* @returns Size of file in bytes
*
* @note File path and file must be validated
*       by isValidFile(const char* path)
*
* -----------------------------------------
*/
int getFileSize(const char* path) {
    struct stat fileStat = {0}; 
    
    stat(path, &fileStat);
    
    return fileStat.st_size;
}

/*! ---------------------------------------
* Reads file 
* 
* @param [in] path        Path to file
* @paran [in] fileSize    Size of file
*
* @returns Pointer on start of read string
*
* @note Fuctions allocates fileSize + 1 blocks
*       of char for the return value
*       BUT NOT DELETED: it's up to you
*
* -----------------------------------------
*/
char* readFile(const char* path, int fileSize) {
    char* result = (char*)malloc((fileSize + 1) * sizeof(char));
    std::ifstream is{path};
    
    is.read(result, fileSize);
    
    is.close();

    return result;
}
