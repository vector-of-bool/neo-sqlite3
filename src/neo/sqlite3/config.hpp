#pragma once

#if __has_include(<neo/sqlite3.tweaks.hpp>)
#include <neo/sqlite3.tweaks.hpp>
#endif

#include <neo/config-pp.hpp>

/**
 * @def neo_sqlite3_ToggleFeature_LoadExtension
 *      Enable/disable runtime loading of binary extensions in SQLite. On POSIX,
 *      enabling this feature will require that all generated executables support
 *      dlopen(), thus creating a runtime dependency on the 'dl' library. Default
 *      is 'Disabled'
 */
#ifndef neo_sqlite3_ToggleFeature_LoadExtension
#define neo_sqlite3_ToggleFeature_LoadExtension Disabled
#endif

/**
 * @def neo_sqlite3_ToggleFeature_ColumnMetadata
 *      Enable/disable support for column metadata. Default is 'Enabled'
 */
#ifndef neo_sqlite3_ToggleFeature_ColumnMetadata
#define neo_sqlite3_ToggleFeature_ColumnMetadata Enabled
#endif

/**
 * @def neo_sqlite3_Setting_ThreadingMode: Set to one of 'SingleThreaded',
 *      'Serialized', or 'Multithreaded' to configure the thread-safety of
 *      SQLite. Refer: https://sqlite.org/threadsafe.html
 *
 * The default value is 'MultiThreaded'
 */
#ifndef neo_sqlite3_Setting_ThreadingMode
#define neo_sqlite3_Setting_ThreadingMode MultiThreaded
#endif

/// Possible values for ThreadingMode: Each is +1 of the value that SQLite expects
/// for SQLITE_THREADSAFE, so we just subtract one to get the real value.
#define neo_sqlite3_SettingVal_ThreadingMode_SingleThreaded +1
#define neo_sqlite3_SettingVal_ThreadingMode_Serialized +2
#define neo_sqlite3_SettingVal_ThreadingMode_MultiThreaded +3

#if NEO_FeatureIsDisabled(neo_sqlite3, ColumnMetadata)
// Annotate column-metadata APIs to produce a warning if it is disabled by the build config
#define NEO_SQLITE3_COLUMN_METADATA_API                                                            \
    [[deprecated(                                                                                  \
        "Column metadata is not enabled in this build of neo-sqlite3, and will always "            \
        "yield empty results")]]
#else
#define NEO_SQLITE3_COLUMN_METADATA_API
#endif
