#include <math.h>
#include <string.h>
#include "raylib.h"

#define CUTE_TILED_IMPLEMENTATION
#define CUTE_TILED_NO_EXTERNAL_TILESET_WARNING // Loading the external tileset, on purpose
#include "lib/cute_tiled.h"

#define TILE_SIZE 16
#define MAP_FILE "maps/1.tmj"
#define TILESET_FILE "maps/Tilesheet.tsj"
#define TILESHEET_IMAGE "sprites/Tilesheet.png"

#define JUMP_SOUND "audio/jump.wav"
#define PLAYER_SHEET "sprites/BaseCharacter1.png"
#define FRAME_SIZE 16
#define FRAMES_PER_ROW 6

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

const float PLAYER_SPEED = 120.0f;
const float GRAVITY = 600.0f;
const float JUMP_FORCE = 260.0f;
const float FRAME_DURATION = 0.1f;
const int IDLE_FRAME = 0;
const int JUMP_FRAME = 2;

// Whether the Ground layer has a solid tile at (col, row).
bool IsSolid(cute_tiled_layer_t *groundLayer, int col, int row) {
    if (!groundLayer || col < 0 || col >= groundLayer->width || row < 0 || row >= groundLayer->height) {
        return false;
    }

    return groundLayer->data[row * groundLayer->width + col] != 0;
}

