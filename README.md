# LSB Image Steganography in C

Hide any file (text, code, anything with an extension) inside a 24-bit **BMP image**, and get it back out later. The image still looks exactly the same to the eye.

![Original image (left) and image with a hidden file (right)](docs/before_after.png)

*Left: the original image. Right: the same image with `secret.txt` hidden inside it. They look identical.*

## How it works

Every pixel in a 24-bit BMP is stored as 3 bytes (blue, green, red). Changing the **least significant bit (LSB)** of a byte changes its colour value by at most 1 out of 255, which the eye cannot see.

The program hides **one bit of secret data in the LSB of each image byte**:

- 1 character (8 bits) uses 8 image bytes
- 1 integer (32 bits) uses 32 image bytes

```
image byte:   1 0 1 1 0 1 1 0      secret bit: 1
                            ↓
new byte:     1 0 1 1 0 1 1 1      (only the last bit changed)
```

The bit operations are the same as in embedded register programming:

```c
arr[i] = arr[i] & ~1;          // clear the LSB
arr[i] = arr[i] | bit;         // set it to the secret bit
bit    = (ch >> i) & 1;        // get bit i of the secret byte
```

## What is stored, and where

For a 25-byte `secret.txt`:

| Image bytes | Count | Hidden item | Value |
|---|---|---|---|
| 0 – 53 | 54 | BMP header (copied, never changed) | — |
| 54 – 69 | 16 | Magic string (2 chars × 8) | `#*` |
| 70 – 101 | 32 | Extension size (int) | 4 |
| 102 – 133 | 32 | Extension (4 chars × 8) | `.txt` |
| 134 – 165 | 32 | Secret file size (int) | 25 |
| 166 – 365 | 200 | Secret data (25 chars × 8) | `My password is SECRET ;)` |
| 366 – end | rest | Remaining image (copied, never changed) | — |

- The **magic string** lets the decoder check whether an image actually contains hidden data.
- Storing the **extension** means the decoder recreates the right file type (`.txt`, `.c`, `.sh` ...).
- Storing the **size** tells the decoder exactly how many bytes to read back.

## Build

```bash
make
```

or without make:

```bash
gcc -Wall -Wextra main.c encode.c decode.c -o stego
```

It compiles with **0 errors and 0 warnings**.

## Run

**Encode** (hide `secret.txt` inside `beautiful.bmp`):

```bash
./stego -e secret.txt beautiful.bmp              # creates stego.bmp
./stego -e secret.txt beautiful.bmp out.bmp      # choose the output name
```

**Decode** (get the file back):

```bash
./stego -d stego.bmp                             # creates decoded.txt
./stego -d stego.bmp myfile                      # creates myfile.txt
```

### Example output

```
$ ./stego -e secret.txt beautiful.bmp
INFO: Open files is completed
width = 1024
height = 768
INFO: Image has enough capacity
INFO: Encoding is completed

$ ./stego -d stego.bmp
INFO: Decoding is selected and validated
INFO: Open files is completed
INFO: Magic string matched
INFO: Extension size decoded: 4
INFO: Extension decoded: .txt, output file: decoded.txt
INFO: Secret file size decoded: 25
INFO: Secret data decoded
INFO: Decoding is completed

$ cat decoded.txt
My password is SECRET ;)
```

## Error handling

| Situation | Message |
|---|---|
| Secret file has no extension | `ERROR: Secret file must have an extension (eg: secret.txt)` |
| Source or output image is not `.bmp` | `ERROR: Source image must be a .bmp file` |
| Secret file is too big for the image | `ERROR: Secret file is too big for this image` |
| Image has no hidden data | `ERROR: Magic string not found. This image has no hidden data` |
| File cannot be opened | `ERROR: Unable to open file <name>` |
| Wrong option | `ERROR: Use -e for encoding or -d for decoding` |

**Capacity check:** before encoding, the program compares the space needed (`(magic + ext size + ext + file size + data) × 8` bytes) with the image's pixel data (`width × height × 3`) and stops if the secret does not fit.

## Project structure

```
├── main.c        # reads the command line and calls encode or decode
├── encode.c/.h   # opens files, checks capacity, hides data in the LSBs
├── decode.c/.h   # checks the magic string, reads the hidden data back
├── types.h       # Status and OperationType enums
├── common.h      # MAGIC_STRING
├── Makefile
├── beautiful.bmp # sample image
└── secret.txt    # sample secret file
```

## Concepts used

C file I/O (`fopen`, `fread`, `fwrite`, `fseek`, `ftell`, `rewind`), bitwise operations (`&`, `|`, `~`, `>>`, `<<`), structures, enums, command-line arguments, modular multi-file programming, and the BMP file format.

## Limitations

- Works with uncompressed **24-bit BMP** images that have the standard 54-byte header.
- The data is hidden, not encrypted. Anyone who runs the decoder on the image can read it.

## Author

**N.M. Gayathri**, built during embedded systems training at Emertxe Information Technologies, Bangalore.
