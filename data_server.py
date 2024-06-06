import os
import sys

config_file = "./conf/data_servers"

if not os.path.exists(config_file):
    print("Config file does not exist...")
    exit(1)

if sys.argv[1] == "start":
    os.system("tmux kill-server")
    with open(config_file, "r") as f:
        for id, line in enumerate(f):
            ip, port = line.strip().split(":")
            if ip != "127.0.0.1":
                print("Non-localhost IP address is not supported by this script...")
                exit(1)
            os.system(f"tmux new -d -s DataServer{id} 'build/bin/data_server {port} {id}'")
elif sys.argv[1] == "stop":
    os.system("tmux kill-server")
