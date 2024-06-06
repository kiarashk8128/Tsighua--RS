import os
import random

n_blocks = 500 # Use a smaller n_blocks for debugging.
n_random_tests = 10
path = "./fs"
file = {
    "name": os.path.join(path, "huge_file.txt"),
    "content": ""
}

open(file["name"], "w").close() # clear the file.
block_size = os.stat(file["name"]).st_blksize
print(f"Block size by stat: {block_size}B")

file_size = block_size * n_blocks
print(f"File size: {file_size // 1024}KB")
file["content"] = "abcd" * (file_size // 4)

# Initialize the file.
with open(file["name"], "w") as f:
    write_len = f.write(file["content"])
    if (write_len != len(file["content"])):
        print(f"Failed to write to file {file['name']}.")
        exit(1)

test_num = 1
def write_and_read(offset, length, content_rep, description):
    global test_num
    content = (content_rep * length)[0:length]
    assert(len(content) == length)
    print(f"Write case {test_num}: {description}.")
    with open(file["name"], "r+") as f:
        f.seek(offset)
        file["content"] = file["content"][:offset] + content + file["content"][offset + length:]
        real_write_len = f.write(content)
        if (real_write_len != length):
            print(f"Failed to write to file {file['name']}.")
            exit(1)
    
    # verify the file.
    with open(file["name"], "r") as f:
        file_content = f.read()
        if (file_content != file["content"]):
            print(f"File content of {file['name']} is not correct.")
            print("Expected:", file["content"])
            print("Got:", file_content)
            exit(1)

    print(f"Read case {test_num}: {description}.")
    with open(file["name"], "r") as f:
        f.seek(offset)
        file_content = f.read(length)
        if (file_content != content):
            print(f"File content of {file['name']} is not correct.")
            print("Expected:", content)
            print("Got:", file_content)
            exit(1)
    test_num += 1

write_and_read(2 * block_size, 2 * block_size, "e", "aligned")
write_and_read(1, block_size - 2, "f", "unaligned in a block")
write_and_read(block_size, 2 * block_size - 1, "g", "start aligned, end unaligned")
write_and_read(block_size + 1, 2 * block_size - 1, "h", "start unaligned, end aligned")
write_and_read(1, file_size - 2, "i", "unaligned across blocks")

random.seed(42)
for i in range(n_random_tests):
    pos1 = random.randint(0, file_size - 1)
    pos2 = random.randint(0, file_size - 1)
    write_and_read(min(pos1, pos2), max(pos1, pos2), chr(ord('a') + pos1 % 26), f"random range [{min(pos1, pos2)}, {max(pos1, pos2)})")

print("Delete files.")
os.remove(file["name"])

if (len(os.listdir(path)) != 0):
    print("Failed to delete the files.")
    exit(1)
