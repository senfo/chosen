#include <string.h>
#include "raylib.h"

#define CUTE_TILED_IMPLEMENTATION
#define CUTE_TILED_NO_EXTERNAL_TILESET_WARNING // Loading the external tileset, on purpose
#include "lib/cute_tiled.h"

#define TILE_SIZE 16
#define MAP_FILE "maps/1.tmj"
#define TILESET_FILE "maps/Tilesheet.tsj"
#define TILESHEET_IMAGE "sprites/Tilesheet.png"

int main(void) {
    InitWindow(800, 450, "Map test");
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

    while (!WindowShouldClose()) {
        BeginDrawing();
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

        EndDrawing();
    }

    UnloadTexture(tilesheet);
    cute_tiled_free_external_tileset(tileset);
    cute_tiled_free_map(map);
    CloseWindow();

    return 0;
}
