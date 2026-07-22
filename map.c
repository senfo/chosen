#include "raylib.h"

#define TILE_SIZE 16

int main(void) {
    InitWindow(800, 450, "Map test");
    SetTargetFPS(60);

    tinytiled_map_t* map = tinytiled_load_map_from_file("assets/1.tmj", NULL);
    if (!map) {
        CloseWindow();
        return 1;
    }

    Texture2D tilesheet = LoadTexture("assets/Tilesheet.png");
    int tilesheetCols = map->tilesets->columns; // 13, from your .tsx

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        tinytiled_layer_t* layer = map->layers;
        while (layer) {
            // tinytiled doesn't hash the type string like cute_tiled does —
            // it exposes it as a plain string via .ptr, so compare directly
            if (layer->type.ptr && strcmp(layer->type.ptr, "tilelayer") == 0) {
                int width = layer->width;
                int height = layer->height;
                int* data = layer->data;

                for (int row = 0; row < height; row++) {
                    for (int col = 0; col < width; col++) {
                        int gid = data[row * width + col];
                        if (gid == 0) continue; // 0 = empty cell, skip

                        // tinytiled has helper functions for flip flags —
                        // strip them off before treating this as a plain index
                        gid = tinytiled_unset_flags(gid);

                        int tileIndex = gid - 1; // firstgid=1, single tileset
                        int srcCol = tileIndex % tilesheetCols;
                        int srcRow = tileIndex / tilesheetCols;

                        Rectangle source = {
                            (float)(srcCol * TILE_SIZE),
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

        EndDrawing();
    }

    UnloadTexture(tilesheet);
    tinytiled_free_map(map);
    CloseWindow();
    return 0;
}
