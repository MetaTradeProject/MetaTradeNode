#include <leveldb/db.h>
#include <leveldb/write_batch.h>
#include <iostream>
#pragma comment(lib , "leveldb_debug.lib")

void view(std::string name) {
	leveldb::DB* db;
	leveldb::Status s;
	leveldb::Options options;
	s = leveldb::DB::Open(options, name, &db);
	assert(s.ok());

	leveldb::Iterator* it = db->NewIterator(leveldb::ReadOptions());
	for (it->SeekToFirst(); it->Valid(); it->Next()) {
		std::cout << it->key().ToString() << ": " << it->value().ToString() << std::endl;
	}
	assert(it->status().ok());  // Check for any errors found during the scan
	delete it;
	
}