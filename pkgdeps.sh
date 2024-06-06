if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    # judge if it is ubuntu.
    if [ -f /etc/lsb-release ]; then
        . /etc/lsb-release
        if [ "$DISTRIB_ID" == "Ubuntu" ]; then
            sudo apt install -y fuse3 libfuse3-dev librocksdb-dev
            exit 0
        fi
        echo "Unsupported Linux distribution: $DISTRIB_ID"
    fi
else
    echo "Unsupported OS: $OSTYPE"
fi
