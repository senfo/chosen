#include "raylib.h"

#define JUMP_SOUND "audio/jump.wav"
#define SPRITE_SHEET "sprites/BaseCharacter1.png"

#define FRAME_SIZE 16
#define FRAMES_PER_ROW 6
#define SPRITE_SCALE 2

typedef enum {
    ANIM_WALK_DOWN = 0,
    ANIM_RUN_LEFT  = 1,
    ANIM_RUN_RIGHT = 2,
    ANIM_WALK_UP   = 3
} AnimRow;

typedef enum {
    FACING_LEFT,
    FACING_RIGHT
} Facing;

const float SPEED = 250.0f;
const float GRAVITY = 1200.0f;
const float JUMP_FORCE = 500.0f;
const float GROUND_Y = 350.0f;
const float SIZE = 32.0f;
const float FRAME_DURATION = 0.1f;
const int IDLE_FRAME = 0;  // clean standing pose
const int JUMP_FRAME = 2;  // adjust after a look — pick whichever column reads best as airborne

int main(void) {
    float playerX = 100;
    float playerY = 350;
    float velocityY = 0;
    bool onGround = false;

    Facing facing = FACING_RIGHT;
    int currentFrame = IDLE_FRAME;
    float animTimer = 0;

    InitWindow(800, 450, "raylib test");
    InitAudioDevice();
    Sound jump = LoadSound(JUMP_SOUND);
    Texture2D playerSheet = LoadTexture(SPRITE_SHEET);
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        bool movingRight = IsKeyDown(KEY_RIGHT);
        bool movingLeft  = IsKeyDown(KEY_LEFT);
        bool moving = (movingRight || movingLeft) && onGround;

        if (movingRight) {
            playerX += SPEED * GetFrameTime();
            facing = FACING_RIGHT;
        }

        if (movingLeft) {
            playerX -= SPEED * GetFrameTime();
            facing = FACING_LEFT;
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

        AnimRow currentAnim;

        if (!onGround) {
            currentAnim = (facing == FACING_RIGHT) ? ANIM_RUN_RIGHT : ANIM_RUN_LEFT;
            currentFrame = JUMP_FRAME;
            animTimer = 0;
        } else if (moving) {
            currentAnim = (facing == FACING_RIGHT) ? ANIM_RUN_RIGHT : ANIM_RUN_LEFT;
            animTimer += GetFrameTime();
            if (animTimer >= FRAME_DURATION) {
                animTimer = 0;
                currentFrame = (currentFrame + 1) % FRAMES_PER_ROW;
            }
        } else {
            currentAnim = ANIM_WALK_DOWN;
            currentFrame = IDLE_FRAME;
            animTimer = 0;
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawLine(0, (int)GROUND_Y + (int)SIZE, 800, (int)GROUND_Y + (int)SIZE, DARKGRAY);

        Rectangle source = {
            (float)(currentFrame * FRAME_SIZE),
            (float)(currentAnim * FRAME_SIZE),
            (float)FRAME_SIZE,
            (float)FRAME_SIZE
        };
        Rectangle dest = {
            playerX, playerY,
            FRAME_SIZE * SPRITE_SCALE, FRAME_SIZE * SPRITE_SCALE
        };
        Vector2 origin = { 0, 0 };

        DrawTexturePro(playerSheet, source, dest, origin, 0.0f, WHITE);
        EndDrawing();
    }

    UnloadTexture(playerSheet);
    UnloadSound(jump);
    CloseAudioDevice();
    CloseWindow();

    return 0;
}
