#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#if __EMSCRIPTEN__
#include <emscripten.h>
#endif

//Screen dimension constants
#define SCREEN_WIDTH  640
#define SCREEN_HEIGHT 480
#define SQUARE_SIZE 32
#define BOARD_WIDTH  (SCREEN_WIDTH  / SQUARE_SIZE) // 640/32 = 20
#define BOARD_HEIGHT (SCREEN_HEIGHT / SQUARE_SIZE) // 480/32 = 15 

// SDL init/quit
bool snake_init();
void snake_close();

void snake_reset();
void snake_new_apple();

//utility text funtion
SDL_Texture* make_text_texture(TTF_Font *font, char *text, SDL_Color *color);

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;

//score text
TTF_Font *gFont = NULL;
SDL_Texture* g_score_texture = NULL;

bool quit = false;

uint32_t points = 0;

uint8_t snake[BOARD_WIDTH][BOARD_HEIGHT] = {0};
uint8_t apple[BOARD_WIDTH][BOARD_HEIGHT] = {0};

uint8_t snake_head_x, snake_head_y = 0;
uint8_t snake_tail_x, snake_tail_y = 0;

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
        gWindow = SDL_CreateWindow( "Snake", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
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
    //Initialize SDL_ttf
    if( TTF_Init() == -1 )
    {
        printf( "SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError() );
        success = false;
    }
    else
    {
        gFont = TTF_OpenFont( "asset_dir/DejaVuSansMono.ttf", SCREEN_HEIGHT);
        if( gFont == NULL )
        {
            printf( "Failed to load font: SDL_ttf Error: %s\n", TTF_GetError() );
            success = false;
        }
    }

    return success;
}

void snake_close()
{
    if(gRenderer)
        SDL_DestroyRenderer(gRenderer);
    if(gWindow)
        SDL_DestroyWindow(gWindow);
    if(gFont)
        TTF_CloseFont(gFont);
    if(g_score_texture)
        SDL_DestroyTexture(g_score_texture);
    SDL_Quit();
}

SDL_Texture* make_text_texture(TTF_Font *font, char *text, SDL_Color *color)
{
    SDL_Surface* surface = TTF_RenderText_Solid( font, text, *color);
    if( surface == NULL )
    {
        printf( "Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError() );
        return NULL;
    }

    //Create texture from surface pixels
    SDL_Texture *texture = SDL_CreateTextureFromSurface( gRenderer, surface);
    if( texture == NULL )
    {
        printf( "Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError() );
        return NULL;
    }
    //Get rid of old surface
    SDL_FreeSurface(surface);

    return texture;
}

void snake_reset()
{
    points = 0;

    memset(snake, 0, BOARD_WIDTH * BOARD_HEIGHT);
    memset(apple, 0, BOARD_WIDTH * BOARD_HEIGHT);

    snake_head_x = 0;
    snake_head_y = 0;
    snake_tail_x = 0;
    snake_tail_y = 0;

    snake_dir = right;

    snake_new_apple();
}
void snake_new_apple()
{
    uint8_t *snake_p = (uint8_t *)snake;
    uint8_t *apple_p = (uint8_t *)apple;

    //Choose a random cell
    uint32_t new_apple = rand() % (BOARD_WIDTH * BOARD_HEIGHT);

    while(snake_p[new_apple] || apple_p[new_apple])
    {
        new_apple++;
        if(new_apple >= BOARD_WIDTH * BOARD_HEIGHT)
            new_apple = 0;
    }
    apple_p[new_apple] = 1;
}

void snake_loop()
{
    static bool game_over = false;

    do
    {
        //Only allow one valid key press per frame. 
        SDL_Event e;
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
                    case SDLK_SPACE:
                        snake_reset();
                        game_over = false;
                        break;
                    default:
                        continue;
                }
                // Got a valid key press, use it this frame
                break;
            }
        }

        if(game_over)
        {
            SDL_Color score_color={0,0,0};
            char score[256];
            sprintf(score, "%d", points);
            SDL_Texture *t = make_text_texture(gFont, score, &score_color);

            SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );
            SDL_RenderClear(gRenderer);
            SDL_RenderCopy(gRenderer, t, NULL, NULL);
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
                if(snake_head_y != BOARD_HEIGHT-1)
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
                if(snake_head_x != BOARD_WIDTH-1)
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

        if(apple[snake_head_x][snake_head_y])
        {
            //Ate the apple, delete it and get a new one
            apple[snake_head_x][snake_head_y] = 0;
            snake_new_apple();
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
        for(int i=0; i<(SCREEN_WIDTH/SQUARE_SIZE); ++i)
        {
            for(int j=0; j<(SCREEN_HEIGHT/SQUARE_SIZE); ++j)
            {
                if(snake[i][j])
                {
                    SDL_Rect fillRect = {i*SQUARE_SIZE, j*SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE}; 
                    SDL_SetRenderDrawColor( gRenderer, 0x00, 0xFF, 0x00, 0xFF );		
                    SDL_RenderFillRect( gRenderer, &fillRect );
                }
                if(apple[i][j])
                {
                    SDL_Rect fillRect = {i*SQUARE_SIZE, j*SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE}; 
                    SDL_SetRenderDrawColor( gRenderer, 0xFF, 0x00, 0x00, 0xFF );		
                    SDL_RenderFillRect( gRenderer, &fillRect );
                }
            }
        }

        //Update screen
        SDL_RenderPresent( gRenderer );

        //Speep up game for every apple
        int delay = 150;
        if(points * 2 > delay)
            delay = 0;
        else
            delay -= 2 * points;
        SDL_Delay(delay);
    }
    while(0);
}

int main( int argc, char* args[] )
{
    //Start up SDL and create window
    if( !snake_init() )
    {
        printf( "Failed to initialize!\n" );
        return 1;
    }

    srand(time(NULL));
    snake_new_apple();

#if __EMSCRIPTEN__
    emscripten_set_main_loop(snake_loop, 0, 1);
#else
    while(quit == false)
    {
        snake_loop();
    }
#endif

    //Free resources and close SDL
    snake_close();

    return 0;
}
