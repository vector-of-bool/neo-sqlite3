#include "./blob.hpp"

#include <neo/sqlite3/c/sqlite3.h>

using namespace neo::sqlite3;

#define MY_BLOB_PTR static_cast<::sqlite3_blob*>(_ptr)

blob::~blob() {
    ::sqlite3_blob_close(MY_BLOB_PTR);
}