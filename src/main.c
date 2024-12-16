#include "raylib.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#define MIN_SIZE 5
#define MAX_SIZE 20
#define SNAKE_INITIAL_LENGTH 1
#define GRID_CELL_SIZE 20
#define MOVEMENT_INTERVAL 0.5f
#define VERTICAL_OFFSET 110
#define BOOSTER_RESPAWN_TIME 6.0f

/* ------------------------- STRUCTURES ------------------------- */

typedef struct Snake {
    Vector2 *body;
    int length;
    Vector2 direction;
} Snake;

typedef struct Booster {
    Vector2 position;
    bool isActive;
    int type; // (0 = speed, 1 = size reducer, 2 = extra life)
} Booster;

typedef struct Letter {
    Vector2 position;
    char value;
} Letter;
Letter letter1, letter2;

typedef struct GameState {
    int mapWidth;
    int mapHeight;
    float normalSpeed;
    float boostedSpeed;
    float speedDuration;
    float boosterTimer;
    float currentSpeed;
    int extraLives;
    const char *wordList[3];
    char currentWord[20];
    char guessedWord[20];
    int currentWordLength;
    char letterChoices[2];
    Letter letter1;
    Letter letter2;
} GameState;

/* ------------------------- INIT ------------------------- */

// the snake's initial spawn location is in the middle of the map, and its direction to the right
void InitSnake(Snake *snake, GameState *gameState) {
    snake->length = SNAKE_INITIAL_LENGTH;
    snake->body = (Vector2 *)malloc(SNAKE_INITIAL_LENGTH * sizeof(Vector2));
    snake->body[0] = (Vector2){gameState->mapWidth / 2, gameState->mapHeight / 2};
    snake->direction = (Vector2){1, 0};
}

void InitBooster(Booster *booster, int type, GameState *gameState) {
    booster->position.x = rand() % (gameState->mapWidth - 2) + 1;
    booster->position.y = rand() % (gameState->mapHeight - 2) + 1;
    booster->isActive = true;
    booster->type = type;
}

void MoveSnake(Snake *snake) {
    for (int i = snake->length - 1; i > 0; i--) {
        snake->body[i] = snake->body[i - 1];
    }

    snake->body[0].x += snake->direction.x;
    snake->body[0].y += snake->direction.y;
}

// Initialize the word for the current game
void InitWordGame(GameState *gameState) {
    int randomIndex = rand() % (sizeof(gameState->wordList)/sizeof(gameState->wordList[0]));
    strcpy(gameState->currentWord, gameState->wordList[randomIndex]);
    gameState->currentWordLength = strlen(gameState->currentWord);
    for (int i = 0; i < gameState->currentWordLength; i++) {
        gameState->guessedWord[i] = '_';
    }
    gameState->guessedWord[gameState->currentWordLength] = '\0';
}

// Generate two random letters: one correct and one incorrect
void GenerateLetterChoices(GameState *gameState) {
    // letter 1 : contained in the word
    char correctLetter;
    int found = 0;
    for (int i = 0; i < gameState->currentWordLength; i++) {
        if (gameState->guessedWord[i] == '_') {
            correctLetter = gameState->currentWord[i];
            found = 1;
            break;
        }
    }

    if (!found) {
        return;
    }

    gameState->letterChoices[0] = correctLetter;

    // letter 2: not contained in word
    do {
        gameState->letterChoices[1] = 'a' + (rand() % 26);
    } while (strchr(gameState->currentWord, gameState->letterChoices[1])); // Ensure it’s not in the word

    if (rand() % 2 == 0) {
        char temp = gameState->letterChoices[0];
        gameState->letterChoices[0] = gameState->letterChoices[1];
        gameState->letterChoices[1] = temp;
    }

    letter1.position = (Vector2){rand() % (gameState->mapWidth - 2) + 1, rand() % (gameState->mapHeight - 2) + 1};
    letter2.position = (Vector2){rand() % (gameState->mapWidth - 2) + 1, rand() % (gameState->mapHeight - 2) + 1};
    letter1.value = gameState->letterChoices[0];
    letter2.value = gameState->letterChoices[1];
}

/* ------------------------- COLLISIONS HANDLING (wall/snake, letter/booster) -------------------------*/

int CheckCollision(Snake *snake, GameState *gameState) {
    // collision with wall
    if (snake->body[0].x < 1 || snake->body[0].x >= gameState->mapWidth - 1 || 
        snake->body[0].y < 1 || snake->body[0].y >= gameState->mapHeight - 1) {
        return 1;
    }

    //collision with snake
    for (int i = 1; i < snake->length; i++) {
        if (snake->body[0].x == snake->body[i].x && snake->body[0].y == snake->body[i].y) {
            return 1;
        }
    }

    return 0;
}

