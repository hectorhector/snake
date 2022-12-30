BUILD_DIR = build

#OBJS specifies which files to compile as part of the project
CSRC = *.c

#CC specifies our native compiler
CC = gcc

#COMPILER_FLAGS specifies the additional compilation options we're using
COMPILER_FLAGS = -Wall

ifeq ($(debug),1)
   COMPILER_FLAGS += -g
else
   COMPILER_FLAGS += -Werror -O2
endif

#LINKER_FLAGS specifies the libraries we're linking against
LINKER_FLAGS = -lSDL2 -lSDL2_image -lSDL2_ttf

#OBJ_NAME specifies the name of our exectuable
BIN_NAME = snake

#This is the target that compiles our executable
all : $(CSRC)
	$(CC) $(CSRC) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(BIN_NAME)


#WASMCC specifies our webassembly compiler
WASMCC = emcc

#Emscripten settings
EMCC_SETTINGS = -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s USE_SDL_TTF=2

#Virtual File flags
VIRTUAL_FS = --preload-file asset_dir/

#HTML_NAME specifies the name of our web files
HTML_NAME = snake.html

web : $(CSRC)
	$(WASMCC) $(CSRC) $(EMCC_SETTINGS) $(VIRTUAL_FS) -o $(HTML_NAME)

clean:
	rm -f $(BIN_NAME) $(HTML_NAME) snake.data snake.js snake.wasm
