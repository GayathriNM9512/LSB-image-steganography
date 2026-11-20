#include <stdio.h>
#include <string.h>
#include "decode.h"
#include "types.h"
#include "common.h"

Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    char *dot = strrchr(argv[2], '.');

    if (dot != NULL && strcmp(dot, ".bmp") == 0)
    {
        decInfo->stego_image_fname = argv[2];
    }
    else
    {
        printf("ERROR: Stego image must be a .bmp file\n");
        return e_failure;
    }

    if (argv[3] != NULL)
    {
        strcpy(decInfo->output_fname, argv[3]);

        char *dot2 = strrchr(decInfo->output_fname, '.');
        if (dot2 != NULL)
        {
            *dot2 = '\0';
        }
    }
    else
    {
        strcpy(decInfo->output_fname, "decoded");
    }

    return e_success;
}
Status open_decode_files(DecodeInfo *decInfo)
{
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "r");

    if (decInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->stego_image_fname);
        return e_failure;
    }

    return e_success;
}
char decode_byte_from_lsb(char *arr)
{
    char ch = 0;

    for (int i = 0; i < 8; i++)
    {
        ch = ch | ((arr[i] & 1) << i);
    }

    return ch;
}

int decode_int_from_image(char *arr)
{
    int size = 0;

    for (int i = 0; i < 32; i++)
    {
        size = size | ((arr[i] & 1) << i);
    }

    return size;
}
void decode_string_from_image(char *str, int size, DecodeInfo *decInfo)
{
    char arr[8];

    for (int i = 0; i < size; i++)
    {
        fread(arr, 8, 1, decInfo->fptr_stego_image);
        str[i] = decode_byte_from_lsb(arr);
    }

    str[size] = '\0';
}
Status decode_magic_string(DecodeInfo *decInfo)
{
    char magic[3];

    decode_string_from_image(magic, strlen(MAGIC_STRING), decInfo);

    if (strcmp(magic, MAGIC_STRING) == 0)
    {
        return e_success;
    }
    else
    {
        return e_failure;
    }
}
Status decode_size_extn(DecodeInfo *decInfo)
{
    char arr[32];

    fread(arr, 32, 1, decInfo->fptr_stego_image);
    decInfo->extn_size = decode_int_from_image(arr);

    if (decInfo->extn_size <= 0 || decInfo->extn_size >= 10)
    {
        return e_failure;
    }

    return e_success;
}

Status decode_secret_file_extn(DecodeInfo *decInfo)
{
    decode_string_from_image(decInfo->extn_output_file, decInfo->extn_size, decInfo);

    strcat(decInfo->output_fname, decInfo->extn_output_file);

    decInfo->fptr_output = fopen(decInfo->output_fname, "w");

    if (decInfo->fptr_output == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to create file %s\n", decInfo->output_fname);
        return e_failure;
    }

    return e_success;
}
Status decode_secret_file_size(DecodeInfo *decInfo)
{
    char arr[32];

    fread(arr, 32, 1, decInfo->fptr_stego_image);
    decInfo->size_output_file = decode_int_from_image(arr);

    return e_success;
}

Status decode_secret_file_data(DecodeInfo *decInfo)
{
    char arr[8];
    char ch;

    for (long i = 0; i < decInfo->size_output_file; i++)
    {
        fread(arr, 8, 1, decInfo->fptr_stego_image);
        ch = decode_byte_from_lsb(arr);
        fwrite(&ch, 1, 1, decInfo->fptr_output);
    }

    return e_success;
}
Status do_decoding(DecodeInfo *decInfo)
{
    if (open_decode_files(decInfo) == e_success)
    {
        printf("INFO: Open files is completed\n");
    }
    else
    {
        return e_failure;
    }

    fseek(decInfo->fptr_stego_image, 54, SEEK_SET);

    if (decode_magic_string(decInfo) == e_success)
    {
        printf("INFO: Magic string matched\n");
    }
    else
    {
        printf("ERROR: Magic string not found. This image has no hidden data\n");
        fclose(decInfo->fptr_stego_image);
        return e_failure;
    }

    if (decode_size_extn(decInfo) == e_success)
    {
        printf("INFO: Extension size decoded: %d\n", decInfo->extn_size);
    }
    else
    {
        printf("ERROR: Invalid extension size\n");
        fclose(decInfo->fptr_stego_image);
        return e_failure;
    }

    if (decode_secret_file_extn(decInfo) == e_success)
    {
        printf("INFO: Extension decoded: %s, output file: %s\n", decInfo->extn_output_file, decInfo->output_fname);
    }
    else
    {
        fclose(decInfo->fptr_stego_image);
        return e_failure;
    }

    decode_secret_file_size(decInfo);
    printf("INFO: Secret file size decoded: %ld\n", decInfo->size_output_file);

    decode_secret_file_data(decInfo);
    printf("INFO: Secret data decoded\n");

    fclose(decInfo->fptr_stego_image);
    fclose(decInfo->fptr_output);

    return e_success;
}