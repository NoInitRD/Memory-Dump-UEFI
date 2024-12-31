#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#include <ctype.h>


#define CHUNK_SIZE (4LL * 1024 * 1024 * 1024) //read the file in 4GB chunks


/*
 * this function searches for a hex pattern in the dump using the path and
 * the pattern as inputs
 */
void search_pattern(const char *memory_dump_path, const char *pattern_hex)
{
    if (pattern_hex == NULL || strlen(pattern_hex) == 0)
    {
        fprintf(stderr, "Error: Empty pattern provided for search.\n");
        exit(1);
    }

    unsigned char *pattern = NULL;
    size_t pattern_length = 0;

    if (pattern_hex[0] == '0' && pattern_hex[1] == 'x')
    {
        pattern_hex += 2;
        pattern_length = strlen(pattern_hex) / 2;
        if (strlen(pattern_hex) % 2 != 0)
        {
            fprintf(stderr, "Error: Invalid pattern length. Ensure the pattern is a valid hex string.\n");
            exit(1);
        }

        for (size_t i = 0; i < strlen(pattern_hex); i++)
        {
            if (!isxdigit((unsigned char)pattern_hex[i]))
            {
                fprintf(stderr, "Error: Invalid character '%c' in hex pattern.\n", pattern_hex[i]);
                exit(1);
            }
        }

        pattern = malloc(pattern_length);
        if (pattern == NULL)
        {
            perror("Memory allocation failed for pattern");
            exit(1);
        }

        for (size_t i = 0; i < pattern_length; i++)
        {
            if (sscanf(pattern_hex + 2 * i, "%2hhx", &pattern[i]) != 1)
            {
                fprintf(stderr, "Error: Invalid hex format in the pattern.\n");
                free(pattern);
                exit(1);
            }
        }
    }
    else
    {
        pattern_length = strlen(pattern_hex);
        pattern = malloc(pattern_length);
        if (pattern == NULL)
        {
            perror("Memory allocation failed for pattern");
            exit(1);
        }

        memcpy(pattern, pattern_hex, pattern_length);
    }

    FILE *file = fopen(memory_dump_path, "rb");
    if (file == NULL)
    {
        perror("Error opening file");
        free(pattern);
        exit(1);
    }

    unsigned char *buffer = malloc(CHUNK_SIZE);
    if (buffer == NULL)
    {
        perror("Failed to allocate buffer");
        free(pattern);
        fclose(file);
        exit(1);
    }

    uint64_t offset = 0;
    while (1)
    {
        size_t read_size = fread(buffer, 1, CHUNK_SIZE, file);
        if (read_size == 0 && !feof(file))
        {
            perror("Error reading file");
            break;
        }

        for (size_t i = 0; i <= read_size - pattern_length; i++)
        {
            if (memcmp(&buffer[i], pattern, pattern_length) == 0)
            {
                printf("Pattern found at offset: 0x%" PRIx64 "\n", offset + i);
            }
        }

        if (read_size < CHUNK_SIZE)
        {
            if (ferror(file))
            {
                perror("Error reading file");
                break;
            }
        }

        offset += read_size;
        if (feof(file))
            break;
    }

    free(pattern);
    free(buffer);
    fclose(file);
}


/*
 * this function searches for the nth non-zero byte in the memory dump using the
 * dump path and the number of non-zero bytes to skip
 */
void search_nth_nonzero(const char *memory_dump_path, const char *nth_value_str)
{
    if (nth_value_str == NULL)
    {
        fprintf(stderr, "Error: Missing value for nth non-zero byte.\n");
        exit(1);
    }

    char *endptr;
    long long nth_nonzero = strtoll(nth_value_str, &endptr, 10);

    if (*endptr != '\0' || endptr == nth_value_str)
    {
        fprintf(stderr, "Error: Invalid input. Nth non-zero byte must be a valid integer.\n");
        exit(1);
    }

    if (nth_nonzero <= 0)
    {
        fprintf(stderr, "Error: Nth non-zero byte must be a positive integer.\n");
        exit(1);
    }

    size_t nth_nonzero_size = (size_t)nth_nonzero;

    FILE *file = fopen(memory_dump_path, "rb");
    if (file == NULL)
    {
        perror("Error opening file");
        exit(1);
    }

    uint64_t offset = 0;
    size_t count = 0;

    unsigned char *buffer = malloc(CHUNK_SIZE);
    if (buffer == NULL)
    {
        perror("Failed to allocate buffer");
        fclose(file);
        exit(1);
    }

    while (1)
    {
        size_t read_size = fread(buffer, 1, CHUNK_SIZE, file);
        if (read_size == 0 && !feof(file))
        {
            perror("Error reading file");
            break;
        }

        for (size_t i = 0; i < read_size; i++)
        {
            if (buffer[i] != 0)
            {
                count++;
                if (count == nth_nonzero_size)
                {
                    printf("Occurrence %zu of non-zero byte found at offset: 0x%" PRIx64 "\n", nth_nonzero_size, offset + i);
                    free(buffer);
                    fclose(file);
                    return;
                }
            }
        }

        if (read_size < CHUNK_SIZE && ferror(file))
        {
            perror("Error reading file");
            break;
        }

        offset += read_size;
        if (feof(file))
            break;
    }

    printf("Error: %zu-th non-zero byte not found.\n", nth_nonzero_size);
    free(buffer);
    fclose(file);
}


/*
 * this function takes the memory dump path, argument type, and value,
 * checks the argument type, and calls the appropriate search function
 */
void search_memory_dump(const char *memory_dump_path, const char *arg_type, const char *arg_value)
{
    if (strcmp(arg_type, "-s") == 0)
    {
        search_pattern(memory_dump_path, arg_value);
    }
    else if (strcmp(arg_type, "-n") == 0)
    {
        search_nth_nonzero(memory_dump_path, arg_value);
    }
    else
    {
        fprintf(stderr, "Error: Invalid argument type '%s'. Use -s for pattern search or -n for nth non-zero byte search.\n", arg_type);
        exit(1);
    }
}


int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        fprintf(stderr, "Usage: %s <memory_dump_path> [-s <pattern>] | [-n <nth_nonzero>]\n", argv[0]);
        return 1;
    }

    const char *memory_dump_path = argv[1];
    const char *arg_type = argv[2];
    const char *arg_value = argv[3];

    if (strcmp(arg_type, "-s") == 0 || strcmp(arg_type, "-n") == 0)
    {
        search_memory_dump(memory_dump_path, arg_type, arg_value);
    }
    else
    {
        fprintf(stderr, "Invalid argument type '%s'. Use -s for pattern search or -n for nth non-zero byte search.\n", arg_type);
        return 1;
    }

    return 0;
}
