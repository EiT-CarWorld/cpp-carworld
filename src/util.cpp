#include "util.h"
#include <sys/stat.h>

bool fileExists(const char* path) {
    struct stat buffer;
    return (stat (path, &buffer) == 0);
}