import os
import random
import time

path = "./fs"

test_dirname = "dir0"
test_filename = "file0"
test_dir = os.path.join(path, test_dirname)
test_file = os.path.join(path, test_filename)

def chmod_test(target):
    st1 = os.stat(target)
    time.sleep(1)
    new_mode = 0o333
    os.chmod(target, new_mode)
    st2 = os.stat(target)

    mode_bit = st2.st_mode & 0o777
    if mode_bit != new_mode:
        print(f"Failed to change the mode of the directory. Expected: {new_mode}, Got: {mode_bit}")
        exit(1)

    st1_other_bit = st1.st_mode & (~0o777)
    st2_other_bit = st2.st_mode & (~0o777)
    if st2_other_bit != st1_other_bit:
        print("Chmod should not change the other bits of the directory.")
        exit(1)
    
    if st2.st_ctime <= st1.st_ctime:
        print("Chmod should update the ctime of the directory.")
        exit(1)
    os.chmod(target, st1.st_mode)

print("Directory mkdir.")
os.mkdir(test_dir)

if test_dirname not in os.listdir(path):
    print("Failed to create the directory.")
    exit(1)

print("Directory chmod.")
chmod_test(test_dir)

print("Directory rmdir.")
os.rmdir(test_dir)

if test_dirname in os.listdir(path):
    print("Failed to create the directory.")
    exit(1)

print("File open.")
open(test_file, "w").close()

if test_filename not in os.listdir(path):
    print("Failed to create the file.")
    exit(1)

print("File chmod.")
chmod_test(test_file)

print("File unlink.")
os.unlink(test_file)

if test_filename in os.listdir(path):
    print("Failed to delete the file.")
    exit(1)
