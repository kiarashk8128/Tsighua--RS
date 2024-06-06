# KVFS: A File System Based on a KV Store
Course project of Storage Basis, 2024 spring

## Brief Description

* In this lab, you will write C++ code to complete KVFS, a simple FUSE user-space file system. 

* The architecture of KVFS is similar to [SingularFS](https://www.usenix.org/system/files/atc23-guo.pdf) and [TableFS](https://www.usenix.org/system/files/conference/atc13/atc13-ren.pdf). A similar architecture is also adopted by recent works in distributed file systems (e.g., LocoFS and InfiniFS).

* This repository is part of KVFS, it supports:
    * Metadata operations in the root directory, including `readdir` and `getattr`.
    * Part of metadata operations, including `create`、`getattr`、`delete`、`truncate`、`utimens`.
    * Read or write the first 4KB block of a file.

* There are 3 basic labs and 1 extended lab. For each basic lab, you should:
    * Modify the code according to the "TODO" to pass the given tests.

    * Answer the questions after the lab.

## Code Structure
```
kvfs
├── build (binary and temporary files during compiling)
├── conf (config file)
│   └── data_servers (the IP and port of data servers in lab3, no need to modify)
├── fs (the mount point of KVFS)
├── include (headers)
│   ├── data_client.h (code of data client, only used in lab3)
│   ├── fuse_ops.h (declaration of FUSE interfaces, no need to modify)
│   ├── id_allocator.h (inode ID allocator, no need to modify)
│   ├── kvs.h (interfaces to connect to RocksDB KV store, modify if you choose the "KV Store Backend" extended lab)
│   ├── log.h (definition of LOG and DLOG macros for replacing printf, no need to modify)
│   └── types.h (structure of metadata object and data block, modify if you choose the "Paper Reproduction" extended lab)
├── src
│   ├── data_server.cpp (code of data server, only used in lab3)
│   ├── fuse_ops.cpp (definition of FUSE interfaces, used in lab1/2/3)
│   └── main.cpp (main function of KVFS, no need to modify)
├── tests (tests for each basic lab)
├── third_party (rpclib library, for network communication in lab3, no need to modify)
├── ...
├── build.sh (compile script)
├── mount_debug.sh (mount KVFS in the frontend, for debugging and testing)
└── test.sh (script to run the given tests for each basic lab)
```

## Basic Lab

In the basic lab, you will learn the key points for implementing a file system, namely, metadata management and data management.

### Lab 0: Quick Start

#### Goals

According to the tutorial below, mount KVFS and pass the tests for lab 0 (`./test.sh 0`).

The correct output is as follows.
```bash
$ ./test.sh 0
(Some other outputs)
Running test tests/lab0.py...
Number of files: 100
Create files & write their first block.
List the root directory.
Read file.
Overwrite in the middle.
Unaligned read.
Verify file content.
Delete files.
All tests passed.
```

#### Dependency

* You should run KVFS in Linux, using WSL is OK here.

* Dependency: `fuse3` and `rocksdb`. If you use Ubuntu, you could run `./pkgdeps.sh` for these dependencies.

#### Run and Debug

* Note: **You should run all the scripts in the root of the repository.**

* Step 1: run `./build.sh` to generate the FUSE file system binary file `build/bin/main`.

* Step 2: run `./mount_debug.sh` to mount the file system to `./fs` directory in the front end. The KV store it relies on is placed in `/tmp/fskv`, which is cleared after each execution of the script.
    * The outputs in the terminal include the operations and the returned values, which may assist you in debugging.

* Step 3: run `./test.sh {lab_id}` in another terminal. In this lab, you should use `./test.sh 0`.

#### Mount as a Daemon

* After Step 1, run `./mount_daemon.sh` to mount the file system as a daemon. It will not clear the KV store in `/tmp/fskv`.

* If you want to change the mount point or the place of the KV store, you could modify the script.

### Lab 1: Data Management

#### Description

Data management is an important part of file systems. File systems should support read or write any segment (denoted as `offset` + `size`) of any file (identified by `path`). However, current KVFS only supports reading or writing the first block of a file.

In this lab, you should add code to enable KVFS to read or write any segment of a file, namely, any offset and any size. You are suggested to only modify or add code in functions marked as `TODO(lab1)` in `fuse_ops.cpp`.

#### Goals

Pass the tests for lab 1 (`./test.sh 1`) and answer the questions below.

The correct output is as follows.
```bash
$ ./test.sh 1
(Some other outputs)
Running test tests/lab0.py...
Number of files: 100
...
Delete files.
Running test tests/lab1.py...
Block size by stat: 4096B
File size: 2000KB
...
Write case 15: random range [62489, 196493).
Read case 15: random range [62489, 196493).
Delete files.
All tests passed.
```

#### Hints

* Read the code to understand how to use key-value (KV) for metadata (the relationship between KV and the triple of ID, file name, and block ID), and answer Q1 and Q2.

* Modify the code: iterate the KV pairs for each data block, directly write for aligned blocks, do read-modify-write for unaligned blocks.

#### Questions

1. In KVFS, how are metadata and data blocks mapped to KV objects? Why don't they conflict even if they share a similar key format?

2. To index to block 0 of file /a, which KV operations should KVFS do (let the ID of / be 0, the ID of /a be 1)?

### Lab 2: Metadata Management

#### Description

Another part of file systems is metadata management. It maintains a directory tree structure and attributes for directories and files (e.g., size and permission). Users can use metadata operations for directory tree reading and modification. However, current KVFS only supports (1) part of the metadata operations and (2) single-level directory tree (i.e., only the root directory).

In this experiment, we will solve these questions, including:

Lab 2.1: Metadata Operations

* Directory tree modification: directory create (`mkdir`), directory remove (`rmdir`).

* Metadata modification: permission changing (`chmod`).

Lab 2.2: Path Resolution & Rename

* Finish the path resolution process in a directory tree.

* Implement the most complex metadata operation, `rename`.

You are suggested to only modify or add code in functions marked as `TODO(lab2.1)` and `TODO(lab2.2)` in `fuse_ops.cpp`. No need to consider permission checking for metadata.

#### Goals

Pass the tests for lab 2 (`./test.sh 2`) and answer the questions below.

The correct output is as follows.
```bash
$ ./test.sh 2
(Some other outputs)
Running test tests/lab0.py...
Number of files: 100
...
Delete files.
Running test tests/lab1.py...
Block size by stat: 4096B
...
Delete files.
Running test tests/lab2_1_ops.py...
Directory mkdir.
...
File unlink.
Running test tests/lab2_2_tree.py...
Create a directory tree.
...
Remove the directory tree.
All tests passed.
```

#### Hints

Lab 2.1

* For `mkdir` and `rmdir`, please refer to the code of `create` and `unlink`.

* For `chmod`, please refer to the code of `utimens`.

* Extra consideration:
    * Removing a non-empty directory should fail and return -ENOTEMPTY, which can be implemented by `matchPrefix` in the KV store.

    * [chmod](https://pubs.opengroup.org/onlinepubs/9699919799/functions/chmod.html) should modify the timestamp of the current metadata.

* No need to consider the parent data modification of `mkdir` and `rmdir`, no need to store `.` and `..` directory entries.

Lab 2.2

* Path resolution: Separate the path level by level, use the ID and the name of the next metadata to splice the key of the next metadata, and get the ID of the next metadata in its value. Loop until reaching the target metadata.

* Rename: Its logic is easy, but you should consider its atomicity. Note that renaming a directory should not influence its child metadata.

#### Questions

Unlike ext2 which separates metadata into directory entries and inodes, KVFS does not separate metadata.

1. Compared to ext2, KVFS has easier logic for some metadata operations. In this experiment, which operations are included? Why?

2. Which timestamps does `chmod` modify? What's the meaning of them?

3. To index to block 0 of file /a/b, which KV operations should KVFS do (let the ID of / be 0, the ID of /a be 1, the ID of /b be 2)?

4. How does KVFS support `readdir`? What's the difference between it and ext2?


5. How do you support `rename`? If the system crashes during it, is there dangling metadata in the directory tree? Can the system restore to a state that `rename` does all or nothing?


### Lab 3: Distributed Data Management

After lab 1 and lab 2, we implemented a "usable" local file system. However, due to the capacity limit of local persistent media (SSD, etc.), the local file system is limited in both capacity and number of files. To break the limits, distributed file systems emerged, which usually follow the "client-server" model. The server is responsible for storing metadata and data; the client (such as the FUSE client) receives the user's file system operations and converts them into network communication with the server.

For the sake of simplicity, we only perform distributed storage on the data part. Metadata management remains unchanged.

To simulate distributed data storage, we start 4 data server instances locally, whose IP (i.e., 127.0.0.1) and port are in the config file of `./conf/data_servers`.

### Quick Start

The steps for running KVFS are a little different.

* Step 1: run `./build.sh` to get the binary.

* Step 2: run data server by `python3 data_server.py start`

* Step 3: run `./mount_debug.sh` to mount KVFS and test it.

Each data server is placed in an independent `tmux` terminal. 
```bash
$ tmux ls
DataServer0: 1 windows (created Fri Feb 23 14:10:49 2024)
DataServer1: 1 windows (created Fri Feb 23 14:10:49 2024)
DataServer2: 1 windows (created Fri Feb 23 14:10:49 2024)
DataServer3: 1 windows (created Fri Feb 23 14:10:49 2024)
```

Using `tmux a -t DataServer{data_server_id}` for debugging the data server.

* Step 4: after testing, run `python3 data_server.py stop` to stop the data server.

### Lab 3: Distributed Data Service

#### Code Analysis

We introduce data servers (in `data_server.cpp`). It supports single-block read or write by `obj_read` and `obj_write`. It listens to the port using [rpclib](https://github.com/rpclib/rpclib) to support remote calls (RPC) of these two functions.

The code for calling these two functions is in `data_client.h`. Its `obj_read` and `obj_write` functions send the remote call request and receive the return value. During the initialization of KVFS, we initiated a DataClient object and connected it to data servers (lazily).

#### Description

Complete the code for distributed `read` and `write`. The data path may be (take read as an example): `op_read` -> DataClient `obj_read` -> DataServer `obj_read` -> DataClient `obj_read` -> `op_read`.

#### Goals

Pass the tests for lab 2 (`./test.sh 2`) in distributed data management, and answer the questions below.

#### Hints

You are suggested to only modify or add code in functions marked as `TODO(lab3)`.

* `fuse_ops.cpp`: Use `obj_write` and `obj_read` to replace the block reading logic in `op_write` and `op_read`.

* `data_client.h`: Distribute data blocks according to their `Key`, and fill in `obj_read` function.

* `data_server.cpp`: Implement single-block read/write logic.

#### Questions

1. In a distributed file system, the latency of synchronization operations is related to both disk and network latency. Assume that the KVFS client is deployed in Beijing and the data server is deployed in the United States. The network latency between the two places is 200ms, the network bandwidth is 100MB/s, and the latency of the server to access the SSD is 50us. What's the **approximate** synchronized data transfer bandwidth for block size 4KB or 128KB? (Hint: the process can be divided into sending a request and transferring data, overhead by serialization can be ignored, single-threaded processing)

2. The default configuration of KVFS contains 4 data servers. Assuming it is full of data, it needs to be expanded by 1 data server. However, depending on your data block distribution policy, certain data blocks need to be migrated. What's the approximate migration ratio? (Hint: Assume there are so many blocks) Is there any way to make the amount of data that needs to be migrated only 30% of the total amount of data while ensuring uniformness? (Hint: Consistent hashing)

## Extended Lab

You should select one of the two labs to finish.

### E1: KV Storage Backend

KVFS is based on RocksDB, a local KV store. RocksDB is based on a file system. In this lab, we will build a KV store over a raw disk, namely, bypass the file system layer.

* Use a file to simulate raw disk: `open` with `O_DIRECT` flag, which only supports >=512B aligned read/write (the memory address should also be aligned, 4KB granularity is recommended).

#### Goals

* Implement the interfaces in `kvs.h`, and pass the tests in lab 2.

* Ensure concurrency consistency.

* Ensure crash consistency: After crash recovery, all the KV updates should do all or nothing.

* Other innovative thoughts or designs.

### E2: Paper Reproduction

Reproduce techniques in the papers. You could select one or more papers below, including:


* (SC'17 [LocoFS](https://storage.cs.tsinghua.edu.cn/papers/sc17-locofs.pdf) Section 3.3) Decouple file metadata to two parts, reducing the size of KV pair involved in file read/write.

* (FAST'22 [InfiniFS](https://www.usenix.org/system/files/fast22-lv.pdf) Section 3.3) Generate directory ID by recursive hashing, predict the directory ID during path resolution and parallelize the process.

* (ATC'23 [SingularFS](https://www.usenix.org/system/files/atc23-guo.pdf) Section 3.2) Learn and implement POSIX [mkdir](https://pubs.opengroup.org/onlinepubs/9699919799/functions/mkdir.html) and [create](https://pubs.opengroup.org/onlinepubs/9699919799/functions/creat.html) (note that they involve the parent metadata), use ordered updates for their atomicity.

## Misc

1. Fix `Transport endpoint is not connected`.
    * run `./umount.sh` in the root of the repository.

2. Use gdb for debugging: refer to `mount_debug.sh`.