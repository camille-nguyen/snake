#include "raylib.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define MIN_SIZE 5
#define MAX_SIZE 20
#define SNAKE_INITIAL_LENGTH 1
#define GRID_CELL_SIZE 20
#define MOVEMENT_INTERVAL 0.5f // 1 second interval for moving the snake

int mapWidth = MIN_SIZE, mapHeight = MIN_SIZE;  // User-defined map size

// Define the Snake structure
typedef struct Snake {
    Vector2 *body;         // Array to store the positions of the snake segments
    int length;            // Snake's current length
    Vector2 direction;     // Current direction of the snake
} Snake;

// Define the Food structure
typedef struct Food {
    Vector2 position;  // Position of the food
} Food;

void InitSnake(Snake *snake, int mapWidth, int mapHeight) {
    snake->length = SNAKE_INITIAL_LENGTH;
    snake->body = (Vector2 *)malloc(SNAKE_INITIAL_LENGTH * sizeof(Vector2));
    snake->body[0] = (Vector2){mapWidth / 2, mapHeight / 2}; // Start in the middle of the map
    snake->direction = (Vector2){1, 0}; // Initially moving to the right
}

void InitFood(Food *food) {
    food->position.x = rand() % (mapWidth - 2) + 1;  // Random x position inside the grid
    food->position.y = rand() % (mapHeight - 2) + 1; // Random y position inside the grid
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

int CheckCollision(Snake *snake) {
    // Check if snake collides with the wall
    if (snake->body[0].x < 1 || snake->body[0].x >= mapWidth - 1 || 
        snake->body[0].y < 1 || snake->body[0].y >= mapHeight - 1) {
        return 1; // Collision with wall
    }

    // Check if snake collides with itself
    for (int i = 1; i < snake->length; i++) {
        if (snake->body[0].x == snake->body[i].x && snake->body[0].y == snake->body[i].y) {
            return 1; // Collision with self
        }
    }

    return 0; // No collision
}

int CheckFoodCollision(Snake *snake, Food *food) {
    // Check if snake eats food
    if (snake->body[0].x == food->position.x && snake->body[0].y == food->position.y) {
        return 1; // Food eaten
    }
    return 0; // No food eaten
}

#define VERTICAL_OFFSET 40  // Adjust as needed

void DrawSnake(Snake *snake) {
    for (int i = 0; i < snake->length; i++) {
        DrawRectangle(snake->body[i].x * GRID_CELL_SIZE, 
                      snake->body[i].y * GRID_CELL_SIZE + VERTICAL_OFFSET, 
                      GRID_CELL_SIZE, GRID_CELL_SIZE, GREEN);
    }
}

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

void DrawFood(Food *food) {
    DrawRectangle(food->position.x * GRID_CELL_SIZE, 
                  food->position.y * GRID_CELL_SIZE + VERTICAL_OFFSET, 
                  GRID_CELL_SIZE, GRID_CELL_SIZE, PINK);
}

void DrawGameOver(Texture2D lost_image) {
    float scale = 0.3f; 
    int scaledWidth = lost_image.width * scale;
    int scaledHeight = lost_image.height * scale;
    DrawText("YOU LOST...", 10, mapHeight * GRID_CELL_SIZE / 2, 30, RED);
    DrawText("Press R to restart or ESC to exit.", 10, mapHeight * GRID_CELL_SIZE / 2 + 40, 20, RAYWHITE);
    DrawTextureEx(lost_image, (Vector2){10, (GetScreenHeight() - scaledHeight) / 2 + 80}, 0.0f, scale, WHITE);



}

void DrawGameWon(int food, Texture2D won_image) {
    float scale = 0.3f;  
    int scaledWidth = won_image.width * scale;
    int scaledHeight = won_image.height * scale;
    DrawText("YOU WON!", 10, mapHeight * GRID_CELL_SIZE / 2, 30, BLUE);
    DrawText(FormatText("Food eaten: %d", food), 10, mapHeight * GRID_CELL_SIZE / 2 + 40, 30, BLUE);
    DrawText("Press R to restart or ESC to exit.", 10, mapHeight * GRID_CELL_SIZE / 2 + 80, 20, RAYWHITE);
    DrawTextureEx(won_image, (Vector2){10, (GetScreenHeight() - scaledHeight) / 2 + 80}, 0.0f, scale, WHITE);

}

void drawMapSizeInfo() {
    BeginDrawing();
    ClearBackground(BLACK);
    
    // Display instructions and current map size
    DrawText("Use arrow keys to change the map size.", 10, 10, 20, RAYWHITE);
    DrawText(FormatText("Width: %i", mapWidth), 10, 40, 20, RAYWHITE);
    DrawText(FormatText("Height: %i", mapHeight), 10, 70, 20, RAYWHITE);
    DrawText("Press ENTER to confirm.", 10, 100, 20, RAYWHITE);
    
    EndDrawing();
}

void RestartGame(Snake *snake, Food *food, int *foodEaten, int *winCondition, bool *isGameRunning) {
    // Free the snake's body and reinitialize the game state
    free(snake->body);
    snake->length = SNAKE_INITIAL_LENGTH;
    snake->body = (Vector2 *)malloc(snake->length * sizeof(Vector2));
    snake->body[0] = (Vector2){mapWidth / 2, mapHeight / 2};
    snake->direction = (Vector2){1, 0}; // Moving to the right again

    InitFood(food);  // Reinitialize food
    *foodEaten = 0;  // Reset food eaten count
    *winCondition = (mapWidth - 2) * (mapHeight - 2) / 10; // Set win condition
    *isGameRunning = true;  // Restart the game
}

int main(void) {
    // Initialization
    InitWindow(800, 600, "Snake Game");
    SetTargetFPS(60); // Set the game FPS to 60
    
    srand(time(NULL));  // Seed the random number generator

    Texture2D lost_image = LoadTexture("lost.png");
    Texture2D won_image = LoadTexture("won.png");


    float lastMoveTime = 0.0f;  // Timer for snake movement
    int foodEaten = 0;
    int winCondition;
    bool isGameRunning = true;

    // Main menu for setting map size
    while (!WindowShouldClose()) {
        // Handle user input for changing map size
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

        // Draw the current map size
        drawMapSizeInfo();
        
        // Start the game once the user presses ENTER
        if (IsKeyPressed(KEY_ENTER)) {
            mapWidth += 2;
            mapHeight += 2;
            winCondition = (mapWidth - 2) * (mapHeight - 2) / 10; // Set win condition
            break;
        }
    }

    // Initialize Snake
    Snake snake;
    InitSnake(&snake, mapWidth, mapHeight);

    // Initialize Food
    Food food;
    InitFood(&food);

    // Start the game loop
    while (!WindowShouldClose()) {
        if (!isGameRunning) {
            if (IsKeyPressed(KEY_ESCAPE)) {
                break; // Exit the game
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

        // Check if 1 second has passed for snake movement
        float currentTime = GetTime();
        if (currentTime - lastMoveTime >= MOVEMENT_INTERVAL) {
            // Move the snake once every second
            MoveSnake(&snake);
            lastMoveTime = currentTime;  // Update last move time
        }

        // Input for movement
        if (IsKeyPressed(KEY_W)) snake.direction = (Vector2){0, -1}; // Up
        if (IsKeyPressed(KEY_A)) snake.direction = (Vector2){-1, 0}; // Left
        if (IsKeyPressed(KEY_S)) snake.direction = (Vector2){0, 1};  // Down
        if (IsKeyPressed(KEY_D)) snake.direction = (Vector2){1, 0};   // Right

        // Check for collisions
        if (CheckCollision(&snake)) {
            isGameRunning = false;
        }

        // Check if snake eats food
        if (CheckFoodCollision(&snake, &food)) {
            // Increase snake length
            snake.length++;
            snake.body = (Vector2 *)realloc(snake.body, snake.length * sizeof(Vector2));
            foodEaten++;

            // Check if the game is won
            if (foodEaten >= winCondition) {
                isGameRunning = false;
            }

            // Place new food at random location
            InitFood(&food);
        }

        // Draw the game screen
        BeginDrawing();
        ClearBackground(BLACK);

        // Draw Walls
        DrawWalls();

        // Draw Snake
        DrawSnake(&snake);

        // Draw Food
        DrawFood(&food);

        DrawText(FormatText("Food Eaten: %d/%d", foodEaten, winCondition), 10, 10, 20, RAYWHITE);

        EndDrawing();
    }

    //unloading pics
    UnloadTexture(lost_image);  
    UnloadTexture(won_image);  

    // De-Initialization
    free(snake.body);
    CloseWindow(); // Close window and OpenGL context

    return 0;
}
