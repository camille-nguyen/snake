#include "raylib.h"
#include <stdlib.h>
#include <time.h>

#define MIN_SIZE 5
#define MAX_SIZE 20
#define SNAKE_INITIAL_LENGTH 1
#define GRID_CELL_SIZE 20
#define MOVEMENT_INTERVAL 1.0f // 1 second interval for moving the snake

int mapWidth = MIN_SIZE, mapHeight = MIN_SIZE;  // User-defined map size

// Define the Snake structure
typedef struct Snake {
    Vector2 *body;         // Array to store the positions of the snake segments
    int length;            // Snake's current length
    Vector2 direction;     // Current direction of the snake
} Snake;

void InitSnake(Snake *snake, int mapWidth, int mapHeight) {
    snake->length = SNAKE_INITIAL_LENGTH;
    snake->body = (Vector2 *)malloc(SNAKE_INITIAL_LENGTH * sizeof(Vector2));
    snake->body[0] = (Vector2){mapWidth / 2, mapHeight / 2}; // Start in the middle of the map
    snake->direction = (Vector2){1, 0}; // Initially moving to the right
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

void DrawSnake(Snake *snake) {
    for (int i = 0; i < snake->length; i++) {
        DrawRectangle(snake->body[i].x * GRID_CELL_SIZE, snake->body[i].y * GRID_CELL_SIZE, 
                      GRID_CELL_SIZE, GRID_CELL_SIZE, GREEN);
    }
}

void DrawWalls() {
    // Draw the black walls around the edges of the grid with a 1-cell gap
    for (int x = 0; x < mapWidth; x++) {
        if (x > 0 && x < mapWidth - 1) {
            // Top wall (1 cell gap)
            DrawRectangle(x * GRID_CELL_SIZE, 0, GRID_CELL_SIZE, GRID_CELL_SIZE, BLACK);
            // Bottom wall (1 cell gap)
            DrawRectangle(x * GRID_CELL_SIZE, (mapHeight - 1) * GRID_CELL_SIZE, GRID_CELL_SIZE, GRID_CELL_SIZE, BLACK);
        }
    }

    for (int y = 0; y < mapHeight; y++) {
        if (y > 0 && y < mapHeight - 1) {
            // Left wall (1 cell gap)
            DrawRectangle(0, y * GRID_CELL_SIZE, GRID_CELL_SIZE, GRID_CELL_SIZE, BLACK);
            // Right wall (1 cell gap)
            DrawRectangle((mapWidth - 1) * GRID_CELL_SIZE, y * GRID_CELL_SIZE, GRID_CELL_SIZE, GRID_CELL_SIZE, BLACK);
        }
    }
}

void DrawGameOver() {
    DrawText("GAME OVER!", 10, mapHeight * GRID_CELL_SIZE / 2, 30, RED);
    DrawText("Press ESC to exit", 10, mapHeight * GRID_CELL_SIZE / 2 + 40, 20, BLACK);
}

void drawMapSizeInfo() {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    
    // Display instructions and current map size
    DrawText("Use arrow keys to change the map size.", 10, 10, 20, BLACK);
    DrawText(FormatText("Width: %i", mapWidth), 10, 40, 20, BLACK);
    DrawText(FormatText("Height: %i", mapHeight), 10, 70, 20, BLACK);
    DrawText("Press ENTER to confirm.", 10, 100, 20, BLACK);
    
    EndDrawing();
}

int main(void) {
    // Initialization
    InitWindow(800, 600, "Snake Game");
    SetTargetFPS(60); // Set the game FPS to 60
    
    float lastMoveTime = 0.0f;  // Timer for snake movement

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
            mapWidth+=2;
            mapHeight+=2;
            break;
        }
    }

    // Initialize Snake
    Snake snake;
    InitSnake(&snake, mapWidth, mapHeight);

    // Start the game loop
    while (!WindowShouldClose()) {
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
            BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawGameOver();
            EndDrawing();
            if (IsKeyPressed(KEY_ESCAPE)) {
                break; // Exit the game on ESC press
            }
            continue; // Skip the rest of the game loop after game over
        }

        // Draw the game screen
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Draw Walls
        DrawWalls();

        // Draw Snake
        DrawSnake(&snake);

        EndDrawing();
    }

    // De-Initialization
    free(snake.body);
    CloseWindow(); // Close window and OpenGL context

    return 0;
}
