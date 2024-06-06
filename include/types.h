#ifndef TYPES_H_
#define TYPES_H_

#include <rocksdb/db.h>
#include <sys/stat.h>

#include <climits>
#include <cstdint>
#include <iostream>

using rocksdb::Slice;  // Slice uses 16B for a string reference: buf & size.

struct uuid_t {
    uint64_t first;
    Slice toSlice() const {
        return Slice((char *)this, sizeof(uuid_t));
    }

    explicit operator uint64_t() const {
        return first;
    }

    uuid_t next(int k = 1) {
        uuid_t next_id = *this;
        next_id.first += k;
        return next_id;
    }
};

inline std::ostream &operator<<(std::ostream &os, const uuid_t &id) {
    os << id.first;
    return os;
}

constexpr uuid_t kRootID = {};

struct MetaKey {
    uuid_t parent_id;
    char name[NAME_MAX];
    uint8_t name_len;

    MetaKey() : parent_id(kRootID), name_len(0) {}

    MetaKey(uuid_t parent_id, const Slice &name) : parent_id(parent_id), name_len(name.size()) {
        assert(name.size() <= NAME_MAX);
        if (name.size() != 0) memcpy(this->name, name.data(), name.size());
    }

    // Deserialize from key slice.
    static MetaKey fromSlice(const Slice &slice) {
        MetaKey key;
        memcpy(&key, slice.data(), slice.size());
        key.name_len = slice.size() - sizeof(parent_id);
        assert(slice.size() - sizeof(parent_id) <= NAME_MAX);
        return key;
    }
    // Serialize to key slice.
    Slice toSlice() const {
        // Use <parent id (fixed_size) + name> as the key of metadata.
        return Slice((char *)this, sizeof(parent_id) + name_len);
    }
};

inline std::ostream &operator<<(std::ostream &os, const MetaKey &key) {
    os << "meta_key{" << key.parent_id << "/" << std::string(key.name, key.name_len) << "}";
    return os;
}

struct MetaValue {
    uuid_t id;
    struct stat st;

    Slice toSlice() const {
        return Slice((char *)this, sizeof(id) + sizeof(st));
    }
};

// Use fixed block size for data management.
constexpr int kDataBlockSize = 4096;

struct DataKey {
    uuid_t id;
    uint64_t block_no;

    DataKey() : id(kRootID), block_no(0) {}

    DataKey(uuid_t id, uint64_t block_no) : id(id), block_no(block_no) {}

    // Deserialize from key slice.
    static DataKey fromSlice(const Slice &slice) {
        DataKey key;
        memcpy(&key, slice.data(), slice.size());
        return key;
    }
    // Serialize to key slice.
    Slice toSlice() const {
        // Use <id (fixed size) + block_no (fixed size)> as the key of data.
        return Slice((char *)this, sizeof(DataKey));
    }
};

struct DataValue {
    char data[kDataBlockSize];

    Slice toSlice() const {
        return Slice((char *)this, sizeof(data));
    }
};

#endif  // TYPES_H_