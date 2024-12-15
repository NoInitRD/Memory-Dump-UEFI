import os
import sys


def main():
    if len(sys.argv) < 2:
        print("Usage: python concatDumps.py <dump1> <dump2>...")
        sys.exit(1)

    name = sys.argv[1]  # Corrected sys.args to sys.argv
    files = sorted([f for f in os.listdir() if f.endswith('.bin')], key=lambda x: int(x[4:-4]))
    print(files)

    with open(name, 'wb') as output_file:
        for file in files:
            with open(file, 'rb') as input_file:
                output_file.write(input_file.read())

    print(f"Files have been concatenated into '{name}'")


if __name__ == "__main__":
    main()
