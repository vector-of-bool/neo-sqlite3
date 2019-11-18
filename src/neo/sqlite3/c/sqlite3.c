// Set the threading mode to multi-threaded, not serialized nor single-threaded
#define SQLITE_THREADSAFE 2
// Disable extension loading so we have no requirement on ::dlopen
#define SQLITE_OMIT_LOAD_EXTENSION 1
// Additional column metadata
#define SQLITE_ENABLE_COLUMN_METADATA 1

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif

#include "sqlite3-inl.h"
