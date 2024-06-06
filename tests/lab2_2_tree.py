import os
import shutil

path = "./fs"
os.chdir(path)

print("Create a directory tree.")
os.makedirs("A/B/C")
os.makedirs("A/D")
os.makedirs("A/F/G")
content = "Hello, World!"
with open("A/1.txt", "w") as f:
    f.write(content)
with open("A/2.txt", "w") as f:
    f.write(content)
with open("A/B/3.txt", "w") as f:
    f.write(content)

print("Remove a directory that is not empty.")
try:
    os.rmdir("A")
except:
    # OSError is expected.
    print("It fails as expected.")
    pass
if "A" not in os.listdir("."):
    print("It should fail.")
    exit(1)

print("Rename a file.")
os.rename("A/1.txt", "A/4.txt")
if "1.txt" in os.listdir("A") or "4.txt" not in os.listdir("A"):
    print("Failed to rename the file.")
    exit(1)

print("Verify the content of the file after rename.")
with open("A/4.txt", "r") as f:
    if f.read() != content:
        print("File content changed after rename.")
        exit(1)

print("Rename a directory.")
os.rename("A/B", "A/E")

if "B" in os.listdir("A") or "E" not in os.listdir("A"):
    print("Failed to rename the directory.")
    exit(1)

dir_in_E = os.listdir("A/E")
if len(dir_in_E) != 2 or "C" not in dir_in_E or "3.txt" not in dir_in_E:
    print("File loss in directory A/E, got:", dir_in_E)
    exit(1)

print("Rename and replace an empty directory.")
os.rename("A/E", "A/D")

if "E" in os.listdir("A") or "D" not in os.listdir("A"):
    print("Failed to rename A/E to replace A/D.")
    exit(1)

print("Verify the content of the sub-file after rename.")
with open("A/D/3.txt", "r") as f:
    if f.read() != content:
        print("File content changed after rename.")
        exit(1)

print("Rename should fail if the target directory is not empty.")
try:
    os.rename("A/D", "A/F")
except:
    # OSError is expected.
    print("It fails as expected.")
    pass

if "D" not in os.listdir("A") or "F" not in os.listdir("A"):
    print("Rename should fail if the target directory is not empty.")
    exit(1)

print("Remove the directory tree.")
shutil.rmtree("A")
if "A" in os.listdir("."):
    print("Failed to remove the directory tree.")
    exit(1)