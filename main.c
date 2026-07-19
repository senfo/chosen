#include "raylib.h"

#define FRAME_SIZE 16
#define FRAMES_PER_ROW 6

#define JUMP_SOUND "audio/jump.wav"

typedef enum {
    WALK_DOWN = 0,
    RUN_RIGHT = 1,
    RUN_LEFT  = 2,
    WALK_UP   = 3
} MainCharacterRow;

const float SPEED = 250.0f;
const float GRAVITY = 1200.0f;
const float JUMP_FORCE = 500.0f;
const float GROUND_Y = 350.0f;
const float SIZE = 32.0f;

int main(void) {
    short currentCharacterFrame = 0; // 0-5
    float playerX = 100;
    float playerY = 350;
    float velocityY = 0;
    bool onGround = false;
    MainCharacterRow spriteRow = RUN_RIGHT;
    Rectangle source = {
        (float)(currentCharacterFrame * FRAME_SIZE),
        (float)(spriteRow * FRAME_SIZE),
        (float)FRAME_SIZE,
        (float)FRAME_SIZE
    };

    InitWindow(800, 450, "raylib test");
    InitAudioDevice();
    SetTargetFPS(60);

    Texture2D playerSheet = LoadTexture("sprites/BaseCharacter1.png");
    Sound jump = LoadSound(JUMP_SOUND);

    while (!WindowShouldClose()) {
        // --- Update ---
        if (IsKeyDown(KEY_RIGHT)) {
            playerX += SPEED * GetFrameTime(); 
        }

        if (IsKeyDown(KEY_LEFT)) {
            playerX -= SPEED * GetFrameTime();
        }

        if (IsKeyPressed(KEY_SPACE) && onGround) {
            velocityY = -JUMP_FORCE;
            onGround = false;

            PlaySound(jump);
        }

        velocityY += GRAVITY * GetFrameTime();
        playerY += velocityY * GetFrameTime();

        if (playerY >= GROUND_Y) {
            playerY = GROUND_Y;
            velocityY = 0;
            onGround = true;
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawLine(0, (int)GROUND_Y + (int)SIZE, 800, (int)GROUND_Y + (int)SIZE, DARKGRAY);
        DrawRectangle((int)playerX, (int)playerY, (int)SIZE, (int)SIZE, MAROON);

        EndDrawing();
    }

    UnloadSound(jump);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}

