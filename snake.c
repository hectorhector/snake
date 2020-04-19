#include <SDL2/SDL.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>

//Screen dimension constants
#define SCREEN_WIDTH  640
#define SCREEN_HEIGHT 480

// 640/32 = 20
// 480/32 = 15
#define BOARD_WIDTH  (SCREEN_WIDTH  / 32)
#define BOARD_HEIGHT (SCREEN_HEIGHT / 32)

// SDL init/quit
bool snake_init();
void snake_close();

void snake_new_apple();

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;

uint8_t snake[BOARD_WIDTH][BOARD_HEIGHT] = {0};
uint8_t apple[BOARD_WIDTH][BOARD_HEIGHT] = {0};

uint8_t snake_head_x, snake_head_y = 0;
uint8_t snake_tail_x, snake_tail_y = 0;
uint8_t apple_x, apple_y = 0;

enum e_snake_dir
{
    up = 1,
    down,
    left,
    right
};
enum e_snake_dir snake_dir = right;

bool snake_init()
{
	//Initialization flag
	bool success = true;

	//Initialize SDL
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
	{
		printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
		success = false;
	}
	else
	{
		//Set texture filtering to linear
		if( !SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" ) )
		{
			printf( "Warning: Linear texture filtering not enabled!" );
		}

		//Create window
		gWindow = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
		if( gWindow == NULL )
		{
			printf( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
			success = false;
		}
		else
		{
			//Create renderer for window
			gRenderer = SDL_CreateRenderer( gWindow, -1, SDL_RENDERER_ACCELERATED );
			if( gRenderer == NULL )
			{
				printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
				success = false;
			}
			else
			{
				//Initialize renderer color
				SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );
			}
		}
	}

    return success;
}

void snake_close()
{
	//Destroy window	
	SDL_DestroyRenderer( gRenderer );
	SDL_DestroyWindow( gWindow );
	gWindow = NULL;
	gRenderer = NULL;

	SDL_Quit();
}

void snake_new_apple()
{
    do
    {
        apple_x = rand() % BOARD_WIDTH;
        apple_y = rand() % BOARD_HEIGHT;
    }
    while(snake[apple_x][apple_y]);

    apple[apple_x][apple_y] = 1;
}

int main( int argc, char* args[] )
{
    uint32_t points = 0;

	//Start up SDL and create window
	if( !snake_init() )
	{
		printf( "Failed to initialize!\n" );
	}
	else
	{
        bool quit = false;
        bool game_over = false;

        //Event handler
        SDL_Event e;

        srand(time(NULL));
        snake_new_apple();

        //While application is running
        while( !quit )
        {

            //Handle events on queue
            while( SDL_PollEvent( &e ) != 0 )
            {
                //User requests quit
                if( e.type == SDL_QUIT )
                {
                    quit = true;
                }
                //User presses a key
                else if( e.type == SDL_KEYDOWN )
                {
                    //Only allow one valid key press per frame. 
                    //As soon as you get one, stop processing events and use the key for the next frame.
                    switch( e.key.keysym.sym )
                    {
                        case SDLK_UP:
                            if(snake_dir == up || snake_dir == down)
                                continue;
                            else
                            {
                                snake_dir = up;
                                break;
                            }

                        case SDLK_DOWN:
                            if(snake_dir == up || snake_dir == down)
                                continue;
                            else
                            {
                                snake_dir = down;
                                break;
                            }

                        case SDLK_LEFT:
                            if(snake_dir == left || snake_dir == right)
                                continue;
                            else
                            {
                                snake_dir = left;
                                break;
                            }

                        case SDLK_RIGHT:
                            if(snake_dir == left || snake_dir == right)
                                continue;
                            else
                            {
                                snake_dir = right;
                                break;
                            }
                        default:
                            continue;
                    }
                    // Got a valid key press, use it this frame
                    break;
                }
            }

            if(game_over)
            {
                SDL_SetRenderDrawColor( gRenderer, 0x00, 0x00, 0xFF, 0xFF );
                SDL_RenderClear( gRenderer );
                SDL_RenderPresent( gRenderer );
                SDL_Delay(200);
                continue;
            }

            //Before we move, save the direction of where we are going in this grid square
            snake[snake_head_x][snake_head_y] = snake_dir;

            //Move to the next grid square
            switch(snake_dir)
            {
                case up:
                    if(snake_head_y != 0)
                        snake_head_y--;
                    else
                        game_over = true;
                    break;
                case down:
                    if(snake_head_y != 14)
                        snake_head_y++;
                    else
                        game_over = true;
                    break;
                case left:
                    if(snake_head_x != 0)
                        snake_head_x--;
                    else
                        game_over = true;
                    break;
                case right:
                    if(snake_head_x != 19)
                        snake_head_x++;
                    else
                        game_over = true;
                    break;
            }
            if(snake[snake_head_x][snake_head_y])
            {
                game_over = true;
            }
            if(game_over)
            {
                continue;
            }

            //Set the new snake head location with a non-zero value so it get's drawn.
            //The actual value doens't matter since it will get written with the snake direction next frame.
            snake[snake_head_x][snake_head_y] = snake_dir;

            if(snake_head_x == apple_x && snake_head_y == apple_y)
            {
                //Ate the apple, delete it and get a new one
                apple[apple_x][apple_y] = 0;
                snake_new_apple();
                apple[apple_x][apple_y] = 1;
                points++;
            }
            else
            {
                //Erase old tail and get the new one
                uint8_t snake_tail_dir = snake[snake_tail_x][snake_tail_y];
                snake[snake_tail_x][snake_tail_y] = 0;
                switch(snake_tail_dir)
                {
                    case up:
                        snake_tail_y--;
                        break;
                    case down:
                        snake_tail_y++;
                        break;
                    case left:
                        snake_tail_x--;
                        break;
                    case right:
                        snake_tail_x++;
                        break;
                }
            }

            //Clear screen
            SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );
            SDL_RenderClear( gRenderer );
            //Draw the snake
            for(int i=0; i<20; ++i)
            {
                for(int j=0; j<15; ++j)
                {
                    if(snake[i][j])
                    {
                        SDL_Rect fillRect = {i*32, j*32, 32, 32}; 
                        SDL_SetRenderDrawColor( gRenderer, 0x00, 0xFF, 0x00, 0xFF );		
                        SDL_RenderFillRect( gRenderer, &fillRect );
                    }
                    if(apple[i][j])
                    {
                        SDL_Rect fillRect = {i*32, j*32, 32, 32}; 
                        SDL_SetRenderDrawColor( gRenderer, 0xFF, 0x00, 0x00, 0xFF );		
                        SDL_RenderFillRect( gRenderer, &fillRect );
                    }
                }
            }

            //Update screen
            SDL_RenderPresent( gRenderer );

            //Speep up game for every apple
            uint32_t  delay = 150;
            if(points * 2 > delay)
                delay = 0;
            else
                delay -= 2 * points;
            SDL_Delay(delay);
        }
	}

	//Free resources and close SDL
	snake_close();

	return 0;
}
