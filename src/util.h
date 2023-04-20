#pragma once

bool fileExists(const char* path);

// Macros for reading and writing binary data
#define WRITEOUT(stream, var) do {                   \
auto local = (var);                                  \
(stream).write((const char*)&(local), sizeof(local));\
} while( false )
#define READIN(stream, var) (stream).read((char*)&(var), sizeof(var))
