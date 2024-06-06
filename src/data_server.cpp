// Data server supporting read & write of data blocks.
// Only used in Lab 4 and Lab 5 (distributed data service).

#include <rpc/server.h>

#include <iostream>
#include <string>

#include "kvs.h"
#include "log.h"

KVStore *kvs = nullptr;

// @param data_key_str: the object's key (serialized).
// @param data: the data to write, with size <= kDataBlockSize.
// @param offset: "intra-object" offset within [0, kDataBlockSize).
// @return: the number of bytes written.
int obj_write(const std::string &data_key_str, const std::string &data, off_t offset) {
    // TODO(lab3): Intra-object write, may require read-modify-write.
    DataKey data_key = DataKey::fromSlice(Slice(data_key_str));
    return data.size();
}

// @param data_key_str: the object's key (serialized).
// @param offset: "intra-object" offset within [0, kDataBlockSize).
// @param size: the number of bytes to read for this object.
// @return: the data read, with size <= kDataBlockSize.
std::string obj_read(const std::string &data_key_str, off_t offset, size_t size) {
    // TODO(lab3): Intra-object read.
    DataKey data_key = DataKey::fromSlice(Slice(data_key_str));
    return "";
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        LOG(INFO) << "Usage: " << argv[0] << " <port> <id>";
        return 1;
    }
    int port = std::stoi(argv[1]);
    int id = std::stoi(argv[2]);

    std::string kv_path = "/tmp/data_server_" + std::to_string(id);
    LOG(INFO) << "Opening KV store at " << kv_path << " ...";
    kvs = new KVStore(kv_path);

    LOG(INFO) << "Data server " << id << " is listening on port " << port;
    rpc::server srv("0.0.0.0", port);

    srv.bind("obj_write", obj_write);
    srv.bind("obj_read", obj_read);
    srv.run();
    return 0;
}