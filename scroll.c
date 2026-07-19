#include "raylib.h"

int main(void) {
    InitWindow(800, 450, "Scrolling Camera");
    SetTargetFPS(60);

    float playerX = 100;
    float playerY = 350;
    const float speed = 250.0f;

    Rectangle platforms[5] = {
        { 0,    400, 300, 50 },
        { 400,  400, 200, 50 },
        { 700,  350, 150, 20 },
        { 950,  400, 300, 50 },
        { 1350, 300, 150, 20 },
    };

    Camera2D camera = { 0 };
    camera.offset = (Vector2){ 400, 225 };
    camera.zoom = 1.0f;

    while (!WindowShouldClose()) {
        if (IsKeyDown(KEY_RIGHT)) {
            playerX += speed * GetFrameTime();
        }

        if (IsKeyDown(KEY_LEFT)) {
            playerX -= speed * GetFrameTime();
        }

        camera.target = (Vector2){ playerX, playerY };

        BeginDrawing();
        ClearBackground(RAYWHITE);
        BeginMode2D(camera);

        for (int i = 0; i < 5; i++) {
            DrawRectangleRec(platforms[i], DARKGRAY);
        }

        DrawRectangle((int)playerX, (int)playerY - 32, 24, 32, MAROON);
        EndMode2D();

        DrawText("Arrow keys to move", 10, 10, 20, BLACK);
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
