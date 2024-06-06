#include <cstring>

#include "fuse_ops.h"
#include "log.h"

static struct options {
    const char *kvs_path;
} opt;

#define OPTION(t, p) \
    { t, offsetof(struct options, p), 1 }
static const struct fuse_opt option_spec[] = { OPTION("--kvs_path=%s", kvs_path), FUSE_OPT_END };

fuse_operations ops;

int main(int argc, char **argv) {
    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
    opt.kvs_path = strdup("/tmp/fskv");

    LOG(INFO) << "KVS path: " << opt.kvs_path;
    if (fuse_opt_parse(&args, &opt, option_spec, nullptr) == -1) {
        LOG(ERROR) << "Failed to parse options";
    }
    op_init(opt.kvs_path);

    ops.getattr = op_getattr;
    ops.readdir = op_readdir;
    ops.create = op_create;
    ops.unlink = op_unlink;
    ops.utimens = op_utimens;

    ops.open = op_open;
    ops.read = op_read;
    ops.write = op_write;
    ops.truncate = op_truncate;
    ops.fsync = op_fsync;
    ops.release = op_close;

    ops.mkdir = op_mkdir;
    ops.rmdir = op_rmdir;
    ops.chmod = op_chmod;
    ops.rename = op_rename;

    LOG(INFO) << "Entering main...";
    fuse_main(args.argc, args.argv, &ops, nullptr);
    LOG(INFO) << "Exited main...";
    return 0;
}