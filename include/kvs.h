#ifndef KVS_H_
#define KVS_H_

#include <fuse3/fuse.h>
#include <types.h>

#include "log.h"

class KVStore {
public:
    std::string db_name;
    rocksdb::DB *db;

    KVStore(std::string path) : db_name(path) {
        rocksdb::Options options;
        options.create_if_missing = true;
        rocksdb::Status status = rocksdb::DB::Open(options, db_name, &db);
        if (!status.ok()) {
            LOG(INFO) << "Failed to open db: " << db_name;
            exit(1);
        }
    }
    ~KVStore() {
        delete db;
    }

    template<class T>
    int get(const Slice &key, T *value) {
        std::string v;
        auto s = db->Get(rocksdb::ReadOptions(), key, &v);
        if (s.ok()) {
            assert(v.size() == sizeof(T));
            memcpy(value, v.data(), v.size());
            return true;
        }
        return false;
    }

    int contains(const Slice &key) {
        std::string v;
        auto s = db->Get(rocksdb::ReadOptions(), key, &v);
        return s.ok();
    }

    int put(const Slice &key, const Slice &value) {
        auto opt = rocksdb::WriteOptions();
        auto s = db->Put(opt, key, value);
        return s.ok();
    }

    int remove(const Slice &key) {
        auto opt = rocksdb::WriteOptions();
        auto s = db->Delete(opt, key);
        return s.ok();
    }

    // Assume that all the matched KV pairs are MetaKey & MetaValue.
    int matchPrefix(const Slice &prefix, std::function<void(const MetaKey &, MetaValue &)> func) {
        auto it = db->NewIterator(rocksdb::ReadOptions());
        it->Seek(prefix);
        while (it->Valid()) {
            // Avoid the special keys (e.g., id) and the keys in other directories.
            if (it->key().size() < prefix.size() || !it->key().starts_with(prefix)) break;  // terminates.
            MetaKey key = MetaKey::fromSlice(it->key());
            MetaValue value;
            memcpy(&value, it->value().data(), sizeof(MetaValue));
            func(key, value);
            it->Next();
        }
        delete it;
        return 0;
    }

    int sync(const Slice &key) {
        db->SyncWAL();
        return true;
    }
};

extern KVStore *kvs;

#endif  // KVS_H_
