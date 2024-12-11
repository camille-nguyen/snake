#include "raylib.h"

#define NDEBUG
#define SCREEN_WIDTH (1000)
#define SCREEN_HEIGHT (1000)
#define HUD_HEIGHT (200)
#define GAME_HEIGHT (SCREEN_HEIGHT - HUD_HEIGHT)
#define GRID_BLOCK_LENGTH (40)
#define BLOCKS_IN_ROW (int)(SCREEN_WIDTH / GRID_BLOCK_LENGTH)
#define BLOCKS_IN_COLUMN (int)(GAME_HEIGHT / GRID_BLOCK_LENGTH)
#define MAX_QUEUE_SIZE (BLOCKS_IN_ROW * BLOCKS_IN_COLUMN)

typedef struct Vector2Int
{
    int x;
    int y;
} Vector2Int;

typedef struct TailPositions
{
    Vector2Int array[MAX_QUEUE_SIZE];
    int size;
} TailPositions;

typedef enum Direction
{
    none,
    up,
    right,
    down,
    left
} Direction;

#define ASSET_PATH "../assets/"

int score;

bool game_grid[BLOCKS_IN_ROW][BLOCKS_IN_COLUMN];

Direction current_direction;
Vector2Int player_pos;
Vector2Int tictac_pos;

TailPositions tail;

void reset_game()
{
    //set all grid blocks to false
    for (int i = 0; i < BLOCKS_IN_ROW; ++i)
    {
        for (int j = 0; j < BLOCKS_IN_COLUMN; ++j)
        {
            game_grid[i][j] = false;
        }
    }
    
    current_direction = none;
    score = 0;
    player_pos = (Vector2Int) {7, BLOCKS_IN_COLUMN / 2};
    tictac_pos = (Vector2Int) {BLOCKS_IN_ROW - 5, BLOCKS_IN_COLUMN / 2};
    
    //give snake 3 blocks of tail to start
    tail = (TailPositions) {
            .array = {(Vector2Int) {6, BLOCKS_IN_COLUMN / 2},
                      (Vector2Int) {5, BLOCKS_IN_COLUMN / 2},
                      (Vector2Int) {4, BLOCKS_IN_COLUMN / 2}},
            .size = 3,
    };
}

