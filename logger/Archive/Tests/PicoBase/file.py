import ujson
import os


def save_file(filename, data):
    tmp = filename + ".tmp"
    with open(tmp, "w") as f:
        ujson.dump(data, f)
    os.rename(tmp, filename)

def load_file(filename):
    data=None
    with open(filename, "r") as f:
        data = ujson.load(f)
    return data

file_data = load_file("status.json")
print(file_data)

file_data["days"] = 2
save_file("status.json", file_data)

file_data = load_file("status.json")
print(file_data)