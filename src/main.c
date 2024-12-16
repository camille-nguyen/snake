#include "raylib.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define MIN_SIZE 5
#define MAX_SIZE 20
#define SNAKE_INITIAL_LENGTH 1
#define GRID_CELL_SIZE 20
#define MOVEMENT_INTERVAL 0.5f
#define VERTICAL_OFFSET 110
#define BOOSTER_RESPAWN_TIME 6.0f

/* ------------------------- GLOBAL VARIABLES ------------------------- */

int mapWidth = MIN_SIZE, mapHeight = MIN_SIZE;

// Movement speed variables
float normalSpeed = 0.5f; // Normal movement interval
float boostedSpeed = 0.25f; // Faster movement interval
float speedDuration = 5.0f; // Booster effect duration (seconds)
float boosterTimer = 0.0f;  // Timer to track booster effect
float currentSpeed = 0.5f;  // Current movement interval

int extraLives = 0;  // Track the player's extra lives

/* ------------------------- STRUCTURES ------------------------- */
typedef struct Snake {
    Vector2 *body;
    int length;
    Vector2 direction;
} Snake;

typedef struct Food {
    Vector2 position;
} Food;

typedef struct Booster {
    Vector2 position;
    bool isActive;
    int type;          // (0 = speed, 1 = size reducer, 2 = extra life)
} Booster;

/* ------------------------- INIT ------------------------- */

// the snake's initial spawn location is in the middle of the map, and its direction to the right
void InitSnake(Snake *snake, int mapWidth, int mapHeight) {
    snake->length = SNAKE_INITIAL_LENGTH;
    snake->body = (Vector2 *)malloc(SNAKE_INITIAL_LENGTH * sizeof(Vector2));
    snake->body[0] = (Vector2){mapWidth / 2, mapHeight / 2};
    snake->direction = (Vector2){1, 0};
}

void InitFood(Food *food) {
    food->position.x = rand() % (mapWidth - 2) + 1;
    food->position.y = rand() % (mapHeight - 2) + 1;
}

void InitBooster(Booster *booster, int type) {
    booster->position.x = rand() % (mapWidth - 2) + 1;
    booster->position.y = rand() % (mapHeight - 2) + 1;
    booster->isActive = true;
    booster->type = type;
}

void MoveSnake(Snake *snake) {
    // Shift snake's body
    for (int i = snake->length - 1; i > 0; i--) {
        snake->body[i] = snake->body[i - 1];
    }

    // Move the head
    snake->body[0].x += snake->direction.x;
    snake->body[0].y += snake->direction.y;
}

/* ------------------------- COLLISIONS HANDLING (Food or booster) -------------------------*/

int CheckCollision(Snake *snake) {
    // Collision with wall
    if (snake->body[0].x < 1 || snake->body[0].x >= mapWidth - 1 || 
        snake->body[0].y < 1 || snake->body[0].y >= mapHeight - 1) {
        return 1;
    }

    // Collision with snake
    for (int i = 1; i < snake->length; i++) {
        if (snake->body[0].x == snake->body[i].x && snake->body[0].y == snake->body[i].y) {
            return 1;
        }
    }

    return 0;
}

int CheckFoodCollision(Snake *snake, Food *food) {
    if (snake->body[0].x == food->position.x && snake->body[0].y == food->position.y) {
        return 1;
    }
    return 0;
}

int CheckBoosterCollision(Snake *snake, Booster *booster) {
    if (booster->isActive && snake->body[0].x == booster->position.x && snake->body[0].y == booster->position.y) {
        booster->isActive = false;
        return 1;
    }
    return 0;
}

/* ------------------------- DRAWING ELEMENTS -------------------------*/

// green snake
void DrawSnake(Snake *snake) {
    for (int i = 0; i < snake->length; i++) {
        DrawRectangle(snake->body[i].x * GRID_CELL_SIZE, 
                      snake->body[i].y * GRID_CELL_SIZE + VERTICAL_OFFSET, 
                      GRID_CELL_SIZE, GRID_CELL_SIZE, GREEN);
    }
}

