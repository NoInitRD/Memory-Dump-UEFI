#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>


#define CHUNK_SIZE (4LL * 1024 * 1024 * 1024) //read the file in 4GB chunks


/*
 * this function extracts and returns the first integer it finds in the filename,
 * it treats the filename as a string and parses digits to form an integer value
 */
int extract_number(const char *filename)
{
    int number = 0;
    while (*filename)
    {
        if (isdigit((unsigned char)*filename))
            number = number * 10 + (*filename - '0');

        filename++;
    }
    return number;
}


/*
 * this function compares two filenames based on the integer numbers extracted
 * from the filenames, it is used for sorting an array of filenames
 */
int compare_files(const void *a, const void *b)
{
    const char *file_a = *(const char **)a;
    const char *file_b = *(const char **)b;

    int num_a = extract_number(file_a);
    int num_b = extract_number(file_b);

    return num_a - num_b;
}


/*
 * this function concatenates multiple files into a single output file,
 * it reads the input files in chunks and writes them to the output file sequentially
 */
void concat_files(const char *output_file_name, char **files, int file_count)
{
    FILE *output_file = fopen(output_file_name, "wb");
    if (output_file == NULL)
    {
        perror("Error opening output file");
        exit(1);
    }

    unsigned char *buffer = malloc(CHUNK_SIZE);
    if (buffer == NULL)
    {
        perror("Memory allocation failed for buffer");
        fclose(output_file);
        exit(1);
    }

    for (int i = 0; i < file_count; i++)
    {
        printf("Adding %s\n", files[i]);

        FILE *input_file = fopen(files[i], "rb");
        if (input_file == NULL)
        {
            perror("Error opening input file");
            free(buffer);
            fclose(output_file);
            exit(1);
        }

        size_t bytes_read;
        while ((bytes_read = fread(buffer, 1, CHUNK_SIZE, input_file)) > 0)
            fwrite(buffer, 1, bytes_read, output_file);

        fclose(input_file);
        free(files[i]);
    }

    free(buffer);
    fclose(output_file);
    printf("Files have been concatenated into '%s'\n", output_file_name);
}


int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        fprintf(stderr, "Usage: %s <output_file_name> <input_files...>\n", argv[0]);
        return 1;
    }

    const char *output_file_name = argv[1];

    char **files = malloc((argc - 2) * sizeof(char*));
    if (files == NULL)
    {
        perror("Memory allocation failed for file names");
        return 1;
    }

    int file_count = 0;
    for (int i = 2; i < argc; i++)
    {
        files[file_count] = strdup(argv[i]);
        file_count++;
    }

    qsort(files, file_count, sizeof(char *), compare_files);
    concat_files(output_file_name, files, file_count);
    free(files);

    return 0;
}
