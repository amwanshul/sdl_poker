# Makefile for raylib-poker on Windows with MinGW-w64

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -I.
LDFLAGS = -L. -lraylib -lopengl32 -lgdi32 -lwinmm

# Project name
TARGET = poker

# Source files
SRCS = raylib_poker.c

# Object files
OBJS = $(SRCS:.c=.o)

# Default target
all: $(TARGET)

# Link the executable
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET).exe $(LDFLAGS)

# Compile source files into object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Run the game
run: all
	./$(TARGET).exe

# Clean up build files
clean:
	del /Q *.o *.exe

.PHONY: all run clean
