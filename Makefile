TARGET = main.exe
CC = gcc
CFLAGS = -Wall -Wextra -std=c2x

.PHONY: clean

$(TARGET): *.c *.h
	$(CC) $^ $(CFLAGS) -o $@

clean:
	del /f $(TARGET)
	del /f *.o
