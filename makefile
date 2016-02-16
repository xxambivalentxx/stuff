SOURCE 	= source/
BUILD	= build/
INCLUDE	= include/
CFLAGS = -m32 -std=gnu99 -O2 -Wall -Werror

OBJ	= $(BUILD)main.o

$(BUILD)%.o: $(SOURCE)*/%.c
	gcc $(CFLAGS) -c $< -o $@
