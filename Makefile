TARGET	= main.exe
CC 		= gcc
CFLAGS 	= -Wall -Wextra -std=c2x
LDFLAGS = -lm
ODIR 	= obj

SRC 	= $(wildcard *.c)
OBJ		= $(patsubst %.c,$(ODIR)/%.o,$(SRC))
DEP		= $(OBJ:.o=.d) 

.PHONY: all clean

all: $(TARGET)

$(ODIR):
	mkdir $@

$(ODIR)/%.o: %.c | $(ODIR)
	$(CC) -c $< $(CFLAGS) -o $@

$(TARGET): $(OBJ)
	$(CC) $^ $(LDFLAGS) -o $@

clean:
	del /f $(TARGET)
	del /f $(ODIR)\*.o $(ODIR)\*.d

-include $(DEP)
