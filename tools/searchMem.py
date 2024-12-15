import sys


def search_memory_dump(memory_dump_path, hex_pattern):
    try:
        pattern = bytes.fromhex(hex_pattern)
    except ValueError:
        print("Error: Invalid hex pattern.")
        sys.exit(1)

    
    try:
        with open(memory_dump_path, "rb") as file:
            dump_data = file.read()

            offset = 0
            while True:
                offset = dump_data.find(pattern, offset)
                if offset == -1:
                    break
                print(f"Pattern found at offset: {offset:#x}")
                offset += 1
        print(f"Error: File {memory_dump_path} not found.")
        sys.exit(1)
    except IOError as e:
        print(f"Error: {e}")
        sys.exit(1)


def main():
    if len(sys.argv) != 3:
        print("Usage: python search_memory_dump.py <memory_dump_path> <hex_pattern>")
        sys.exit(1)

    memory_dump_path = sys.argv[1]
    hex_pattern = sys.argv[2]

    if len(hex_pattern) % 2 != 0:
        print("Error: Hex pattern must have an even number of characters.")
        sys.exit(1)

    search_memory_dump(memory_dump_path, hex_pattern)


if __name__ == "__main__":
    main()
