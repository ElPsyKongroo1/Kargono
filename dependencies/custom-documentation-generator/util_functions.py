# Python program to find the SHA-1 message digest of a file

# importing the hashlib module
import hashlib
import shutil
import os
import openai
import json
import sys
import openai_keys


def produce_comments(src_string):
    openai.api_key = openai_keys.openai_key1
    is_output_ready = False
    while not is_output_ready:

        initialOutput = openai.ChatCompletion.create(
            model="gpt-3.5-turbo",
            messages=[
                {"role": "system", "content": "The format of your response should be a python list"
                                              " with key-value pair dictionary elements. Do not add any additional text!"
                                              "The input will be a c++ file. The key/value pairs have the few first words of the line being referenced as the key and the comment as the value."
                                              "The response should be in the following format:"
                                              '[{"void Carry(int MUX) {:" "/// @brief Architecture definition of the MUX"}, {"void Carry(int MUX) {:": "/// @details More details about this mux element."}]'
                                              "Also strip all new line characters from your output."
                                              "Only one comment for each line!"},
                {"role": "system", "content": 'Here is an example of a decent response: '
                                              '[{"enum class EventType": "/// @brief Enumeration defining different event types in the game engine"}, {"enum EventCategory": "/// @brief Enumeration defining different event categories in the game engine"}, {"#define EVENT_CLASS_TYPE(type)": "/// @brief Macro for defining the event class category and its members"},{"class KG_API Event": "/// @class Event", "31": "/// @brief Base class for all events in the game engine"},{"inline bool IsInCategory": "/// @return The category flags of the event"}]'
                                              'Responses should be longer!'},
                {"role": "user", "content": 'I am going to provide you a file from my game engine directory. Please provide'
                                            "comments that are compatible with the Doxygen API. Return me a list where"
                                            "the few first words of the line being referenced is the key, and the comment is the value."
                                            "Please add documentation like @param, @return, @source, and your interpretation of the objects in the source code."
                                            "This should include comments for functions, enumerations, classes, methods, parameters, and return values. "
                                            "To ensure the c++ files still compile, ensure you add /// before each comment."
                                            "Also note that each line from the input is separated by a newline character"
                                            "The next input will be the file itself."
                 },
                {"role": "user", "content": src_string}

            ]
        )
        while True:

            is_output_clean = openai.ChatCompletion.create(
                model="gpt-3.5-turbo",
                messages=[
                    {"role": "system", "content" : "Please check this string to ensure it would parse correctly in"
                                                   "the method json.loads() from Python library module json. "
                                                   "If there would be no issues. Return ONLY the letter T."
                                                   "Otherwise, return the letter F."
                                                   "Your response should only be one letter, either T or F."},
                    {"role": "system", "content": "Your response should only be one letter, either T or F."
                                                  "Do NOT add any more text. Extra text will break my code!"},
                    {"role": "user", "content": initialOutput.choices[0].message.content}

                ]
            )
            if is_output_clean.choices[0].message.content == "T":
                print("JSON format should be correct!")
                is_output_ready = True
                break
            if is_output_clean.choices[0].message.content == "F":
                print('Had to retry output. Sanitization says output is not correct.')
                break
            print("Sanitization step did not respond with the correct output. Trying Again. Here is the incorrect output:")
            print(is_output_clean.choices[0].message.content)

    return json.loads(initialOutput.choices[0].message.content)


def push_directory(src, dest, ignore_directories=''):
    # Remove any current directory and replace with new
    if not os.path.exists(src):
        raise Exception("Source Directory is invalid!")

    if not os.path.exists(dest):
        os.mkdir(dest)

    shutil.rmtree(dest)
    shutil.copytree(src, dest, ignore=shutil.ignore_patterns(*ignore_directories))


def remove_directory(src):
    # Remove any current directory and replace with new
    if os.path.exists(src):
        shutil.rmtree(src)


def push_file(src, dest):
    # Remove any current directory and replace with new
    if not os.path.exists(src):
        raise Exception("Source file is invalid!")

    if os.path.exists(dest):
        os.remove(dest)

    os.makedirs(os.path.dirname(dest), exist_ok=True)
    shutil.copyfile(src, dest)


def compare_hash(filename_1, filename_2):
    if not os.path.exists(filename_1):
        raise Exception("Input file does not exist. I don't know this would happen.")
    if not os.path.exists(filename_2):
        return False

    digest_1 = hash_file(filename_1)
    digest_2 = hash_file(filename_2)
    if digest_1 == digest_2:
        return True
    else:
        return False


def hash_file(filename):
    """"This function returns the SHA-1 hash
   of the file passed into it"""

    # make a hash object
    h = hashlib.sha1()

    # open file for reading in binary mode
    with open(filename, 'rb') as file:
        # loop till the end of the file
        chunk = 0
        while chunk != b'':
            # read only 1024 bytes at a time
            chunk = file.read(1024)
            h.update(chunk)

    # return the hex representation of digest
    return h.hexdigest()
