#include "./blob.hpp"

#include <sqlite3/sqlite3.h>

using namespace neo::sqlite3;

#define MY_BLOB_PTR static_cast<::sqlite3_blob*>(_ptr)

blob::~blob() { ::sqlite3_blob_close(MY_BLOB_PTR); }