// white walls
void DrawWalls() {
    for (int x = 0; x < mapWidth; x++) {
        if (x >= 0 && x < mapWidth) {
            DrawRectangle(x * GRID_CELL_SIZE, 0 + VERTICAL_OFFSET, GRID_CELL_SIZE, GRID_CELL_SIZE, RAYWHITE);
            DrawRectangle(x * GRID_CELL_SIZE, (mapHeight - 1) * GRID_CELL_SIZE + VERTICAL_OFFSET, GRID_CELL_SIZE, GRID_CELL_SIZE, RAYWHITE);
        }
    }
    for (int y = 0; y < mapHeight; y++) {
        if (y >= 0 && y < mapHeight) {
            DrawRectangle(0, y * GRID_CELL_SIZE + VERTICAL_OFFSET, GRID_CELL_SIZE, GRID_CELL_SIZE, RAYWHITE);
            DrawRectangle((mapWidth - 1) * GRID_CELL_SIZE, y * GRID_CELL_SIZE + VERTICAL_OFFSET, GRID_CELL_SIZE, GRID_CELL_SIZE, RAYWHITE);
        }
    }
}

void DrawFood(Food *food, Texture2D food_image) {
    Rectangle source = {0.0f, 0.0f, food_image.width, food_image.height};
    Rectangle dest = {
        food->position.x * GRID_CELL_SIZE,
        VERTICAL_OFFSET + food->position.y * GRID_CELL_SIZE,
        GRID_CELL_SIZE,
        GRID_CELL_SIZE
    }; // Position and size
    Vector2 origin = {0.0f, 0.0f};
    DrawTexturePro(food_image, source, dest, origin, 0.0f, WHITE);
}

void DrawBooster(Booster *booster) {
    if (booster->isActive) {
        Color boosterColor;
        if (booster->type == 0) boosterColor = YELLOW; // speed
        else if (booster->type == 1) boosterColor = PURPLE; // size reducer
        else if (booster->type == 2) boosterColor = BLUE; // extra life

        DrawRectangle(booster->position.x * GRID_CELL_SIZE, 
                      booster->position.y * GRID_CELL_SIZE + VERTICAL_OFFSET, 
                      GRID_CELL_SIZE, GRID_CELL_SIZE, boosterColor);
    }
}

void HandleSizeReducer(Snake *snake) {
    if (snake->length > 1) {
        snake->length -= 2; // reduce size by 2
        if (snake->length < 1) { // make sure the snake is always at least 1 square long
            snake->length = 1;
        }
        snake->body = (Vector2 *)realloc(snake->body, snake->length * sizeof(Vector2));
    }
}

void HandleCollision(Snake *snake, bool *isGameRunning) {
    if (CheckCollision(snake)) {
        if (extraLives > 0) {
            extraLives--;

            // Reset snake size to 1 and place it at the center of the map (like at the start)
            snake->length = 1;
            free(snake->body);
            snake->body = (Vector2 *)malloc(sizeof(Vector2));
            snake->body[0] = (Vector2){mapWidth / 2, mapHeight / 2};  
            
            snake->direction = (Vector2){1, 0}; // initially moving to the right
        } else {
            *isGameRunning = false;  // end game if no extra lives
        }
    }
}

void DrawGameOver(Texture2D lost_image) {
    float scale = 0.3f; 
    int scaledWidth = lost_image.width * scale;
    int scaledHeight = lost_image.height * scale;
    DrawText("YOU LOST...", 10, 10, 30, RED);
    DrawText("Press R to restart or ESC to exit.", 10, 40, 20, RAYWHITE);
    DrawTextureEx(lost_image, (Vector2){10, 70}, 0.0f, scale, WHITE);



}

