#include "fuse_ops.h"

#include <unistd.h>

#include "data_client.h"
#include "id_allocator.h"
#include "kvs.h"
#include "types.h"

KVStore *kvs;               // for metadata & data storage
IDAllocator *id_allocator;  // for inode ID
DataClient *data_client;    // (lab3) for distributed data store

// Get the first component of a path. E.g., "/a/b/c" -> "a"
Slice firstPathComponent(const char *path) {
    // In fuse file systems, we can assume that all the paths are absolute (i.e., without "." and "..").
    const char *start, *end;
    for (start = path; *start == '/'; ++start) {}  // skip all the beginning slashes
    for (end = start; *end != '/' && *end != '\0'; ++end) {}
    return Slice(start, end - start);
}

void op_init(const std::string &kvs_path) {
    LOG(INFO) << "Initialize KV store at " << kvs_path;
    kvs = new KVStore(kvs_path);
    id_allocator = new IDAllocator(kvs);
    // NOTE: Root directory's key is special: "kRootID + empty string"
    MetaKey key(kRootID, Slice());
    if (!kvs->contains(key.toSlice())) {
        MetaValue val{};
        val.id = kRootID;
        val.st.st_atime = val.st.st_mtime = val.st.st_ctime = time(nullptr);
        val.st.st_mode = S_IFDIR | 0755;
        val.st.st_uid = getuid();
        val.st.st_gid = getgid();
        val.st.st_nlink = 2;
        val.st.st_blksize = kDataBlockSize;
        val.st.st_ino = (uint64_t)(val.id);
        kvs->put(key.toSlice(), val.toSlice());
    }

    // (lab3) Initialize the data client.
    // In lab1 and lab2, it can be ignored as we use the local KV store for data storage.
    data_client = new DataClient();
    data_client->connect("./conf/data_servers");
}

int lookup(const char *path, MetaKey *key) {
    // Currently, lookup only supports the root directory.
    // Permission check is omitted.
    // TODO(lab2.2): recursive lookup in a directory tree
    *key = MetaKey(kRootID, firstPathComponent(path));
    return 0;
}

int op_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, fuse_file_info *fi,
               fuse_readdir_flags flags) {
    // Get all the children in a directory.
    MetaKey key{};
    int ret = lookup(path, &key);
    if (ret != 0) return ret;

    // Here we get the target directory's ID using its key.
    MetaValue val;
    if (!kvs->get(key.toSlice(), &val)) return -ENOENT;
    if (!S_ISDIR(val.st.st_mode)) return -ENOTDIR;

    // Note: the children in a directory have the same prefix <directory_ID>.
    kvs->matchPrefix(val.id.toSlice(), [&](const MetaKey &key, MetaValue &value) {
        // Using Slice for filename is not true here, as it may not end with '\0'.
        auto filename = std::string(key.name, key.name_len);
        if (filename.size() != 0) {  // the key of the root directory has name_len == 0.
            filler(buf, filename.data(), &value.st, 0, (fuse_fill_dir_flags)0);
        }
    });
    return 0;
}

int op_getattr(const char *path, struct stat *stbuf, fuse_file_info *fi) {
    // Get the metadata of a file.
    MetaKey key;
    int ret = lookup(path, &key);
    if (ret != 0) return ret;

    MetaValue val;
    if (kvs->get(key.toSlice(), &val)) {
        *stbuf = val.st;
        return 0;
    }
    return -ENOENT;
}

int op_create(const char *path, mode_t mode, fuse_file_info *fi) {
    // Create a file.
    MetaKey key;
    int ret = lookup(path, &key);
    if (ret != 0) return ret;

    // First, check if the file exists.
    if (kvs->contains(key.toSlice())) {
        return -EEXIST;
    }

    // If it does not exist, create a new file.
    MetaValue val{};
    val.id = id_allocator->alloc();
    val.st.st_atime = val.st.st_mtime = val.st.st_ctime = time(nullptr);
    val.st.st_mode = mode | S_IFREG;
    val.st.st_uid = getuid();
    val.st.st_gid = getgid();
    val.st.st_nlink = 1;
    val.st.st_size = 0;
    val.st.st_blksize = kDataBlockSize;
    val.st.st_ino = (uint64_t)(val.id);
    kvs->put(key.toSlice(), val.toSlice());
    return 0;
}

int op_unlink(const char *path) {
    // Simple version of unlink without hard links.
    // Here unlink is equal to remove a file.
    MetaKey key;
    int ret = lookup(path, &key);
    if (ret != 0) return ret;

    MetaValue val;
    if (!kvs->get(key.toSlice(), &val)) return -ENOENT;
    if (S_ISDIR(val.st.st_mode)) return -EISDIR;

    // Remove all the data blocks.
    int n_blocks = (val.st.st_size + kDataBlockSize - 1) / kDataBlockSize;
    for (int i = 0; i < n_blocks; ++i) {
        DataKey data_key(val.id, i);
        kvs->remove(data_key.toSlice());
    }

    // Remove the file's metadata.
    kvs->remove(key.toSlice());
    return 0;
}

int op_utimens(const char *path, const struct timespec tv[2], struct fuse_file_info *fi) {
    MetaKey key;
    int ret = lookup(path, &key);
    if (ret != 0) return ret;

    // Perform read-modify-write.
    MetaValue val;
    if (!kvs->get(key.toSlice(), &val)) return -ENOENT;
    uint64_t now = time(nullptr);
    if (tv[0].tv_nsec == UTIME_NOW || tv[1].tv_nsec == UTIME_NOW) {
        val.st.st_atime = val.st.st_mtime = now;
    } else {
        val.st.st_atime = tv[0].tv_sec;
        val.st.st_mtime = tv[1].tv_sec;
    }
    val.st.st_ctime = now;  // utimens updates ctime.
    kvs->put(key.toSlice(), val.toSlice());
    return 0;
}