int main(void) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1024, 576, "Map test");
    SetTargetFPS(60);

    cute_tiled_map_t *map = cute_tiled_load_map_from_file(MAP_FILE, NULL);
    if (!map) {
        CloseWindow();

        return 1;
    }

    cute_tiled_tileset_t *tileset = cute_tiled_load_external_tileset(TILESET_FILE, NULL);
    if (!tileset) {
        cute_tiled_free_map(map);
        CloseWindow();

        return 1;
    }

    int tilesheetCols = tileset->columns;
    Texture2D tilesheet = LoadTexture(TILESHEET_IMAGE);

    // Ground is the only layer the player collides with for now.
    cute_tiled_layer_t *groundLayer = NULL;
    for (cute_tiled_layer_t *l = map->layers; l; l = l->next) {
        if (l->name.ptr && strcmp(l->name.ptr, "Ground") == 0) {
            groundLayer = l;
            break;
        }
    }

    InitAudioDevice();
    Sound jumpSound = LoadSound(JUMP_SOUND);
    Texture2D playerSheet = LoadTexture(PLAYER_SHEET);

    // Map is drawn at native tile resolution into this texture, then
    // scaled up as a whole to fill the window.
    int gameWidth = map->width * TILE_SIZE;
    int gameHeight = map->height * TILE_SIZE;
    RenderTexture2D target = LoadRenderTexture(gameWidth, gameHeight);
    SetTextureFilter(target.texture, TEXTURE_FILTER_POINT);

    float playerX = TILE_SIZE;
    float playerY = 0;
    float velocityY = 0;
    bool onGround = false;
    Facing facing = FACING_RIGHT;
    int currentFrame = IDLE_FRAME;
    float animTimer = 0;

    while (!WindowShouldClose()) {
        if ((IsKeyDown(KEY_LEFT_SUPER) || IsKeyDown(KEY_RIGHT_SUPER)) && IsKeyPressed(KEY_F)) {
            ToggleFullscreen();
        }

        float dt = GetFrameTime();
        bool movingRight = IsKeyDown(KEY_RIGHT);
        bool movingLeft = IsKeyDown(KEY_LEFT);
        bool moving = (movingRight || movingLeft) && onGround;

        if (movingRight) {
            playerX += PLAYER_SPEED * dt;
            facing = FACING_RIGHT;
        }

        if (movingLeft) {
            playerX -= PLAYER_SPEED * dt;
            facing = FACING_LEFT;
        }

        if (playerX < 0) {
            playerX = 0;
        }

        if (playerX > gameWidth - FRAME_SIZE) {
            playerX = gameWidth - FRAME_SIZE;
        }

        if (IsKeyPressed(KEY_SPACE) && onGround) {
            velocityY = -JUMP_FORCE;
            onGround = false;
            PlaySound(jumpSound);
        }

        int footCol = (int)(playerX + FRAME_SIZE / 2.0f) / TILE_SIZE;
        if (footCol < 0) {
            footCol = 0;
        }

        if (footCol >= map->width) {
            footCol = map->width - 1;
        }

        int footRow = (int)((playerY + FRAME_SIZE) / TILE_SIZE);

        // Only look for ground to land on when not actively moving upward
        // (jumping) - otherwise a jump would immediately re-snap to the
        // tile it just launched from.
        int groundRow = -1;
        if (velocityY >= 0) {
            for (int row = footRow - 1; row <= footRow + 1; row++) {
                if (IsSolid(groundLayer, footCol, row)) {
                    groundRow = row;
                    break;
                }
            }
        }

        if (groundRow >= 0) {
            // Ground is within a tile of the feet - stand on it. Covers
            // level ground and small (1-tile) steps up or down, like the
            // pond's diagonal-looking edges.
            playerY = (float)(groundRow * TILE_SIZE - FRAME_SIZE);
            velocityY = 0;
            onGround = true;
        }
        else {
            // Nothing within reach - fall. The same check above runs again
            // next frame, so this naturally catches landing once we're
            // actually close to solid ground again.
            velocityY += GRAVITY * dt;
            playerY += velocityY * dt;
            onGround = false;
        }

        AnimRow currentAnim;
        if (!onGround) {
            currentAnim = (facing == FACING_RIGHT) ? ANIM_RUN_RIGHT : ANIM_RUN_LEFT;
            currentFrame = JUMP_FRAME;
            animTimer = 0;
        }
        else if (moving) {
            currentAnim = (facing == FACING_RIGHT) ? ANIM_RUN_RIGHT : ANIM_RUN_LEFT;
            animTimer += dt;
            if (animTimer >= FRAME_DURATION) {
                animTimer = 0;
                currentFrame = (currentFrame + 1) % FRAMES_PER_ROW;
            }
        }
        else {
            currentAnim = ANIM_WALK_DOWN;
            currentFrame = IDLE_FRAME;
            animTimer = 0;
        }

        BeginTextureMode(target);
        ClearBackground(RAYWHITE);

        cute_tiled_layer_t *layer = map->layers;
        while (layer) {
            if (layer->type.ptr && strcmp(layer->type.ptr, "tilelayer") == 0) {
                int width = layer->width;
                int height = layer->height;
                int *data = layer->data;

                for (int row = 0; row < height; row++) {
                    for (int col = 0; col < width; col++) {
                        int gid = data[row * width + col]; // These are the values in the CSV.

                        // 0 == empty cell, skip
                        if (gid == 0) {
                            continue;
                        }

                        // cute_tiled has helper functions for flip flags.
                        // Strip them off before treating this as a plain index.
                        gid = cute_tiled_unset_flags(gid);

                        int tileIndex = gid - 1; // Tiled tiles start at index 1
                        int srcCol = tileIndex % tilesheetCols; // Tiles are sequential. Do away with the row.
                        int srcRow = tileIndex / tilesheetCols; // Same thing, but for rows.

                        Rectangle source = {
                            (float)(srcCol * TILE_SIZE), // This is gonna suck if we don't stick to 16x16
                            (float)(srcRow * TILE_SIZE),
                            (float)TILE_SIZE,
                            (float)TILE_SIZE
                        };
                        Rectangle dest = {
                            (float)(col * TILE_SIZE),
                            (float)(row * TILE_SIZE),
                            (float)TILE_SIZE,
                            (float)TILE_SIZE
                        };
                        Vector2 origin = { 0, 0 };

                        DrawTexturePro(tilesheet, source, dest, origin, 0.0f, WHITE);
                    }
                }
            }

            layer = layer->next;
        }

        Rectangle playerSource = {
            (float)(currentFrame * FRAME_SIZE),
            (float)(currentAnim * FRAME_SIZE),
            (float)FRAME_SIZE,
            (float)FRAME_SIZE
        };
        Rectangle playerDest = { playerX, playerY, FRAME_SIZE, FRAME_SIZE };
        Vector2 playerOrigin = { 0, 0 };
        DrawTexturePro(playerSheet, playerSource, playerDest, playerOrigin, 0.0f, WHITE);

        EndTextureMode();

        int screenWidth = GetScreenWidth();
        int screenHeight = GetScreenHeight();
        float scale = fminf((float)screenWidth / gameWidth, (float)screenHeight / gameHeight);

        Rectangle source = {
            0, 0,
            (float)target.texture.width, -(float)target.texture.height // Flip vertically; render textures are y-flipped.
        };
        Rectangle dest = {
            (screenWidth - gameWidth * scale) * 0.5f,
            (screenHeight - gameHeight * scale) * 0.5f,
            gameWidth * scale,
            gameHeight * scale
        };
        Vector2 origin = { 0, 0 };

        BeginDrawing();
        ClearBackground(BLACK);
        DrawTexturePro(target.texture, source, dest, origin, 0.0f, WHITE);
        EndDrawing();
    }

    UnloadRenderTexture(target);
    UnloadTexture(playerSheet);
    UnloadSound(jumpSound);
    CloseAudioDevice();
    UnloadTexture(tilesheet);
    cute_tiled_free_external_tileset(tileset);
    cute_tiled_free_map(map);
    CloseWindow();

    return 0;
}
