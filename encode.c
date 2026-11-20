#include <stdio.h>
#include <string.h>     
#include "encode.h"
#include "types.h"
#include "common.h"     

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}

/* 
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

    	return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

    	return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

    	return e_failure;
    }

    // No failure return e_success
    return e_success;
}

Status check_capacity(EncodeInfo *encInfo)
{
    int size_src_image = get_image_size_for_bmp(encInfo->fptr_src_image);

    //Move the secret_fptr to the last
    fseek(encInfo->fptr_secret, 0, SEEK_END);

    encInfo->size_secret_file = ftell(encInfo->fptr_secret);

    int data_size = strlen(MAGIC_STRING) + sizeof(int) + strlen(encInfo->extn_secret_file) + sizeof(int) + encInfo->size_secret_file;

    if(data_size*8 < size_src_image)
    {
        return e_success;
    }
    else
    {
        return e_failure;
    }
}


Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    char *dot = strrchr(argv[2], '.');

    if (dot != NULL && dot != argv[2] && dot[1] != '\0')
    {
        encInfo->secret_fname = argv[2];
        strcpy(encInfo->extn_secret_file, dot);
    }
    else
    {
        printf("ERROR: Secret file must have an extension (eg: secret.txt)\n");
        return e_failure;
    }
    char *dot2 = strrchr(argv[3], '.');

    if (dot2 != NULL && strcmp(dot2, ".bmp") == 0)
    {
        encInfo->src_image_fname = argv[3];
    }
    else
    {
        printf("ERROR: Source image must be a .bmp file\n");
        return e_failure;
    }
    if (argv[4] != NULL)
    {
        char *dot3 = strrchr(argv[4], '.');

        if (dot3 != NULL && strcmp(dot3, ".bmp") == 0)
        {
            encInfo->stego_image_fname = argv[4];
        }
        else
        {
            printf("ERROR: Output image must be a .bmp file\n");
            return e_failure;
        }
    }
    else
    {
        encInfo->stego_image_fname = "stego.bmp";
    }
    return e_success;
}

Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    rewind(fptr_src_image);
    char arr[54];
    fread(arr, 54, 1, fptr_src_image);
    fwrite(arr, 54, 1, fptr_dest_image);
    return e_success;
}

Status encode_magic_string(char *magic_string, EncodeInfo *encInfo)
{
    encode_string_to_image(magic_string,strlen(magic_string),encInfo);
    return e_success;
}

Status encode_size_extn(EncodeInfo *encInfo)
{
    char arr[32];
    fread(arr, 32, 1, encInfo->fptr_src_image);
    encode_int_to_image(strlen(encInfo->extn_secret_file), arr);
    fwrite(arr, 32, 1, encInfo->fptr_stego_image);
    return e_success;
}

Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    encode_string_to_image(file_extn, strlen(file_extn), encInfo);
    return e_success;
}

Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    char arr[32];
    fread(arr, 32, 1, encInfo->fptr_src_image);
    encode_int_to_image(file_size, arr);
    fwrite(arr, 32, 1, encInfo->fptr_stego_image);
    return e_success;
}

Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char str[encInfo->size_secret_file + 1];
    rewind(encInfo->fptr_secret);
    fread(str, encInfo->size_secret_file, 1, encInfo->fptr_secret);

    encode_string_to_image(str, encInfo->size_secret_file, encInfo);
    return e_success;
}
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char ch;

    while (fread(&ch, 1, 1, fptr_src) == 1)
    {
        fwrite(&ch, 1, 1, fptr_dest);
    }

    return e_success;
}
Status do_encoding(EncodeInfo *encInfo)
{
    if(open_files(encInfo) == e_success)
    {
        printf("INFO: Open files is completed\n");
    }
    else
    {
        printf("ERROR: Open files is a failure\n");
        return e_failure;
    }

    if(check_capacity(encInfo) == e_success)
    {
        printf("INFO: Image has enough capacity\n");
    }
    else
    {
        printf("ERROR: Secret file is too big for this image\n");
        return e_failure;
    }

    copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image);

    encode_magic_string(MAGIC_STRING, encInfo);

    encode_size_extn(encInfo);

    encode_secret_file_extn(encInfo->extn_secret_file, encInfo);

    encode_secret_file_size(encInfo->size_secret_file, encInfo);

    encode_secret_file_data(encInfo);

    copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image);

    // close all the files
    fclose(encInfo->fptr_src_image);
    fclose(encInfo->fptr_secret);
    fclose(encInfo->fptr_stego_image);

    return e_success;
}

void encode_string_to_image(const char *str, int size, EncodeInfo *encInfo)
{
    char arr[8];
    for(int i = 0; i < size ; i++)
    {
        fread(arr, 8, 1, encInfo->fptr_src_image);
        encode_byte_to_lsb(str[i], arr);
        fwrite(arr, 8, 1, encInfo->fptr_stego_image);
    }
}

void encode_byte_to_lsb(char ch, char *arr)
{
    for(int i = 0; i < 8; i++)
    {
        char get = (ch >> i) & 1;
        arr[i] = arr[i] & (~1);
        arr[i] = arr[i] | get;
    }
}

void encode_int_to_image(int size, char *arr)
{
    for(int i = 0; i < 32; i++)
    {
        char get = (size >> i) & 1;
        arr[i] = arr[i] & (~1);
        arr[i] = arr[i] | get;
    }
}