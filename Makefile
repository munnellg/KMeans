OBJS = $(wildcard src/*.c)

CC = gcc

LINKER_FLAGS = -lm -lSDL2 -lSDL2_image

COMPILER_FLAGS = -Wall

OBJ = kmeans

all: $(OBJS)
	$(CC) $(COMPILER_FLAGS) $(OBJS) -o $(OBJ) $(LINKER_FLAGS) 
