#include <stdio.h>
#include "encode.h"
#include "types.h"
#include <string.h>
#include "common.h"
#include "decode.h"

OperationType check_operation_type(char *argv[])
{
        if (strcmp(argv[1], "-e") == 0)
        return e_encode;
        else if (strcmp(argv[1], "-d") == 0)
        return e_decode;
    else
        return e_unsupported;


}

int main(int argc , char *argv[])
{
    if (argc < 2)       
    {                           
        printf("Usage: ./a.out -e <secret_file> <source.bmp> [output.bmp]\n");
        return 1;
    }
    OperationType res = check_operation_type(argv);

    if(res == e_encode)
    {
           if (argc < 4 || argc > 5)                   // ← add: encoding needs 4 or 5 args
        {
            printf("Usage: ./a.out -e <secret_file> <source.bmp> [output.bmp]\n");
            return 1;
        }
        // do_encode
        EncodeInfo encInfo;
        if(read_and_validate_encode_args(argv, &encInfo) == e_success)
        {
            // ptintf("Encoding is selected and validated\n");
            if(do_encoding(&encInfo) == e_success)
            {
                printf("INFO: Encoding is completed\n");
            }
            else
            {
                printf("ERROR: Encoding is failed\n");
            }
        }
        else
        {
            printf("Encoding validation is a failure");
        }
    }
        else if(res == e_decode)
    {
        if (argc < 3 || argc > 4)
        {
            printf("Usage: ./a.out -d <stego.bmp> [output_name]\n");
            return 1;
        }

        DecodeInfo decInfo;
        if (read_and_validate_decode_args(argv, &decInfo) == e_success)
        {
            printf("INFO: Decoding is selected and validated\n");
            if (do_decoding(&decInfo) == e_success)
            {
                printf("INFO: Decoding is completed\n");
            }
            else
            {
                printf("ERROR: Decoding is failed\n");
            }
        }
        else
        {
            printf("ERROR: Decoding validation is a failure\n");
        }
    }
    else
    {
         printf("ERROR: Use -e for encoding or -d for decoding\n");
    }
    return 0;
}
