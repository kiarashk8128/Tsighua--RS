set -v
fusermount -u ./fs
build/bin/main --kvs_path=/tmp/fskv ./fs