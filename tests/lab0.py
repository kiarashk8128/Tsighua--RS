import os
import random

path = "./fs"
file_num = 100

print("Number of files:", file_num)

# clear the directory.
for entry in os.listdir(path):
    os.remove(os.path.join(path, entry))

if (len(os.listdir(path)) != 0):
    print("Failed to clear the directory.")
    exit(1)

file_list = [{
    "name": os.path.join(path, f"file{i}.txt"), 
    "content": str(i % 10) * random.randint(10, 20)
} for i in range(file_num)]

print("Create files & write their first block.")
for file in file_list:
    with open(file["name"], "w") as f:
        write_len = f.write(file["content"])
        if (write_len != len(file["content"])):
            print(f"Failed to write to file {file['name']}.")
            exit(1)

print("List the root directory.")
entries = os.listdir(path)
if (len(entries) != file_num):
    print("Directory size is not correct.")
    print("Expected:", file_num)
    print("Got:", entries, "with size", len(entries))
    exit(1)

for file in file_list:
    if (file["name"].split("/")[-1] not in entries):
        print(f"File {file['name']} not found in the directory.")
        print("Got:", entries, "with size", len(entries))
        exit(1)

print("Read file.")
for file in file_list:
    with open(file["name"], "r") as f:
        content = f.read()
        if (content != file["content"]):
            print(f"File content of {file['name']} is not correct.")
            print("Expected:", file["content"])
            print("Got:", content)
            exit(1)

print("Overwrite in the middle.")
for file in file_list:
    file["content"] = file["content"][0] + "a" * (len(file["content"]) - 2) + file["content"][-1]
    with open(file["name"], "r+") as f:
        f.seek(1)
        write_len = f.write(file["content"][1:-1])
        if (write_len != len(file["content"][1:-1])):
            print(f"Failed to overwrite file {file['name']}.")
            exit(1)

print("Unaligned read.")
for file in file_list:
    with open(file["name"], "r") as f:
        f.seek(1)
        content = f.read(len(file["content"]) - 2)
        if (content != file["content"][1:-1]):
            print(f"Partial file content of {file['name']} is not correct.")
            print("Expected:", file["content"])
            print("Got:", content)
            exit(1)

print("Verify file content.")
for file in file_list:
    with open(file["name"], "r") as f:
        content = f.read()
        if (content != file["content"]):
            print(f"File content of {file['name']} is not correct.")
            print("Expected:", file["content"])
            print("Got:", content)
            exit(1)

print("Delete files.")
for file in file_list:
    os.remove(file["name"])

if (len(os.listdir(path)) != 0):
    print("Failed to delete the files.")
    exit(1)
