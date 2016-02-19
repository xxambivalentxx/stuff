SOURCE 	= source/
BUILD	= build/
INCLUDE	= include/
TARGET	= edata-parse
CFLAGS = -g -I $(INCLUDE) -m32 -std=gnu99 -O2 -Wall -Werror

all: $(TARGET)

clean:
	rm -f $(BUILD)*.o
	rm -f $(TARGET)

OBJ	= $(BUILD)main.o $(BUILD)parse.o $(BUILD)read.o

$(BUILD)%.o: $(SOURCE)%.c
	gcc $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJ)
	gcc $(CFLAGS) $(OBJ) -o $(TARGET)
	