void DrawGameWon(int food, Texture2D won_image) {
    float scale = 0.3f;  
    int scaledWidth = won_image.width * scale;
    int scaledHeight = won_image.height * scale;
    DrawText("YOU WON!", 10, 10, 30, BLUE);
    DrawText(FormatText("Food eaten: %d", food), 10, 40, 30, BLUE);
    DrawText("Press R to restart or ESC to exit.", 10, 70, 20, RAYWHITE);
    DrawTextureEx(won_image, (Vector2){10, 100}, 0.0f, scale, WHITE);

}

void drawMapSizeInfo(Texture2D snake_image) {
    BeginDrawing();
    ClearBackground(BLACK);

    float scale = 0.1f;  
    int scaledWidth = snake_image.width * scale;
    int scaledHeight = snake_image.height * scale;
    DrawTextureEx(snake_image, (Vector2){10, 10}, 0.0f, scale, WHITE);
    
    DrawText("Use arrow keys to change the map size.", 10, 140, 20, RAYWHITE);
    DrawText(FormatText("Width: %i", mapWidth), 10, 170, 20, RAYWHITE);
    DrawText(FormatText("Height: %i", mapHeight), 10, 200, 20, RAYWHITE);
    DrawText("Press ENTER to confirm.", 10, 230, 20, RAYWHITE);

    EndDrawing();
}

void RestartGame(Snake *snake, Food *food, int *foodEaten, int *winCondition, bool *isGameRunning) {
    free(snake->body);
    snake->length = SNAKE_INITIAL_LENGTH;
    snake->body = (Vector2 *)malloc(snake->length * sizeof(Vector2));
    snake->body[0] = (Vector2){mapWidth / 2, mapHeight / 2};
    snake->direction = (Vector2){1, 0};

    InitFood(food);
    *foodEaten = 0;
    *winCondition = (mapWidth - 2) * (mapHeight - 2) / 10;
    *isGameRunning = true;
}

