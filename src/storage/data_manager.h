#pragma once

#include "data_record.h"
#include "sqlite3.h"
/*
class DataManager {
public:
    bool createTables() noexcept;
    bool createRecord(DbRecord&) noexcept;
    bool readRecord() noexcept;
    bool updateRecord() noexcept;
    bool deleteRecord(uint64_t) noexcept;
    bool getAllRecords() noexcept;

    static DbRecord convertToDbRecord(DataRecordView&) noexcept;
    static DataRecordView convertFromDbRecord(DbRecord&) noexcept;
    static void serializeTLV() noexcept;

private:
    std::unordered_map<uint64_t, DbRecord> allRecords;
    sqlite3* m_db;
};
*/