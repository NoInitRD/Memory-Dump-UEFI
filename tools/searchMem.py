import sys


#read in 4gb chunks
CHUNK_SIZE = 4 * 1024 * 1024 * 1024


def search_memory_dump(memory_dump_path, hex_pattern):
    try:
        pattern = bytes.fromhex(hex_pattern)
    except ValueError:
        print("Error: Invalid hex pattern.")
        sys.exit(1)

    try:
        with open(memory_dump_path, "rb") as file:
            offset = 0

            while True:
                chunk = file.read(CHUNK_SIZE)
                
                if not chunk:
                    break

                chunk_offset = chunk.find(pattern)
                while chunk_offset != -1:
                    print(f"Pattern found at offset: {offset + chunk_offset:#x}")
                    chunk_offset = chunk.find(pattern, chunk_offset + 1)

                offset += len(chunk)

        print(f"Search completed for file {memory_dump_path}.")

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
