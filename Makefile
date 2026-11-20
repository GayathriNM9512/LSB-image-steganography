CC      = gcc
CFLAGS  = -Wall -Wextra
TARGET  = stego
SRCS    = main.c encode.c decode.c

$(TARGET): $(SRCS) encode.h decode.h types.h common.h
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

clean:
	rm -f $(TARGET) a.out stego.bmp decoded.*
