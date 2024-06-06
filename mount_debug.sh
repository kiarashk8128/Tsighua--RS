set -v
fusermount -u ./fs
rm -rf /tmp/fskv
build/bin/main --kvs_path=/tmp/fskv -d ./fs 
# gdb --args build/bin/main --kvs_path=/tmp/fskv -d ./fs 