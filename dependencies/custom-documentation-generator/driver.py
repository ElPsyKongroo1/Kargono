import traceback
import sys
import glob
import os
import shutil

import util_functions

def create_output():

    folder_name = "Engine"
    source_dir = "./../../Engine"
    ignore_directories = ['dependencies*']
    working_dir = "./directories/" + folder_name + "-working-dir"
    cache_dir = "./directories/" + folder_name + "-cache-dir"
    output_dir = "./directories/" + folder_name + "-output-dir"

    # Move source code to initial working directory
    util_functions.push_directory(source_dir, working_dir, ignore_directories)

    # Add paths to all files in working directory
    working_files = glob.glob(working_dir + '/**/*.cpp', recursive=True) + glob.glob(working_dir + '/**/*.h', recursive=True)
    working_files = [os.path.normpath(i) for i in working_files]

    # Check if code has been modified
    temp_modified_files = []
    for i in working_files:
        if not util_functions.compare_hash(i, i.replace("-working-dir", "-cache-dir", 1)):
            temp_modified_files.append(i)

    # Add modified files to modified files list and apply limit to number of files to be changed
    modified_files = []
    for file_path in temp_modified_files:
        modified_files.append(file_path)

    # for index in range(MAX_NUM_UPLOAD):
    #     if index > (len(temp_modified_files) - 1):
    #         break
    #     modified_files.append(temp_modified_files[index])

    # Output if no files need to be modified
    if len(modified_files) == 0:
        print("Files had not changes")
        return

    # Copy files into output_dir
    for i in modified_files:
        util_functions.push_file(i, i.replace("-working-dir", "-output-dir", 1))

    # Change modified_files paths to point to output_dir
    modified_files = [i.replace("-working-dir", "-output-dir", 1) for i in modified_files]
    print(modified_files)

    # Add comments to output documents
    l = 0
    for modified_file in modified_files:
        # if (l >= MAX_NUM_UPLOAD):
        #     break;
        print("Working on file number " + str(l + 1) + " out of " + str(len(modified_files)))
        with open(modified_file, 'r') as file:
            data_list = file.readlines()
            data = ''.join(data_list)
            comment_list = util_functions.produce_comments(data)
            print(comment_list)
        with open(modified_file, "r") as f:
            file_contents = f.readlines()
        for j in comment_list:
            key = list(j.keys())[0]
            value = list(j.values())[0]
            index = 0
            for file_line in file_contents:
                if key in file_line:
                    file_contents.insert(index, value + '\n')
                    break
                index += 1
        file_contents = "".join(file_contents)
        with open(modified_file, "w") as f:
            f.write(file_contents)
        l += 1
        util_functions.push_file(modified_file.replace("-output-dir", "-working-dir", 1),
                                 modified_file.replace("-output-dir", "-cache-dir", 1))



    #     with open(i, 'r') as file:
    #         data = file.read()
    #         print(data)

    # Final copy of working_dir to cache_dir
    # for index in range(MAX_NUM_UPLOAD):
    #     util_functions.push_file(modified_files[index].replace("-output-dir", "-working-dir", 1), modified_files[index].replace("-output-dir", "-cache-dir", 1))

    util_functions.remove_directory(working_dir)

    # Output Successfull Run of Code!
    if os.path.exists('error_log.txt'):
        os.remove('error_log.txt')
    with open('error_log.txt', 'a') as f:
        f.write("No errors for recent run!")

try:
    create_output()
except Exception as e:
    if os.path.exists('error_log.txt'):
        os.remove('error_log.txt')
    with open('error_log.txt', 'a') as f:
        f.write(str(e))
        f.write(traceback.format_exc())
        sys.exit(1)