int CheckLetterCollision(Snake *snake, Letter *letter) {
    return (snake->body[0].x == letter->position.x && snake->body[0].y == letter->position.y);
}

int CheckBoosterCollision(Snake *snake, Booster *booster, Sound boosterSound) {
    if (booster->isActive && snake->body[0].x == booster->position.x && snake->body[0].y == booster->position.y) {
        booster->isActive = false;
        PlaySound(boosterSound);
        return 1;
    }
    return 0;
}

/* ------------------------- GAME LOGIC -------------------------*/

void HandleLetterCollision(Snake *snake, GameState *gameState, Letter *letter, Sound eatSound) {
    char chosenLetter = letter->value;

    // check if the eaten letter is in the word
    int found = 0;
    for (int i = 0; i < gameState->currentWordLength; i++) {
        if (gameState->currentWord[i] == chosenLetter && gameState->guessedWord[i] == '_') {
            gameState->guessedWord[i] = chosenLetter; // Update guessed word
            found = 1;
        }
    }

    // if the letter is incorrect or already guessed
    if (!found) {
        gameState->extraLives--;
        if (gameState->extraLives < 0) {
            gameState->extraLives = 0;
        }
    } else {
        PlaySound(eatSound);
    }

    snake->length += 1; 
    snake->body = (Vector2 *)realloc(snake->body, snake->length * sizeof(Vector2));

    GenerateLetterChoices(gameState);
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

void HandleCollision(Snake *snake, GameState *gameState, bool *isGameRunning) {
    if (CheckCollision(snake, gameState)) {
        if (gameState->extraLives > 0) {
            gameState->extraLives--;

            // Reset snake size to 1 and place it at the center of the map (like at the start)
            snake->length = 1;
            free(snake->body);
            snake->body = (Vector2 *)malloc(sizeof(Vector2));
            snake->body[0] = (Vector2){gameState->mapWidth / 2, gameState->mapHeight / 2};  
            
            snake->direction = (Vector2){1, 0}; // initially moving to the right
        } else {
            *isGameRunning = false;  // end game if no extra lives
        }
    }
}

void RestartGame(Snake *snake, GameState *gameState, int *currentWordLength, bool *isGameRunning) {
    free(snake->body);
    snake->length = SNAKE_INITIAL_LENGTH;
    snake->body = (Vector2 *)malloc(snake->length * sizeof(Vector2));
    snake->body[0] = (Vector2){gameState->mapWidth / 2, gameState->mapHeight / 2};
    snake->direction = (Vector2){1, 0};

    InitWordGame(gameState);
    GenerateLetterChoices(gameState);
    *currentWordLength = 0;
    *isGameRunning = true;
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
void DrawWalls(GameState *gameState) {
    for (int x = 0; x < gameState->mapWidth; x++) {
        if (x >= 0 && x < gameState->mapWidth) {
            DrawRectangle(x * GRID_CELL_SIZE, 0 + VERTICAL_OFFSET, GRID_CELL_SIZE, GRID_CELL_SIZE, RAYWHITE);
            DrawRectangle(x * GRID_CELL_SIZE, (gameState->mapHeight - 1) * GRID_CELL_SIZE + VERTICAL_OFFSET, GRID_CELL_SIZE, GRID_CELL_SIZE, RAYWHITE);
        }
    }
    for (int y = 0; y < gameState->mapHeight; y++) {
        if (y >= 0 && y < gameState->mapHeight) {
            DrawRectangle(0, y * GRID_CELL_SIZE + VERTICAL_OFFSET, GRID_CELL_SIZE, GRID_CELL_SIZE, RAYWHITE);
            DrawRectangle((gameState->mapWidth - 1) * GRID_CELL_SIZE, y * GRID_CELL_SIZE + VERTICAL_OFFSET, GRID_CELL_SIZE, GRID_CELL_SIZE, RAYWHITE);
        }
    }
}

// booster
// speed = yellow, size reduce = purple, blue = extra life
void DrawBooster(Booster *booster) {
    if (booster->isActive) {
        Color boosterColor;
        if (booster->type == 0) boosterColor = YELLOW;
        else if (booster->type == 1) boosterColor = PURPLE;
        else if (booster->type == 2) boosterColor = BLUE;

        DrawRectangle(booster->position.x * GRID_CELL_SIZE, 
                      booster->position.y * GRID_CELL_SIZE + VERTICAL_OFFSET, 
                      GRID_CELL_SIZE, GRID_CELL_SIZE, boosterColor);
    }
}

void DrawGuessedWord(GameState *gameState) {
    DrawText(FormatText("Word: %s", gameState->guessedWord), 10, 40, 20, YELLOW);
}

void DrawLetters(Letter *letter1, Letter *letter2) {
    DrawRectangle(letter1->position.x * GRID_CELL_SIZE,
                  letter1->position.y * GRID_CELL_SIZE + VERTICAL_OFFSET,
                  GRID_CELL_SIZE, GRID_CELL_SIZE, RED);
    DrawText(&letter1->value, letter1->position.x * GRID_CELL_SIZE + 5,
             letter1->position.y * GRID_CELL_SIZE + VERTICAL_OFFSET -2, 20, WHITE);

    DrawRectangle(letter2->position.x * GRID_CELL_SIZE,
                  letter2->position.y * GRID_CELL_SIZE + VERTICAL_OFFSET,
                  GRID_CELL_SIZE, GRID_CELL_SIZE, RED);
    DrawText(&letter2->value, letter2->position.x * GRID_CELL_SIZE + 5,
             letter2->position.y * GRID_CELL_SIZE + VERTICAL_OFFSET - 2, 20, WHITE);
}

void DrawGameOver(Texture2D lost_image) {
    float scale = 0.3f; 
    int scaledWidth = lost_image.width * scale;
    int scaledHeight = lost_image.height * scale;
    DrawText("YOU LOST...", 10, 10, 30, RED);
    DrawText("Press R to restart or ESC to exit.", 10, 40, 20, RAYWHITE);
    DrawTextureEx(lost_image, (Vector2){10, 70}, 0.0f, scale, WHITE);

}

void DrawGameWon(char *currentWord, Texture2D won_image) {
    float scale = 0.3f;  
    int scaledWidth = won_image.width * scale;
    int scaledHeight = won_image.height * scale;
    DrawText("YOU WON!", 10, 10, 30, BLUE);
    DrawText(FormatText("Word found: %s", currentWord), 10, 40, 25, BLUE);
    DrawText("Press R to restart or ESC to exit.", 10, 70, 20, RAYWHITE);
    DrawTextureEx(won_image, (Vector2){10, 100}, 0.0f, scale, WHITE);

}

void drawMapSizeInfo(Texture2D snake_image, GameState *gameState) {
    BeginDrawing();
    ClearBackground(BLACK);

    float scale = 0.1f;  
    int scaledWidth = snake_image.width * scale;
    int scaledHeight = snake_image.height * scale;
    DrawTextureEx(snake_image, (Vector2){10, 10}, 0.0f, scale, WHITE);
    
    DrawText("Use arrow keys to change the map size.", 10, 140, 20, RAYWHITE);
    DrawText(FormatText("Width: %i", gameState->mapWidth), 10, 170, 20, RAYWHITE);
    DrawText(FormatText("Height: %i", gameState->mapHeight), 10, 200, 20, RAYWHITE);
    DrawText("Press ENTER to confirm.", 10, 230, 20, RAYWHITE);

    EndDrawing();
}

/* ------------------------- MAIN GAME -------------------------*/

int main(void) {
    InitWindow(800, 600, "Snake de la Hess");
    SetTargetFPS(60);
    
    srand(time(NULL));

    /* LOADING ASSETS */
    Texture2D lost_image = LoadTexture("../assets/lost.png");
    Texture2D won_image = LoadTexture("../assets/won.png");
    Texture2D snake_image = LoadTexture("../assets/snake.png");
    InitAudioDevice();              
    Music music = LoadMusicStream("../assets/mysims.mp3");
    Sound eatSound = LoadSound("../assets/fruit.mp3");
    Sound boosterSound = LoadSound("../assets/booster.mp3");


    GameState gameState = {
        .wordList = {"ccu", "pineapple", "taiwan"}, // if wanna add more words, change size in struct
        .mapWidth = MIN_SIZE,
        .mapHeight = MIN_SIZE,
        .normalSpeed = 0.5f,
        .boostedSpeed = 0.25f,
        .speedDuration = 5.0f,
        .boosterTimer = 0.0f,
        .currentSpeed = 0.5f,
        .extraLives = 0,
        .currentWordLength = 0

    };
    float lastMoveTime = 0.0f;
    int foodEaten = 0;
    InitWordGame(&gameState);
    GenerateLetterChoices(&gameState);
    bool isGameRunning = true;

    // map size customization screen
    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_UP) && gameState.mapHeight < MAX_SIZE) {
            gameState.mapHeight++;
        }
        if (IsKeyPressed(KEY_DOWN) && gameState.mapHeight > MIN_SIZE) {
            gameState.mapHeight--;
        }
        if (IsKeyPressed(KEY_RIGHT) && gameState.mapWidth < MAX_SIZE) {
            gameState.mapWidth++;
        }
        if (IsKeyPressed(KEY_LEFT) && gameState.mapWidth > MIN_SIZE) {
            gameState.mapWidth--;
        }

        drawMapSizeInfo(snake_image, &gameState);
        
        if (IsKeyPressed(KEY_ENTER)) {
            gameState.mapWidth += 2;
            gameState.mapHeight += 2;
            break;
        }
    }

    Snake snake;
    InitSnake(&snake, &gameState);

    Booster currentBooster;
    InitBooster(&currentBooster, rand() % 3, &gameState);
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
                RestartGame(&snake, &gameState, &gameState.currentWordLength, &isGameRunning);
                InitWordGame(&gameState);
                GenerateLetterChoices(&gameState);
            }
            BeginDrawing();
            ClearBackground(BLACK);
            if (strcmp(gameState.currentWord, gameState.guessedWord) == 0) {
                DrawGameWon(gameState.currentWord, won_image);
            } else {
                DrawGameOver(lost_image);
            }
            EndDrawing();
            continue;
        }

        float currentTime = GetTime();
        if (currentTime - lastMoveTime >= gameState.currentSpeed) {
            MoveSnake(&snake);
            lastMoveTime = currentTime;
        }

        // Handle letter collisions
        if (CheckLetterCollision(&snake, &letter1)) {
            HandleLetterCollision(&snake, &gameState, &letter1, eatSound);
            if (strcmp(gameState.currentWord, gameState.guessedWord) == 0) {
                isGameRunning = false;
            }
        } else if (CheckLetterCollision(&snake, &letter2)) {
            HandleLetterCollision(&snake, &gameState, &letter2, eatSound);
            if (strcmp(gameState.currentWord, gameState.guessedWord) == 0) {
                isGameRunning = false;
            }
        }

        // snake movement with WASD
        if (IsKeyPressed(KEY_W)) snake.direction = (Vector2){0, -1}; // up
        if (IsKeyPressed(KEY_A)) snake.direction = (Vector2){-1, 0}; // left
        if (IsKeyPressed(KEY_S)) snake.direction = (Vector2){0, 1}; // down
        if (IsKeyPressed(KEY_D)) snake.direction = (Vector2){1, 0}; // right

        HandleCollision(&snake,&gameState, &isGameRunning);

        float deltaTime = GetFrameTime();
        boosterSpawnTimer += deltaTime;
        
        if (CheckBoosterCollision(&snake, &currentBooster, boosterSound)) {
            boosterSpawnTimer = 0.0f;
            currentBooster.isActive = false;
            if (currentBooster.type == 0) {
                gameState.boosterTimer = gameState.speedDuration;
                gameState.currentSpeed = gameState.boostedSpeed;
            } else if (currentBooster.type == 1) {
                HandleSizeReducer(&snake);
            } else if (currentBooster.type == 2) {
                gameState.extraLives++;
            }
        }

        if (gameState.boosterTimer > 0.0f) {
            gameState.boosterTimer -= deltaTime;
            if (gameState.boosterTimer <= 0.0f) {
                gameState.currentSpeed = gameState.normalSpeed;
            }
        }

        // Spawn a new booster if timer exceeds the respawn time
        if (!currentBooster.isActive && boosterSpawnTimer >= BOOSTER_RESPAWN_TIME) {
            InitBooster(&currentBooster, rand() % 3, &gameState);
            boosterSpawnTimer = 0.0f;
        }

        BeginDrawing();
        ClearBackground(BLACK);

        DrawWalls(&gameState);

        DrawSnake(&snake);
        DrawLetters(&letter1, &letter2);
        DrawGuessedWord(&gameState);

        DrawBooster(&currentBooster);

        DrawText(FormatText("Lives: %d", gameState.extraLives+1), 10, 10, 20, RAYWHITE);
        

        // Write Speed booster text if the user has ate it
        if (gameState.boosterTimer > 0.0f) {
            DrawText(FormatText("SPEED BOOST!!! %.2f", gameState.boosterTimer), 10, 70, 20, YELLOW);
        }

        EndDrawing();
        
    }

    //unloading pics
    UnloadTexture(lost_image);  
    UnloadTexture(won_image);  
    UnloadTexture(snake_image);

    UnloadMusicStream(music);
    UnloadSound(eatSound);
    UnloadSound(boosterSound);

    CloseAudioDevice();

    free(snake.body);
    CloseWindow();

    return 0;
}