int main(void) {
    InitWindow(800, 600, "Snake de la Hess");
    SetTargetFPS(60);
    
    srand(time(NULL));

    Texture2D lost_image = LoadTexture("../assets/lost.png");
    Texture2D won_image = LoadTexture("../assets/won.png");
    Texture2D snake_image = LoadTexture("../assets/snake.png");
    Texture2D food_image = LoadTexture("../assets/fruit_peach.png");

    InitAudioDevice();              
    Music music = LoadMusicStream("../assets/dgrp.mp3");

    float lastMoveTime = 0.0f;  // Timer for snake movement
    int foodEaten = 0;
    int winCondition;
    bool isGameRunning = true;

    // map size customization screen
    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_UP) && mapHeight < MAX_SIZE) {   // Increase height
            mapHeight++;
        }
        if (IsKeyPressed(KEY_DOWN) && mapHeight > MIN_SIZE) { // Decrease height
            mapHeight--;
        }
        if (IsKeyPressed(KEY_RIGHT) && mapWidth < MAX_SIZE) {  // Increase width
            mapWidth++;
        }
        if (IsKeyPressed(KEY_LEFT) && mapWidth > MIN_SIZE) {   // Decrease width
            mapWidth--;
        }

        drawMapSizeInfo(snake_image);
        
        if (IsKeyPressed(KEY_ENTER)) {
            mapWidth += 2;
            mapHeight += 2;
            winCondition = (mapWidth - 2) * (mapHeight - 2) / 10; // the number of food eaten to win is calculated from the map size
            break;
        }
    }

    Snake snake;
    InitSnake(&snake, mapWidth, mapHeight);

    Food food;
    InitFood(&food);

    Booster currentBooster;
    InitBooster(&currentBooster, rand()%3);
    float boosterSpawnTimer = 0.0f; // this will be used to spawn new boosters after 10 seconds (everytime player eats one)

    // game start
    while (!WindowShouldClose()) {
        // start the music whenever the game starts (when map is generated i mean)
        PlayMusicStream(music);

        // play music continuously until the window is closed
        UpdateMusicStream(music);

        if (!isGameRunning) {
            // if game is lost or won, stop music
            StopMusicStream(music);

            // exit game
            if (IsKeyPressed(KEY_ESCAPE)) {
                break;
            }
            if (IsKeyPressed(KEY_R)) {
                RestartGame(&snake, &food, &foodEaten, &winCondition, &isGameRunning);
            }
            BeginDrawing();
            ClearBackground(BLACK);
            if (foodEaten >= winCondition) {
                DrawGameWon(foodEaten, won_image);
            } else {
                DrawGameOver(lost_image);
            }
            EndDrawing();
            continue;
        }

        float currentTime = GetTime();
        if (currentTime - lastMoveTime >= currentSpeed) {
            MoveSnake(&snake);
            lastMoveTime = currentTime;
        }

        // snake movement with WASD
        if (IsKeyPressed(KEY_W)) snake.direction = (Vector2){0, -1}; // up
        if (IsKeyPressed(KEY_A)) snake.direction = (Vector2){-1, 0}; // left
        if (IsKeyPressed(KEY_S)) snake.direction = (Vector2){0, 1}; // down
        if (IsKeyPressed(KEY_D)) snake.direction = (Vector2){1, 0}; // right

        HandleCollision(&snake, &isGameRunning);

        if (CheckFoodCollision(&snake, &food)) {
            snake.length++;
            snake.body = (Vector2 *)realloc(snake.body, snake.length * sizeof(Vector2));
            foodEaten++;

            if (foodEaten >= winCondition) {
                isGameRunning = false;
            }

            // place new food at random location
            InitFood(&food);
        }

        float deltaTime = GetFrameTime();
        boosterSpawnTimer += deltaTime;
        
        if (CheckBoosterCollision(&snake, &currentBooster)) {
            boosterSpawnTimer = 0.0f; // Reset spawn timer when booster is eaten
            currentBooster.isActive = false; // Deactivate booster
            if (currentBooster.type == 0) {
                boosterTimer = speedDuration;
                currentSpeed = boostedSpeed;
            } else if (currentBooster.type == 1) {
                HandleSizeReducer(&snake);
            } else if (currentBooster.type == 2) {
                extraLives++;
            }
        }

        if (boosterTimer > 0.0f) {
            boosterTimer -= deltaTime;
            if (boosterTimer <= 0.0f) {
                currentSpeed = normalSpeed;
            }
        }

        // Spawn a new booster if timer exceeds the respawn time
        if (!currentBooster.isActive && boosterSpawnTimer >= BOOSTER_RESPAWN_TIME) {
            InitBooster(&currentBooster, rand() % 2); // Randomly choose between types 0 and 1
            boosterSpawnTimer = 0.0f; // Reset timer
        }

        // Draw the game screen
        BeginDrawing();
        ClearBackground(BLACK);

        DrawWalls();

        DrawSnake(&snake);
        DrawFood(&food, food_image);
        DrawBooster(&currentBooster);

        DrawText(FormatText("Food Eaten: %d/%d", foodEaten, winCondition), 10, 10, 20, RAYWHITE);
        DrawText(FormatText("Lives: %d", extraLives+1), 10, 40, 20, BLUE);
        

        // Write Speed booster text if the user has ate it
        if (boosterTimer > 0.0f) {
            DrawText(FormatText("SPEED BOOST!!! %.2f", boosterTimer), 10, 70, 20, YELLOW);
        }

        EndDrawing();
    }

    //unloading pics
    UnloadTexture(lost_image);  
    UnloadTexture(won_image);  
    UnloadTexture(snake_image);
    UnloadTexture(food_image);

    // unloading music
    UnloadMusicStream(music);
    CloseAudioDevice();

    free(snake.body);
    CloseWindow();

    return 0;
}