#ifndef FUSE_OPS_H_
#define FUSE_OPS_H_

#include <fuse3/fuse.h>

#include <string>

void op_init(const std::string &kvs_path);

int op_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, fuse_file_info *fi,
               fuse_readdir_flags flags);

int op_getattr(const char *path, struct stat *stbuf, fuse_file_info *fi);

int op_create(const char *path, mode_t mode, fuse_file_info *fi);

int op_unlink(const char *path);

int op_utimens(const char *path, const struct timespec tv[2], struct fuse_file_info *fi);

int op_open(const char *path, fuse_file_info *fi);

int op_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi);

int op_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi);

int op_truncate(const char *path, off_t size, struct fuse_file_info *fi);

int op_fsync(const char *path, int datasync, struct fuse_file_info *fi);

int op_close(const char *path, fuse_file_info *fi);

int op_mkdir(const char *path, mode_t mode);

int op_rmdir(const char *path);

int op_chmod(const char *path, mode_t mode, struct fuse_file_info *fi);

int op_rename(const char *from, const char *to, unsigned int flags);

#endif  // FUSE_OPS_H_