BUILD_DIR = build

#OBJS specifies which files to compile as part of the project
CSRC = *.c

#CC specifies which compiler we're using
CC = gcc

#COMPILER_FLAGS specifies the additional compilation options we're using
COMPILER_FLAGS = -Wall

ifeq ($(debug),1)
   COMPILER_FLAGS += -g
else
   COMPILER_FLAGS += -Werror -O2
endif

#LINKER_FLAGS specifies the libraries we're linking against
LINKER_FLAGS = -lSDL2 -lSDL2_image

#OBJ_NAME specifies the name of our exectuable
BIN_NAME = snake

#This is the target that compiles our executable
all : $(CSRC)
	$(CC) $(CSRC) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(BIN_NAME)
