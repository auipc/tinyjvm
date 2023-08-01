#!/usr/bin/python3
import os

JAVA_COMPILER="javac"
JAVA_VM="./jvm"

for file in os.listdir("tests"):
    if not file.endswith(".java"):
        continue

    class_filename = file.replace(".java", ".class")
    class_name = class_filename.split(".class")[0]
    description_filename = f"{class_name}.description"

    java_file = os.path.join("tests", file)
    class_file = os.path.join("tests", class_filename)
    description_file = os.path.join("tests", description_filename)
    desc_file = open(description_file, "r")
    description = desc_file.read()
    desc_file.close()

    print(f"Running test {class_name} -- {description}")
    # purge existing file
    if os.path.isfile(class_file):
        os.remove(class_file)

    os.system(f"{JAVA_COMPILER} {java_file} -g:none")
    if not os.path.isfile(class_file):
        print(f"Failed to compile test {java_file}")
        continue

    exit_code = os.system(f"{JAVA_VM} {class_name} {class_file} >/dev/null 2>&1") >> 8
    if exit_code != 0:
        print(f"TEST {class_name} FAILED")
    else:
        print(f"TEST {class_name} SUCCESS")
