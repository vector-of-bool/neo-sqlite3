#include <neo/sqlite3/config.hpp>

// Set the SQLite compilation options based on our library compilation options
#define SQLITE_THREADSAFE (NEO_GetSettingValue(neo_sqlite3, ThreadingMode) - 1)
#define SQLITE_OMIT_LOAD_EXTENSION NEO_FeatureIsDisabled(neo_sqlite3, LoadExtension)
#define SQLITE_ENABLE_COLUMN_METADATA NEO_FeatureIsEnabled(neo_sqlite3, ColumnMetadata)

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wfloat-conversion"
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif

#include "sqlite3-inl.h"
