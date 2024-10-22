# snake

## background
The classic game, Snake!

Implemented using the [SDL2 game development libarary](https://www.libsdl.org/) as well as
[Webassembly](https://emscripten.org/docs/compiling/WebAssembly.html). Combining these two technologies means that you can use C/C++ to write video
games what will play in your Web Browser!

You can [play the game here](https://hec.to/r/snake/) on my website.

## build instructions
Prerequisites: You will need to install the SDL2 library as well as Emscripten, the webassembly compiler. I have only
built this on Linux.

To build the full program, run `make web`.

You can skip the webassembly part of the build by simply running `make`. This is faster and useful for just debugging
the C code.

You can clean the build and delete the webassembly artifacts with `make clean`.

The `upload.sh` script will upload the files to run the game to your webserver. I have my webserver hardcoded, but you
can easily replace the URL.

## game instructions
Use arrow keys to move. Press Space Bar to reset. Score is displayed when you Game Over.
