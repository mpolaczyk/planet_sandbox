import os

def write_h_files(directory, output_file, exclude_keywords):
    with open(output_file, "w") as file:
        if exclude_keywords is None:
            exclude_keywords = []
        file.write("#pragma once\n\n")
        file.write("// Header generated automatically, don't modify!\n")
        file.write("// It should be included only by external projects!\n\n")
        for root, dirs, files in os.walk(directory):
            for file_name in files:
                if file_name.endswith(".h"):
                    if file_name == output_file:
                        continue
                    file_path = os.path.join(root, file_name)
                    if any(keyword in file_path for keyword in exclude_keywords):
                        continue
                    relative_file_path = os.path.relpath(file_path, directory)
                    print(f'#include "{relative_file_path}"')
                    file.write(f'#include "{relative_file_path}"\n')

if __name__ == "__main__":
    write_h_files(os.getcwd(), "engine.h", ["third_party", "assimp_logger.h"])