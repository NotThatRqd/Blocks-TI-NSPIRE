#pragma once
#include <stdint.h>

#include "draw.h"

typedef struct player {
    int x, y, z;
    Block_t current_block;

    world_t *world;

    void move(int8_t dx, int8_t dy, int8_t dz) {}
    void _move(int8_t dx, int8_t dy, int8_t dz) {
        undraw();

        x += dx;
        y += dy;
        z += dz;

        // Clamp player position to within the world grid
        x = (x < 0) ? 0 : (x >= WORLD_SIZE) ? WORLD_SIZE - 1 : x;
        y = (y < 0) ? 0 : (y >= WORLD_HEIGHT) ? WORLD_HEIGHT - 1 : y;
        z = (z < 0) ? 0 : (z >= WORLD_SIZE) ? WORLD_SIZE - 1 : z;

        draw();
    }

    void draw() {}
    void _draw() {
        int screen_x = scroll_x + 160 + (16 * x) - (16 * z);
        int screen_y = scroll_y + 209 - (8 * x) - (8 * z) - (16 * y);

        _show_msgbox("Ok Jarmin", "before depth", 0);
        uint8_t depth = project_view_depth(x, y, z);

        _show_msgbox("Ok Jarmin", "before tri grid idx", 0);
        int tri_grid_idx = world->project(x, y, z, TOP_FACE);

        _show_msgbox("Ok Jarmin", "before draw back texture", 0);
        // Draw Back Texture
        if (world->tri_grid_depth[tri_grid_idx] > depth)
            draw_left_triangle(screen_x, screen_y, player_tex[RIGHT_FACE * 2], SHADOW);

        tri_grid_idx++;

        _show_msgbox("Ok Jarmin", "before thingssssssss", 0);
        if (world->tri_grid_depth[tri_grid_idx] > depth)
            draw_right_triangle(screen_x, screen_y, player_tex[LEFT_FACE * 2 + 1], SHADOW);

        tri_grid_idx = world->project(x, y, z, MID_FACE);

        if (world->tri_grid_depth[tri_grid_idx] > depth)
            draw_right_triangle(screen_x - 16, screen_y + 8, player_tex[RIGHT_FACE * 2 + 1], SHADOW);

        tri_grid_idx++;

        if (world->tri_grid_depth[tri_grid_idx] > depth)
            draw_left_triangle(screen_x + 16, screen_y + 8, player_tex[LEFT_FACE * 2], SHADOW);

        tri_grid_idx = world->project(x, y, z, BOT_FACE);

        if (world->tri_grid_depth[tri_grid_idx] > depth)
            draw_left_triangle(screen_x, screen_y + 16, player_tex[TOP_FACE * 2], SHADOW);

        tri_grid_idx++;

        if (world->tri_grid_depth[tri_grid_idx] > depth)
            draw_right_triangle(screen_x, screen_y + 16, player_tex[TOP_FACE * 2 + 1], SHADOW);

        // Draw Front Texture
        tri_grid_idx = world->project(x, y, z, TOP_FACE);

        draw_left_triangle(screen_x, screen_y, player_tex[TOP_FACE * 2],
                           world->tri_grid_depth[tri_grid_idx] >= depth ? 0 : SHADOW);

        tri_grid_idx++;

        draw_right_triangle(screen_x, screen_y, player_tex[TOP_FACE * 2 + 1],
                            world->tri_grid_depth[tri_grid_idx] >= depth ? 0 : SHADOW);

        tri_grid_idx = world->project(x, y, z, MID_FACE);

        draw_right_triangle(screen_x - 16, screen_y + 8, player_tex[LEFT_FACE * 2 + 1],
                            world->tri_grid_depth[tri_grid_idx] >= depth ? 0 : SHADOW);

        tri_grid_idx++;

        draw_left_triangle(screen_x + 16, screen_y + 8, player_tex[RIGHT_FACE * 2],
                           world->tri_grid_depth[tri_grid_idx] >= depth ? 0 : SHADOW);

        tri_grid_idx = world->project(x, y, z, BOT_FACE);

        draw_left_triangle(screen_x, screen_y + 16, player_tex[LEFT_FACE * 2],
                           world->tri_grid_depth[tri_grid_idx] >= depth ? 0 : SHADOW);

        tri_grid_idx++;

        draw_right_triangle(screen_x, screen_y + 16, player_tex[RIGHT_FACE * 2 + 1],
                            world->tri_grid_depth[tri_grid_idx] >= depth ? 0 : SHADOW);

        _show_msgbox("Ok Jarmin", "finished draw player", 0);
    }

    void undraw() {
        empty_draw_region();
        expand_draw_region(x, y, z);
        draw_tri_grid(*world);
    }

    void scroll_to_center(int &goal_x, int &goal_y) {
        int screen_x = scroll_x + 160 + (16 * x) - (16 * z);
        int screen_y = scroll_y + 209 - (8 * x) - (8 * z) - (16 * y);

        goal_x = (SCREEN_WIDTH / 2) + scroll_x - screen_x;
        goal_y = (SCREEN_HEIGHT / 2) + scroll_y - screen_y - 16;
    }

    void scroll_to_contain(int &goal_x, int &goal_y) {
        int screen_x = scroll_x + 160 + (16 * x) - (16 * z);
        int screen_y = scroll_y + 209 - (8 * x) - (8 * z) - (16 * y);

        int target_x = screen_x;
        int target_y = screen_y;

        if (target_x < BLOCK_HALF_WIDTH + SCROLL_SPEED) target_x = BLOCK_HALF_WIDTH + SCROLL_SPEED;
        if (target_x > SCREEN_WIDTH - BLOCK_HALF_WIDTH - SCROLL_SPEED)
            target_x = SCREEN_WIDTH - BLOCK_HALF_WIDTH - SCROLL_SPEED;
        if (target_y < SCROLL_SPEED) target_y = SCROLL_SPEED;
        if (target_y > SCREEN_HEIGHT - BLOCK_HEIGHT - SCROLL_SPEED)
            target_y = SCREEN_HEIGHT - BLOCK_HEIGHT - SCROLL_SPEED;

        goal_x = target_x + scroll_x - screen_x;
        goal_y = target_y + scroll_y - screen_y;
    }

} player_t;