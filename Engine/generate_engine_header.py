import os

def write_h_files(directory, output_file, exclude_keywords):
    with open(output_file, "w") as file:
        if exclude_keywords is None:
            exclude_keywords = []
        file.write("#pragma once\n\n")
        file.write("// Header generated automatically by generate_engine_header.py\n")
        file.write("// It contains only header files that export symbols using ENGINE_API\n")
        file.write("// It should be included only by external projects!\n\n")
        file.write("#include \"core\entry_point.h\"\n")
        for root, dirs, files in os.walk(directory):
            for file_name in files:
                if file_name.endswith(".h"):
                    if file_name == output_file:
                        continue
                    full_file_path = os.path.join(root, file_name)
                    if any(keyword in full_file_path for keyword in exclude_keywords):
                        continue
                    try:
                        with open(full_file_path, "r", encoding="utf-8") as f:
                            file_contents = f.read()
                            if "ENGINE_API" not in file_contents:
                                continue
                    except Exception as e:
                        print(f"Error reading file {full_file_path}: {e}")
                        continue
                    relative_file_path = os.path.relpath(full_file_path, directory)
                    print(f'#include "{relative_file_path}"')
                    file.write(f'#include "{relative_file_path}"\n')

if __name__ == "__main__":
    write_h_files(os.getcwd(), "engine.h", ["third_party", "assimp_logger.h", "entry_point.h"])