int op_open(const char *path, fuse_file_info *fi) {
    // Open a file.
    // Creation (O_CREAT, O_EXCL, O_NOCTTY) flags are handled by the kernel.
    // Other special flags (e.g., O_RDONLY, O_APPEND) are omitted here.
    struct stat stbuf;
    return op_getattr(path, &stbuf, fi);
}

int op_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    // Read data from a file.
    // Only supports reading the first data block [0, kDataBlockSize).
    // TODO(lab1): support reading more data blocks.
    // TODO(lab3): read each block from the data server instead of the local KV store, using "data_client".
    assert(offset + size <= kDataBlockSize);
    MetaKey key;
    int ret = lookup(path, &key);
    if (ret != 0) return ret;

    // Get the target file's ID.
    MetaValue val;
    if (!kvs->get(key.toSlice(), &val)) return -ENOENT;
    if (S_ISDIR(val.st.st_mode)) return -EISDIR;

    // Get the actual file size and the bytes to read.
    off_t file_size = val.st.st_size;
    off_t bytes_to_read = std::min((off_t)size, file_size - offset);  // The actual bytes to read.
    if (bytes_to_read <= 0) return 0;

    // Read the corresponding data blocks.
    DataKey data_key(val.id, offset / kDataBlockSize);
    DataValue data_val;
    if (!kvs->get(data_key.toSlice(), &data_val)) {
        // If the data block does not exist, fill with 0.
        memset(data_val.data, 0, kDataBlockSize);
    }
    int transfer_size = bytes_to_read;
    assert(transfer_size <= kDataBlockSize);
    memcpy(buf, data_val.data + offset, transfer_size);
    return bytes_to_read;  // Read returns the actual bytes read if success.
}

int op_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    // Write data to a file.
    // Only supports writing the first data block [0, kDataBlockSize).
    // TODO(lab1): support writing more data blocks.
    // TODO(lab3): write each block to the data server instead of the local KV store, using "data_client".
    assert(offset + size <= kDataBlockSize);

    MetaKey key;
    int ret = lookup(path, &key);
    if (ret != 0) return ret;

    // Get the target file's ID.
    MetaValue val;
    if (!kvs->get(key.toSlice(), &val)) return -ENOENT;
    if (S_ISDIR(val.st.st_mode)) return -EISDIR;

    // Do read-modify-write in case of unaligned writes.
    DataKey data_key(val.id, offset / kDataBlockSize);
    DataValue data_val;
    // 1. read
    if (!kvs->get(data_key.toSlice(), &data_val)) {
        // If the data block does not exist, fill with 0.
        memset(data_val.data, 0, kDataBlockSize);
    }
    // 2. modify
    int transfer_size = size;
    assert(transfer_size <= kDataBlockSize);
    memcpy(data_val.data + offset, buf, transfer_size);
    // 3. write
    kvs->put(data_key.toSlice(), data_val.toSlice());

    // Update the file size and timestamps.
    val.st.st_size = std::max(val.st.st_size, offset + (off_t)size);
    val.st.st_mtime = val.st.st_ctime = time(nullptr);
    kvs->put(key.toSlice(), val.toSlice());
    return size;  // Write returns the requested bytes if success.
}

int op_truncate(const char *path, off_t size, struct fuse_file_info *fi) {
    MetaKey key;
    int ret = lookup(path, &key);
    if (ret != 0) return ret;

    MetaValue val;
    if (!kvs->get(key.toSlice(), &val)) return -ENOENT;
    if (S_ISDIR(val.st.st_mode)) return -EISDIR;

    // Remove the data blocks that are no longer needed.
    int n_blocks = (val.st.st_size + kDataBlockSize - 1) / kDataBlockSize;
    int n_new_blocks = (size + kDataBlockSize - 1) / kDataBlockSize;
    for (int i = n_new_blocks; i < n_blocks; ++i) {
        DataKey data_key(val.id, i);
        kvs->remove(data_key.toSlice());
    }

    // Update the file size and timestamps.
    val.st.st_size = size;
    val.st.st_mtime = val.st.st_ctime = time(nullptr);
    kvs->put(key.toSlice(), val.toSlice());
    return 0;
}

int op_fsync(const char *path, int datasync, struct fuse_file_info *fi) {
    // For current KV Store (RocksDB), syncing WAL ensures the durability of all the KV pairs.
    // The key is not mandatory, so we use an empty string.
    kvs->sync("");
    return 0;
}

int op_close(const char *path, fuse_file_info *fi) {
    // Need to do nothing as we do not save any file descriptor.
    return 0;
}

int op_mkdir(const char *path, mode_t mode) {
    // TODO(lab2.1): implement mkdir
    // Create a new directory, similar to op_create.
    return -ENOSYS;
}

int op_rmdir(const char *path) {
    // TODO(lab2.1): implement rmdir
    // Remove the directory, similar to op_unlink.
    return -ENOSYS;
}

int op_chmod(const char *path, mode_t mode, struct fuse_file_info *fi) {
    // TODO(lab2.1): implement chmod
    // Only change the permission bits, not the whole mode
    return -ENOSYS;
}

int op_rename(const char *from, const char *to, unsigned int flags) {
    // TODO(lab2.2): implement rename
    // Rename "from" to "to", no need to care for the flags (i.e., RENAME_EXCHANGE or RENAME_NOREPLACE).
    // Namely, it should replace the "to" directory (should be empty) or file.
    // Consider the atomicity of this operation.
    return -ENOSYS;
}
