#ifndef DECODE_H
#define DECODE_H

#include <stdio.h>
#include "types.h"

typedef struct _DecodeInfo
{
    /* Stego image info */
    char *stego_image_fname;
    FILE *fptr_stego_image;

    /* Output (decoded) file info */
    char output_fname[100];
    FILE *fptr_output;
    int extn_size;
    char extn_output_file[10];
    long size_output_file;

} DecodeInfo;

Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);
Status do_decoding(DecodeInfo *decInfo);
Status open_decode_files(DecodeInfo *decInfo);
Status decode_magic_string(DecodeInfo *decInfo);
Status decode_size_extn(DecodeInfo *decInfo);
Status decode_secret_file_extn(DecodeInfo *decInfo);
Status decode_secret_file_size(DecodeInfo *decInfo);
Status decode_secret_file_data(DecodeInfo *decInfo);
void decode_string_from_image(char *str, int size, DecodeInfo *decInfo);
char decode_byte_from_lsb(char *arr);
int decode_int_from_image(char *arr);

#endif