#ifndef DATA_CLIENT_H_
#define DATA_CLIENT_H_

// Data client which interacts with data servers.
// Only used in Lab 4 and Lab 5 (distributed data service).

#include <rpc/client.h>

#include <fstream>
#include <string>
#include <vector>

#include "log.h"
#include "types.h"

class DataClient {
public:
    // Assume that cwd is the project's root directory.
    void connect(const std::string &conf_file) {
        std::fstream file;
        file.open(conf_file, std::ios::in);
        if (!file.is_open()) {
            LOG(ERROR) << "Data server config file not found: " << conf_file;
            exit(1);
        }

        // The config file includes several lines, each line is a data server's {ip}:{port}.
        std::string line;
        while (std::getline(file, line)) {
            // line == {ip}:{port}
            size_t pos = line.find(':');
            if (pos == std::string::npos) continue;
            std::string ip = line.substr(0, pos);
            int port = std::stoi(line.substr(pos + 1));
            rpc::client *client = new rpc::client(ip, port);
            client->set_timeout(1000);
            clients.push_back(client);
        }
    }

    rpc::client *clientOf(const DataKey &key) {
        // TODO(lab3): get the corresponding data servers according to the data key.
        // You can use std::hash<std::string>{}(str); for hashing.
        return clients[0];
    }

    // Write inside a single object.
    // @param key: the key of the data block.
    // @param data: the data to write in the object.
    // @param offset: intra-object offset.
    // @return: the number of bytes written.
    int obj_write(const DataKey &key, const std::string &data, off_t offset) {
        auto client = clientOf(key);
        return client->call("obj_write", key.toSlice().ToString(), data, offset).as<int>();
    }

    // Read inside a single object.
    // @param key: the key of the data block.
    // @param offset: intra-object offset.
    // @param size: the number of bytes to read for this object.
    // @return: the data read.
    std::string obj_read(const DataKey &key, off_t offset, size_t size) {
        // TODO(lab3): Call intra-object read function.
        return "";
    }

    std::vector<rpc::client *> clients;
};

#endif  // DATA_CLIENT_H_
