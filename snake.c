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
#define SCREEN_HEIGHT 640
#define SQUARE_SIZE 64
#define BOARD_WIDTH  (SCREEN_WIDTH  / SQUARE_SIZE)
#define BOARD_HEIGHT (SCREEN_HEIGHT / SQUARE_SIZE)

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
bool game_over = false;
bool win = false;

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
    if(SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
        success = false;
    }
    else
    {
        //Set texture filtering to linear
        if(!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
        {
            printf("Warning: Linear texture filtering not enabled!");
        }

        //Create window
        gWindow = SDL_CreateWindow("Snake", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if(gWindow == NULL)
        {
            printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
            success = false;
        }
        else
        {
            //Create renderer for window
            gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
            if(gRenderer == NULL)
            {
                printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
                success = false;
            }
            else
            {
                //Initialize renderer color
                SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
            }
        }
    } 
    //Initialize SDL_ttf
    if(TTF_Init() == -1)
    {
        printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
        success = false;
    }
    else
    {
        gFont = TTF_OpenFont("asset_dir/DejaVuSansMono.ttf", SCREEN_HEIGHT);
        if(gFont == NULL)
        {
            printf("Failed to load font: SDL_ttf Error: %s\n", TTF_GetError());
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
    SDL_Surface* surface = TTF_RenderText_Solid(font, text, *color);
    if(surface == NULL)
    {
        printf("Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError());
        return NULL;
    }

    //Create texture from surface pixels
    SDL_Texture *texture = SDL_CreateTextureFromSurface(gRenderer, surface);
    if(texture == NULL)
    {
        printf("Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError());
        return NULL;
    }
    //Get rid of old surface
    SDL_FreeSurface(surface);

    return texture;
}

void snake_reset()
{
    win = false;
    game_over = false;
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
    uint32_t first_new_apple = new_apple;

    // Check if the new apple is in a snake or existing apple
    while(snake_p[new_apple] || apple_p[new_apple])
    {
        new_apple++;
        if(new_apple >= BOARD_WIDTH * BOARD_HEIGHT)
            new_apple = 0;

        // If no valid apple placements exist, player has won the game
        if(new_apple == first_new_apple)
        {
            game_over = true;
            win = true;
            return;
        }
    }
    // Found a valid apple placement
    apple_p[new_apple] = 1;
}

void move_snake()
{
    // Don't do anything if we are in game over state
    if(game_over)
        return;

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
        return;

    //Set the new snake head location with a non-zero value so it get's drawn.
    //The actual value doens't matter since it will get written with the snake direction next frame.
    snake[snake_head_x][snake_head_y] = snake_dir;

    if(apple[snake_head_x][snake_head_y])
    {
        //Ate the apple, delete it and get a new one
        points++;
        apple[snake_head_x][snake_head_y] = 0;
        snake_new_apple();
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
}

void snake_loop()
{
    bool moved = false;

    //Only allow one valid key press per frame. 
    SDL_Event e;
    while(SDL_PollEvent(&e) != 0)
    {
        //User requests quit
        if(e.type == SDL_QUIT)
        {
            quit = true;
        }
        //User presses a key
        else if(e.type == SDL_KEYDOWN)
        {
            //For more responseive gameplay, move the snake whenever the player inputs a valid move
            switch(e.key.keysym.sym)
            {
                case SDLK_UP:
                    if((snake_dir != up) && (snake_dir != down))
                    {
                        snake_dir = up;
                        move_snake();
                        moved = true;
                    }
                    break;

                case SDLK_DOWN:
                    if((snake_dir != up) && (snake_dir != down))
                    {
                        snake_dir = down;
                        move_snake();
                        moved = true;
                    }
                    break;

                case SDLK_LEFT:
                    if((snake_dir != left) && (snake_dir != right))
                    {
                        snake_dir = left;
                        move_snake();
                        moved = true;
                    }
                    break;

                case SDLK_RIGHT:
                    if((snake_dir != left) && (snake_dir != right))
                    {
                        snake_dir = right;
                        move_snake();
                        moved = true;
                    }
                    break;

                case SDLK_SPACE:
                    snake_reset();
                    break;

                default:
                    break;
            }
        }
    }

    //If the player hasn't made a move, advance the snake for them 
    if(moved == false)
        move_snake();

    if(game_over)
    {
        SDL_Color score_color={0,0,0};
        char score[256];

        if(win == true)
            sprintf(score, "%s", "win!");
        else
            sprintf(score, "%d", points);

        SDL_Texture *score_text = make_text_texture(gFont, score, &score_color);

        SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderClear(gRenderer);
        SDL_RenderCopy(gRenderer, score_text, NULL, NULL);
        SDL_RenderPresent(gRenderer);
        SDL_Delay(200);
        return;
    }

    // Clear screen before drawing the new one
    SDL_SetRenderDrawColor(gRenderer, 0xAA, 0xAA, 0xAA, 0xAA);
    SDL_RenderClear(gRenderer);

    //Draw the snake
    for(int i=0; i<(SCREEN_WIDTH/SQUARE_SIZE); ++i)
    {
        for(int j=0; j<(SCREEN_HEIGHT/SQUARE_SIZE); ++j)
        {
            if(snake[i][j])
            {
                SDL_Rect fillRect = {i*SQUARE_SIZE, j*SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE}; 
                SDL_SetRenderDrawColor(gRenderer, 0x00, 0xFF, 0x00, 0xFF);		
                SDL_RenderFillRect(gRenderer, &fillRect);
            }
            if(apple[i][j])
            {
                SDL_Rect fillRect = {i*SQUARE_SIZE, j*SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE}; 
                SDL_SetRenderDrawColor(gRenderer, 0xFF, 0x00, 0x00, 0xFF);		
                SDL_RenderFillRect(gRenderer, &fillRect);
            }
        }
    }

    //Update screen
    SDL_RenderPresent(gRenderer);

#if __EMSCRIPTEN__
    //no delay on webasm
#else
    //Speep up game for every apple
    uint32_t delay = 125;
    SDL_Delay(delay);
#endif
}

int main(int argc, char* args[])
{
    //Start up SDL and create window
    if(!snake_init())
    {
        printf("Failed to initialize!\n");
        return 1;
    }

    srand(time(NULL));
    snake_new_apple();

#if __EMSCRIPTEN__
    emscripten_set_main_loop(snake_loop, 8, 1);
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
