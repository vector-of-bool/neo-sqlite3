#pragma once

#include <neo/platform.hpp>

#if !__dds_header_check && defined(NEO_PRAGMA_WARNING)
NEO_PRAGMA_WARNING("This file is deprecated. Use <neo/sqlite3/connection.hpp> instead.");
#endif

#include "./connection.hpp"