int main(void)
{
    // Initialization
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "SNAAAAAAAAKE!");
    
    SetTargetFPS(60); // Set our game to run at 60 frames-per-second
    
    double tick_time = 0.2;
    double time = GetTime();
    int high_score = 0;
    
    reset_game();
    
    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        //get input to change direction
        switch (current_direction)
        {
            case none:
                if (IsKeyPressed(KEY_UP)) { current_direction = up; }
                else if (IsKeyPressed(KEY_RIGHT)) { current_direction = right; }
                else if (IsKeyPressed(KEY_DOWN)) { current_direction = down; }
                break;
            case up:
            case down:
                if (IsKeyPressed(KEY_RIGHT)) { current_direction = right; }
                else if (IsKeyPressed(KEY_LEFT)) { current_direction = left; }
                break;
            case right:
            case left:
                if (IsKeyPressed(KEY_UP)) { current_direction = up; }
                else if (IsKeyPressed(KEY_DOWN)) { current_direction = down; }
                break;
        }
        
        //in-game tick
        if (current_direction != none && GetTime() >= time + tick_time)
        {
            time = GetTime();
            
            //turn off end of tail
            game_grid[tail.array[tail.size - 1].x][tail.array[tail.size - 1].y] = false;
            
            //move snake head one space in current direction O R  J U S T  K I L L  I T
            Vector2Int next_pos = player_pos;
            switch (current_direction)
            {
                case up:
                    if (player_pos.y == 0) { reset_game(); }
                    else { next_pos.y = player_pos.y - 1; }
                    break;
                case right:
                    if (player_pos.x == BLOCKS_IN_ROW - 1) { reset_game(); }
                    else { next_pos.x = player_pos.x + 1; }
                    break;
                case down:
                    if (player_pos.y == BLOCKS_IN_COLUMN - 1) { reset_game(); }
                    else { next_pos.y = player_pos.y + 1; }
                    break;
                case left:
                    if (player_pos.x == 0) { reset_game(); }
                    else { next_pos.x = player_pos.x - 1; }
                    break;
            }
            
            //check next_pos does not overlap tail
            for (int i = 0; i < tail.size; ++i)
            {
                if (next_pos.x == tail.array[i].x && next_pos.y == tail.array[i].y)
                {
                    reset_game();
                }
            }
            
            //eat a tictac
            if (next_pos.x == tictac_pos.x && next_pos.y == tictac_pos.y)
            {
                tail.size += 1;
                score += 1;
                if (high_score < score) { high_score = score; }
                
                //relocate tictac
                while (1)
                {
                    tictac_pos.x = GetRandomValue(0, BLOCKS_IN_ROW - 1);
                    tictac_pos.y = GetRandomValue(0, BLOCKS_IN_COLUMN - 1);
                    if (tictac_pos.x == next_pos.x && tictac_pos.y == next_pos.y ||
                        tictac_pos.x == player_pos.x && tictac_pos.y == player_pos.y)
                    {
                        continue;
                    }
                    for (int i = 0; i < tail.size; ++i)
                    {
                        if(next_pos.x == tail.array[i].x && next_pos.y == tail.array[i].y){
                            continue;
                        }
                    }
                    break;
                }
            }
            
            //move all tail blocks along
            for (int i = tail.size - 1;
                 i >= 0; --i)
            {
                if (i == 0)
                {
                    tail.array[i] = player_pos;
                } else
                {
                    tail.array[i] = tail.array[i - 1];
                }
            }
            
            if (current_direction != none) { player_pos = next_pos; }
        }
        
        //update positions of blocks
        //TODO: only go into this code when a change is made to any of the Vector2Int structs
        {
            game_grid[player_pos.x][player_pos.y] = true;
            game_grid[tictac_pos.x][tictac_pos.y] = true;
            
            for (int i = 0; i < tail.size; ++i)
            {
                game_grid[tail.array[i].x][tail.array[i].y] = true;
            }
        }
        
        
        BeginDrawing();
        
        //HUD
        BeginScissorMode(0, 0, SCREEN_WIDTH, HUD_HEIGHT);
        ClearBackground(GOLD);
        DrawText(FormatText("Tic Tacs Collected: %i", score), SCREEN_WIDTH * 0.1,
                 HUD_HEIGHT * 0.4, 20, BLACK);
        DrawText(FormatText("High Score: %i", high_score), SCREEN_WIDTH * 0.6,
                 HUD_HEIGHT * 0.4, 20, BLACK);
        EndScissorMode();
        
        //game grid
        BeginScissorMode(0, HUD_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT - HUD_HEIGHT);
        ClearBackground(BLACK);
        
        //draw grid block borders for debugging
        #ifndef NDEBUG
        for (int i = 0; i < BLOCKS_IN_ROW; ++i)
        {
            for (int j = 0; j < BLOCKS_IN_COLUMN; ++j)
            {
                DrawRectangleLines(i * GRID_BLOCK_LENGTH,
                                   HUD_HEIGHT + j * GRID_BLOCK_LENGTH,
                                   GRID_BLOCK_LENGTH, GRID_BLOCK_LENGTH, WHITE);
            }
        }
        #endif
        
        //recolour blocks (on/off)
        for (int i = 0; i < BLOCKS_IN_ROW; ++i)
        {
            for (int j = 0; j < BLOCKS_IN_COLUMN; ++j)
            {
                if (game_grid[i][j])
                {
                    DrawRectangle(i * GRID_BLOCK_LENGTH,
                                  HUD_HEIGHT + j * GRID_BLOCK_LENGTH, GRID_BLOCK_LENGTH,
                                  GRID_BLOCK_LENGTH, DARKGREEN);
                }
            }
        }
        
        //draw soft border around snake head
        DrawRectangleLines(player_pos.x * GRID_BLOCK_LENGTH,
                           HUD_HEIGHT + player_pos.y * GRID_BLOCK_LENGTH,
                           GRID_BLOCK_LENGTH, GRID_BLOCK_LENGTH, DARKBROWN);
        //make ticc tacc white
        DrawRectangle(tictac_pos.x * GRID_BLOCK_LENGTH,
                      HUD_HEIGHT + tictac_pos.y * GRID_BLOCK_LENGTH,
                      GRID_BLOCK_LENGTH, GRID_BLOCK_LENGTH, RAYWHITE);
        
        
        EndScissorMode();
        
        EndDrawing();
    }
    
    // De-Initialization
    
    CloseWindow(); // Close window and OpenGL context
    
    return 0;
}
