#ifndef ID_ALLOCATOR_H_
#define ID_ALLOCATOR_H_

#include "kvs.h"

struct IDAllocator {
    static constexpr char kIDKey[] = "id";
    static constexpr int kIDBlockSize = 1024;
    uuid_t cur_id;
    KVStore *kvs;

    IDAllocator(KVStore *kvs) : kvs(kvs) {
        cur_id.first = 1;
        if (kvs->get(kIDKey, &cur_id)) {
            // skip current block, as it's already allocated in previous startup.
            cur_id = cur_id.next(kIDBlockSize);
            LOG(INFO) << "Restore next metadata ID to " << cur_id;
        } else {
            LOG(INFO) << "Initialize next metadata ID to " << cur_id;
            kvs->put(kIDKey, cur_id.toSlice());
        }
    }

    // Use "batch allocate" for inode ID.
    // Do not reuse ID.
    uuid_t alloc() {
        uuid_t ret = cur_id;
        cur_id = cur_id.next();
        if (cur_id.first % kIDBlockSize == 1) {
            kvs->put(kIDKey, cur_id.toSlice());
        }
        return ret;
    }
};

extern IDAllocator *id_allocator;

#endif  // ID_ALLOCATOR_H_