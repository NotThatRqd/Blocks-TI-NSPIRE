#include "draw.h"

#include <libndls.h>
#include <string.h>

#include "textures.h"

scr_type_t current_lcd_type;

uint16_t back_buffer[LCD_CNT] = {0};

int scroll_x = 0;
int scroll_y = 0;

// Bounds to draw within
uint16_t draw_x0 = 0;
uint16_t draw_y0 = 0;
uint16_t draw_x1 = SCREEN_WIDTH;
uint16_t draw_y1 = SCREEN_HEIGHT;

// Copies pixels from a texture line into a back_buffer line, applying a constant mask
// across all pixels In this case, the texture can be transparent, with blank
// pixels represented as a zero
void copy_tex_line(uint16_t *dest, uint8_t *tex, uint8_t flags, int length) {
    for (int i = 0; i < length; i++) {
        if (tex[i]) dest[i] = tex_palette[tex[i] | flags];
    }
}

// Copies pixels from a texture line into a back_buffer line, along with applying
// shadow and water mask terms
void copy_tex_line(uint16_t *dest, uint8_t *tex, uint8_t *shadow_mask, uint8_t *water_mask, int length) {
    for (int i = 0; i < length; i++) {
        dest[i] = tex_palette[tex[i] | shadow_mask[i] | water_mask[i]];
    }
}

// Copies SKY colored pixels with the water mask applied into a back_buffer line
void copy_tex_line(uint16_t *dest, uint8_t *water_mask, int length) {
    for (int i = 0; i < length; i++) {
        dest[i] = tex_palette[SKY | water_mask[i]];
    }
}

// Draws a left facing triangle and checks every pixel to ensure nothing gets
// drawn out of bounds
void draw_left_triangle_clipped(int x0, int y0, uint8_t *tex, uint8_t *shadow_mask, uint8_t *water_mask) {
    for (int row = 1; row <= 8; row++) {
        for (int dx = x0 - 2 * row; dx < x0; dx++) {
            uint8_t color = *(tex++) | *(shadow_mask++) | *(water_mask++);

            int dy = (row - 1 + y0);

            if (dy >= 0 && dy < SCREEN_HEIGHT && dx >= 0 && dx < SCREEN_WIDTH)
                back_buffer[SCREEN_WIDTH * dy + dx] = tex_palette[color];
        }
    }

    for (int row = 1; row < 8; row++) {
        for (int dx = x0 - 16 + 2 * row; dx < x0; dx++) {
            uint8_t color = *(tex++) | *(shadow_mask++) | *(water_mask++);

            int dy = (8 + row - 1 + y0);

            if (dy >= 0 && dy < SCREEN_HEIGHT && dx >= 0 && dx < SCREEN_WIDTH)
                back_buffer[SCREEN_WIDTH * dy + dx] = tex_palette[color];
        }
    }
}

// Draws a right facing triangle and checks every pixel to ensure nothing gets
// drawn out of bounds
void draw_right_triangle_clipped(int x0, int y0, uint8_t *tex, uint8_t *shadow_mask, uint8_t *water_mask) {
    for (int row = 1; row <= 8; row++) {
        for (int dx = x0; dx < x0 + 2 * row; dx++) {
            uint8_t color = *(tex++) | *(shadow_mask++) | *(water_mask++);

            int dy = (row - 1 + y0);

            if (dy >= 0 && dy < SCREEN_HEIGHT && dx >= 0 && dx < SCREEN_WIDTH)
                back_buffer[SCREEN_WIDTH * dy + dx] = tex_palette[color];
        }
    }

    for (int row = 1; row < 8; row++) {
        for (int dx = x0; dx < x0 + 16 - 2 * row; dx++) {
            uint8_t color = *(tex++) | *(shadow_mask++) | *(water_mask++);

            int dy = (8 + row - 1 + y0);

            if (dy >= 0 && dy < SCREEN_HEIGHT && dx >= 0 && dx < SCREEN_WIDTH)
                back_buffer[SCREEN_WIDTH * dy + dx] = tex_palette[color];
        }
    }
}

// Draws a left facing triangle
void draw_left_triangle(int x0, int y0, uint8_t *tex, uint8_t *shadow_mask, uint8_t *water_mask) {
    // If we're going to clip, defer to the slow version
    if (x0 < 16 || x0 > SCREEN_WIDTH || y0 < 0 || y0 >= SCREEN_HEIGHT - 16) {
        draw_left_triangle_clipped(x0, y0, tex, shadow_mask, water_mask);
        return;
    }

    // Our base back_buffer pointer for each line
    uint16_t *base = &back_buffer[SCREEN_WIDTH * y0 + x0];
    // The width of each line as we draw
    uint8_t width = 0;

    // Top half
    for (int row = 1; row <= 8; row++) {
        width += 2;
        // memcpy(base - width, tex, width);
        copy_tex_line(base - width, tex, shadow_mask, water_mask, width);
        tex += width;
        shadow_mask += width;
        water_mask += width;
        base += SCREEN_WIDTH;
    }

    // Bottom half
    for (int row = 1; row < 8; row++) {
        width -= 2;
        // memcpy(base - width, tex, width);
        copy_tex_line(base - width, tex, shadow_mask, water_mask, width);
        tex += width;
        shadow_mask += width;
        water_mask += width;
        base += SCREEN_WIDTH;
    }
}

// Draws a right facing triangle
void draw_right_triangle(int x0, int y0, uint8_t *tex, uint8_t *shadow_mask, uint8_t *water_mask) {
    // If we're going to clip, defer to the slow version
    if (x0 < 0 || x0 >= SCREEN_WIDTH - 16 || y0 < 0 || y0 >= SCREEN_HEIGHT - 16) {
        draw_right_triangle_clipped(x0, y0, tex, shadow_mask, water_mask);
        return;
    }

    // Our base back_buffer pointer for each line
    uint16_t *base = &back_buffer[SCREEN_WIDTH * y0 + x0];
    // The width of each line as we draw
    uint8_t width = 0;

    // Top half
    for (int row = 1; row <= 8; row++) {
        width += 2;
        copy_tex_line(base, tex, shadow_mask, water_mask, width);
        tex += width;
        shadow_mask += width;
        water_mask += width;
        base += SCREEN_WIDTH;
    }

    // Bottom half
    for (int row = 1; row < 8; row++) {
        width -= 2;
        copy_tex_line(base, tex, shadow_mask, water_mask, width);
        tex += width;
        shadow_mask += width;
        water_mask += width;
        base += SCREEN_WIDTH;
    }
}

// Draws a left facing triangle and checks every pixel to ensure nothing gets
// drawn out of bounds
void draw_left_triangle_clipped(int x0, int y0, uint8_t *water_mask) {
    for (int row = 1; row <= 8; row++) {
        for (int dx = x0 - 2 * row; dx < x0; dx++) {
            uint8_t color = SKY | *(water_mask++);

            int dy = (row - 1 + y0);

            if (dy >= 0 && dy < SCREEN_HEIGHT && dx >= 0 && dx < SCREEN_WIDTH)
                back_buffer[SCREEN_WIDTH * dy + dx] = tex_palette[color];
        }
    }

    for (int row = 1; row < 8; row++) {
        for (int dx = x0 - 16 + 2 * row; dx < x0; dx++) {
            uint8_t color = SKY | *(water_mask++);

            int dy = (8 + row - 1 + y0);

            if (dy >= 0 && dy < SCREEN_HEIGHT && dx >= 0 && dx < SCREEN_WIDTH)
                back_buffer[SCREEN_WIDTH * dy + dx] = tex_palette[color];
        }
    }
}

// Draws a right facing triangle and checks every pixel to ensure nothing gets
// drawn out of bounds
void draw_right_triangle_clipped(int x0, int y0, uint8_t *water_mask) {
    for (int row = 1; row <= 8; row++) {
        for (int dx = x0; dx < x0 + 2 * row; dx++) {
            uint8_t color = SKY | *(water_mask++);

            int dy = (row - 1 + y0);

            if (dy >= 0 && dy < SCREEN_HEIGHT && dx >= 0 && dx < SCREEN_WIDTH)
                back_buffer[SCREEN_WIDTH * dy + dx] = tex_palette[color];
        }
    }

    for (int row = 1; row < 8; row++) {
        for (int dx = x0; dx < x0 + 16 - 2 * row; dx++) {
            uint8_t color = SKY | *(water_mask++);

            int dy = (8 + row - 1 + y0);

            if (dy >= 0 && dy < SCREEN_HEIGHT && dx >= 0 && dx < SCREEN_WIDTH)
                back_buffer[SCREEN_WIDTH * dy + dx] = tex_palette[color];
        }
    }
}

// Draws a left facing triangle
void draw_left_triangle(int x0, int y0, uint8_t *water_mask) {
    // If we're going to clip, defer to the slow version
    if (x0 < 16 || x0 > SCREEN_WIDTH || y0 < 0 || y0 >= SCREEN_HEIGHT - 16) {
        draw_left_triangle_clipped(x0, y0, water_mask);
        return;
    }

    // Our base back_buffer pointer for each line
    uint16_t *base = &back_buffer[SCREEN_WIDTH * y0 + x0];
    // The width of each line as we draw
    uint8_t width = 0;

    // Top half
    for (int row = 1; row <= 8; row++) {
        width += 2;
        copy_tex_line(base - width, water_mask, width);
        water_mask += width;
        base += SCREEN_WIDTH;
    }

    // Bottom half
    for (int row = 1; row < 8; row++) {
        width -= 2;
        copy_tex_line(base - width, water_mask, width);
        water_mask += width;
        base += SCREEN_WIDTH;
    }
}

// Draws a right facing triangle
void draw_right_triangle(int x0, int y0, uint8_t *water_mask) {
    // If we're going to clip, defer to the slow version
    if (x0 < 0 || x0 >= SCREEN_WIDTH - 16 || y0 < 0 || y0 >= SCREEN_HEIGHT - 16) {
        draw_right_triangle_clipped(x0, y0, water_mask);
        return;
    }

    // Our base back_buffer pointer for each line
    uint16_t *base = &back_buffer[SCREEN_WIDTH * y0 + x0];
    // The width of each line as we draw
    uint8_t width = 0;

    // Top half
    for (int row = 1; row <= 8; row++) {
        width += 2;
        copy_tex_line(base, water_mask, width);
        water_mask += width;
        base += SCREEN_WIDTH;
    }

    // Bottom half
    for (int row = 1; row < 8; row++) {
        width -= 2;
        copy_tex_line(base, water_mask, width);
        water_mask += width;
        base += SCREEN_WIDTH;
    }
}

// Draws a left facing triangle and checks every pixel to ensure nothing gets
// drawn out of bounds
void draw_left_triangle_clipped(int x0, int y0, uint8_t *tex, uint8_t flags) {
    for (int row = 1; row <= 8; row++) {
        for (int dx = x0 - 2 * row; dx < x0; dx++) {
            uint8_t color = *(tex++);

            if (color == 0) continue;

            color |= flags;

            int dy = (row - 1 + y0);

            if (dy >= 0 && dy < SCREEN_HEIGHT && dx >= 0 && dx < SCREEN_WIDTH)
                back_buffer[SCREEN_WIDTH * dy + dx] = tex_palette[color];
        }
    }

    for (int row = 1; row < 8; row++) {
        for (int dx = x0 - 16 + 2 * row; dx < x0; dx++) {
            uint8_t color = *(tex++);

            if (color == 0) continue;

            color |= flags;

            int dy = (8 + row - 1 + y0);

            if (dy >= 0 && dy < SCREEN_HEIGHT && dx >= 0 && dx < SCREEN_WIDTH)
                back_buffer[SCREEN_WIDTH * dy + dx] = tex_palette[color];
        }
    }
}

// Draws a right facing triangle and checks every pixel to ensure nothing gets
// drawn out of bounds
void draw_right_triangle_clipped(int x0, int y0, uint8_t *tex, uint8_t flags) {
    for (int row = 1; row <= 8; row++) {
        for (int dx = x0; dx < x0 + 2 * row; dx++) {
            uint8_t color = *(tex++);

            if (color == 0) continue;

            color |= flags;

            int dy = (row - 1 + y0);

            if (dy >= 0 && dy < SCREEN_HEIGHT && dx >= 0 && dx < SCREEN_WIDTH)
                back_buffer[SCREEN_WIDTH * dy + dx] = tex_palette[color];
        }
    }

    for (int row = 1; row < 8; row++) {
        for (int dx = x0; dx < x0 + 16 - 2 * row; dx++) {
            uint8_t color = *(tex++);

            if (color == 0) continue;

            color |= flags;

            int dy = (8 + row - 1 + y0);

            if (dy >= 0 && dy < SCREEN_HEIGHT && dx >= 0 && dx < SCREEN_WIDTH)
                back_buffer[SCREEN_WIDTH * dy + dx] = tex_palette[color];
        }
    }
}

// Draws a left facing triangle
void draw_left_triangle(int x0, int y0, uint8_t *tex, uint8_t flags) {
    // If we're going to clip, defer to the slow version
    if (x0 < 16 || x0 > SCREEN_WIDTH || y0 < 0 || y0 >= SCREEN_HEIGHT - 16) {
        draw_left_triangle_clipped(x0, y0, tex, flags);
        return;
    }

    // Our base back_buffer pointer for each line
    uint16_t *base = &back_buffer[SCREEN_WIDTH * y0 + x0];
    // The width of each line as we draw
    uint8_t width = 0;

    // Top half
    for (int row = 1; row <= 8; row++) {
        width += 2;
        // memcpy(base - width, tex, width);
        copy_tex_line(base - width, tex, flags, width);
        tex += width;
        base += SCREEN_WIDTH;
    }

    // Bottom half
    for (int row = 1; row < 8; row++) {
        width -= 2;
        // memcpy(base - width, tex, width);
        copy_tex_line(base - width, tex, flags, width);
        tex += width;
        base += SCREEN_WIDTH;
    }
}

// Draws a right facing triangle
void draw_right_triangle(int x0, int y0, uint8_t *tex, uint8_t flags) {
    // If we're going to clip, defer to the slow version
    if (x0 < 0 || x0 >= SCREEN_WIDTH - 16 || y0 < 0 || y0 >= SCREEN_HEIGHT - 16) {
        draw_right_triangle_clipped(x0, y0, tex, flags);
        return;
    }

    // Our base back_buffer pointer for each line
    uint16_t *base = &back_buffer[SCREEN_WIDTH * y0 + x0];
    // The width of each line as we draw
    uint8_t width = 0;

    // Top half
    for (int row = 1; row <= 8; row++) {
        width += 2;
        copy_tex_line(base, tex, flags, width);
        tex += width;
        base += SCREEN_WIDTH;
    }

    // Bottom half
    for (int row = 1; row < 8; row++) {
        width -= 2;
        copy_tex_line(base, tex, flags, width);
        tex += width;
        base += SCREEN_WIDTH;
    }
}

void draw_block(int x, int y, uint8_t *tex) {
    draw_left_triangle(x, y, &tex[TEX_SIZE * (TOP_FACE * 2)], shadow_masks[SHADOW_NONE][TOP_FACE * 2],
                       water_masks[WATER_NONE][TOP_FACE * 2]);
    draw_right_triangle(x, y, &tex[TEX_SIZE * (TOP_FACE * 2 + 1)], shadow_masks[SHADOW_NONE][TOP_FACE * 2 + 1],
                        water_masks[WATER_NONE][TOP_FACE * 2 + 1]);

    draw_left_triangle(x, y + 16, &tex[TEX_SIZE * (LEFT_FACE * 2)], shadow_masks[SHADOW_NONE][LEFT_FACE * 2],
                       water_masks[WATER_NONE][LEFT_FACE * 2]);
    draw_right_triangle(x - 16, y + 8, &tex[TEX_SIZE * (LEFT_FACE * 2 + 1)],
                        shadow_masks[SHADOW_NONE][LEFT_FACE * 2 + 1], water_masks[WATER_NONE][LEFT_FACE * 2 + 1]);

    draw_left_triangle(x + 16, y + 8, &tex[TEX_SIZE * (RIGHT_FACE * 2)], shadow_masks[SHADOW_NONE][RIGHT_FACE * 2],
                       water_masks[WATER_NONE][RIGHT_FACE * 2]);
    draw_right_triangle(x, y + 16, &tex[TEX_SIZE * (RIGHT_FACE * 2 + 1)], shadow_masks[SHADOW_NONE][RIGHT_FACE * 2 + 1],
                        water_masks[WATER_NONE][RIGHT_FACE * 2 + 1]);
}

void draw_block(uint8_t x, uint8_t y, uint8_t z, uint8_t *tex) {
    int screen_x = scroll_x + 160 + (16 * x) - (16 * z);
    int screen_y = scroll_y + 209 - (8 * x) - (8 * z) - (16 * y);

    draw_block(screen_x, screen_y, tex);
}

void draw_tri_grid(world_t &world) {
    int origin_x = SCREEN_WIDTH / 2;
    int draw_y = SCREEN_HEIGHT - 15;

    int start_row = (scroll_y + SCREEN_HEIGHT - draw_y1 - 1) / 8;
    int end_row = (scroll_y + SCREEN_HEIGHT - draw_y0 + 7) / 8;

    // Clamp the range to [0, ROW_CNT)
    start_row = (start_row < 0 ? 0 : start_row);
    end_row = (end_row > ROW_CNT ? ROW_CNT : end_row);

    draw_y -= 8 * start_row;

    for (int row = start_row; row < end_row; row++) {
        int width = world.tri_grid_row_width[row];
        int draw_x = origin_x - world.tri_grid_row_px_offset[row];
        int offset = world.tri_grid_row_offset[row];

        // Round down the start triangle, and up the end triangle
        int start_tri = (draw_x0 - draw_x - scroll_x + 16) / 16;
        int end_tri = (draw_x1 - draw_x - scroll_x + 31) / 16;

        // Clamp the range to [0, width)
        start_tri = (start_tri < 0 ? 0 : start_tri);
        end_tri = (end_tri > width ? width : end_tri);

        draw_x += start_tri * 16;

        if (((offset + start_tri) & 1) == 1) {
            draw_x -= 16;
        }

        for (int i = start_tri; i < end_tri; i++) {
            uint8_t texture = world.tri_grid_tex[world.tri_grid_rows[row] + i];
            // uint8_t texture = world.tri_grid_depth[world.tri_grid_rows[row] +
            // i] % 8 + STONE;
            uint8_t flags = world.tri_grid_flags[world.tri_grid_rows[row] + i];

            uint8_t face = flags & FACE_MASK;
            uint8_t shadow = (flags & SHADOW_MASK) >> SHADOW_OFFSET;
            uint8_t water = (flags & WATER_MASK) >> WATER_OFFSET;

            if (((i - offset) & 1) == 0) {
                // Draw filled triangles with texture and shadows
                if (texture != 0) {
                    texture -= 2;
                    draw_left_triangle(draw_x + scroll_x, draw_y + scroll_y, textures[texture][face * 2 + 0],
                                       shadow_masks[shadow][face * 2 + 0], water_masks[water][face * 2 + 0]);
                }
                // Draw empty triangles with sky color and water
                else {
                    draw_left_triangle(draw_x + scroll_x, draw_y + scroll_y, water_masks[water][face * 2 + 0]);
                }
            } else {
                // Draw filled triangles with texture and shadows
                if (texture != 0) {
                    texture -= 2;
                    draw_right_triangle(draw_x + scroll_x, draw_y + scroll_y, textures[texture][face * 2 + 1],
                                        shadow_masks[shadow][face * 2 + 1], water_masks[water][face * 2 + 1]);
                }
                // Draw empty triangles with sky color and water
                else {
                    draw_right_triangle(draw_x + scroll_x, draw_y + scroll_y, water_masks[water][face * 2 + 1]);
                }

                draw_x += 32;
            }
        }

        draw_y -= 8;
    }
}

/*
void scroll_view(world_t &world, int x, int y) {
    scroll_x += x;
    scroll_y += y;

    int abs_x = (x > 0) ? x : -x;
    int abs_y = (y > 0) ? y : -y;

    // uint16_t *src_row = back_buffer;
    // uint16_t *dst_row = back_buffer;

    //// Vertical scrolling
    // if (y > 0) {
    // dst_row += SCREEN_WIDTH * abs_y;
    //} else {
    // src_row += SCREEN_WIDTH * abs_y;
    //}

    //// Horizontal scrolling
    // if (x > 0) {
    // dst_row += abs_x;
    //} else {
    // src_row += abs_x;
    //}

    // --- Handle vertical direction ---
    if (y >= 0) {
        // Scrolling DOWN → start from bottom
        for (int row = SCREEN_HEIGHT - 1; row >= abs_y; row--) {
            uint16_t *src_row = back_buffer + (row - abs_y) * SCREEN_WIDTH;
            uint16_t *dst_row = back_buffer + row * SCREEN_WIDTH;

            // --- Handle horizontal direction per row ---
            if (x >= 0) {
                // Scrolling RIGHT → copy right-to-left
                memmove(dst_row + abs_x, src_row, (SCREEN_WIDTH - abs_x) * sizeof(uint16_t));
            } else {
                // Scrolling LEFT → copy left-to-right
                memmove(dst_row, src_row + abs_x, (SCREEN_WIDTH - abs_x) * sizeof(uint16_t));
            }
        }
    } else {
        // Scrolling UP → start from top
        for (int row = 0; row < SCREEN_HEIGHT - abs_y; row++) {
            uint16_t *src_row = back_buffer + (row + abs_y) * SCREEN_WIDTH;
            uint16_t *dst_row = back_buffer + row * SCREEN_WIDTH;

            // --- Handle horizontal direction per row ---
            if (x >= 0) {
                // Scrolling RIGHT → copy right-to-left
                memmove(dst_row + abs_x, src_row, (SCREEN_WIDTH - abs_x) * sizeof(uint16_t));
            } else {
                // Scrolling LEFT → copy left-to-right
                memmove(dst_row, src_row + abs_x, (SCREEN_WIDTH - abs_x) * sizeof(uint16_t));
            }
        }
    }

    // Move the frame row by row
    // for (int row = 0; row < SCREEN_HEIGHT - abs_y; row++) {
    // memmove((void *)dst_row, (void *)src_row, (SCREEN_WIDTH - abs_x) * sizeof(uint16_t));

    // src_row += SCREEN_WIDTH;
    // dst_row += SCREEN_WIDTH;
    //}

    // Vertical scrolling
    draw_x0 = 0;
    draw_x1 = SCREEN_WIDTH;

    if (y > 0) {
        draw_y0 = 0;
        draw_y1 = y + 16;
    } else if (y < 0) {
        draw_y0 = SCREEN_HEIGHT + y - 16;
        draw_y1 = SCREEN_HEIGHT;
    }

    if (y != 0) {
        // Clear out the rectangle
        for (int row = draw_y0; y < draw_y1; row++) {
            // TODO: NOTE: this ONLY works because sky is 0 and so you can write it next to itself and it's still the
            // correct value
            // memset(&back_buffer[row * SCREEN_WIDTH + draw_x0], SKY, (draw_x1 - draw_x0) * sizeof(uint16_t));
            for (int column = draw_x0; column < draw_x1; column++) {
                back_buffer[row * SCREEN_WIDTH + column] = tex_palette[SKY];
            }
        }

        // Redraw in a patch
        draw_tri_grid(world);
    }

    // Horizontal scrolling
    draw_y0 = 0;
    draw_y1 = SCREEN_HEIGHT;

    if (x > 0) {
        draw_x0 = 0;
        draw_x1 = x + 16;
    } else if (x < 0) {
        draw_x0 = SCREEN_WIDTH + x - 16;
        draw_x1 = SCREEN_WIDTH;
    }

    if (x != 0) {
        // Clear out the rectangle
        for (int row = draw_y0; row < draw_y1; row++) {
            // TODO: NOTE: this ONLY works because sky is 0 and so you can write it next to itself and it's still the
            // correct value
            // memset(&back_buffer[y * SCREEN_WIDTH + draw_x0], SKY, (draw_x1 - draw_x0) * sizeof(uint16_t));
            for (int column = draw_x0; column < draw_x1; column++) {
                back_buffer[row * SCREEN_WIDTH + column] = tex_palette[SKY];
            }
        }

        // Redraw in a patch
        draw_tri_grid(world);
    }
}
*/

void scroll_view(world_t &world, int x, int y) {
    // Update global scroll offsets
    scroll_x += x;
    scroll_y += y;

    int abs_x = (x > 0) ? x : -x;
    int abs_y = (y > 0) ? y : -y;

    // Pointer to start of back buffer (16-bit pixels now)
    uint16_t *dst = back_buffer;

    // Move the buffer content in place, using memmove to handle overlapping regions safely
    if (y != 0 || x != 0) {
        // Calculate row offsets in pixels (not bytes)
        if (y >= 0) {
            // Scrolling down → start from bottom to avoid overwriting data
            for (int row = SCREEN_HEIGHT - 1; row >= abs_y; row--) {
                uint16_t *dst_row = dst + row * SCREEN_WIDTH;
                uint16_t *src_row = dst + (row - abs_y) * SCREEN_WIDTH;

                if (x >= 0) {
                    // Scroll right → copy from right to left
                    memmove(dst_row + abs_x, src_row, (SCREEN_WIDTH - abs_x) * sizeof(uint16_t));
                } else {
                    // Scroll left → copy from left to right
                    memmove(dst_row, src_row + abs_x, (SCREEN_WIDTH - abs_x) * sizeof(uint16_t));
                }
            }
        } else {
            // Scrolling up → start from top to avoid overwriting data
            for (int row = 0; row < SCREEN_HEIGHT - abs_y; row++) {
                uint16_t *dst_row = dst + row * SCREEN_WIDTH;
                uint16_t *src_row = dst + (row + abs_y) * SCREEN_WIDTH;

                if (x >= 0) {
                    // Scroll right → copy from right to left
                    memmove(dst_row + abs_x, src_row, (SCREEN_WIDTH - abs_x) * sizeof(uint16_t));
                } else {
                    // Scroll left → copy from left to right
                    memmove(dst_row, src_row + abs_x, (SCREEN_WIDTH - abs_x) * sizeof(uint16_t));
                }
            }
        }
    }

    //
    // --- Redraw newly revealed vertical strips ---
    //
    draw_x0 = 0;
    draw_x1 = SCREEN_WIDTH;

    if (y > 0) {
        draw_y0 = 0;
        draw_y1 = y + 16;
    } else if (y < 0) {
        draw_y0 = SCREEN_HEIGHT + y - 16;
        draw_y1 = SCREEN_HEIGHT;
    }

    if (y != 0) {
        for (int py = draw_y0; py < draw_y1; py++) {
            uint16_t *row_start = &back_buffer[py * SCREEN_WIDTH + draw_x0];
            for (int px = draw_x0; px < draw_x1; px++) {
                row_start[px - draw_x0] = tex_palette[SKY];
            }
        }
        draw_tri_grid(world);
    }

    //
    // --- Redraw newly revealed horizontal strips ---
    //
    draw_y0 = 0;
    draw_y1 = SCREEN_HEIGHT;

    if (x > 0) {
        draw_x0 = 0;
        draw_x1 = x + 16;
    } else if (x < 0) {
        draw_x0 = SCREEN_WIDTH + x - 16;
        draw_x1 = SCREEN_WIDTH;
    }

    if (x != 0) {
        for (int py = draw_y0; py < draw_y1; py++) {
            uint16_t *row_start = &back_buffer[py * SCREEN_WIDTH + draw_x0];
            for (int px = draw_x0; px < draw_x1; px++) {
                row_start[px - draw_x0] = tex_palette[SKY];
            }
        }
        draw_tri_grid(world);
    }
}

void dim_screen() {
    for (int i = 0; i < LCD_CNT; i++) {
        back_buffer[i] |= SHADOW;
    }
}

// TODO: port this function
/*void draw_num(int x, int y, uint8_t n) {
    char buf[4];

    for (int i = 0; i < 3; i++) {
        buf[2 - i] = (n % 10) + '0';
        n /= 10;
    }

    buf[3] = 0;

    gfx_SetDrawScreen();
    gfx_SetColor(SKY);
    gfx_FillRectangle(x, y, 24, 8);
    gfx_SetColor(0);
    gfx_PrintStringXY(buf, x, y);
    gfx_SetDrawBuffer();
}*/

void empty_draw_region(void) {
    draw_x0 = SCREEN_WIDTH;
    draw_x1 = 0;
    draw_y0 = SCREEN_HEIGHT;
    draw_y1 = 0;
}

int max(int a, int b) { return (a > b) ? a : b; }

int min(int a, int b) { return (a < b) ? a : b; }

uint16_t max(uint16_t a, uint16_t b) { return (a > b) ? a : b; }

uint16_t min(uint16_t a, uint16_t b) { return (a < b) ? a : b; }

void expand_draw_region(uint8_t x, uint8_t y, uint8_t z) {
    int screen_x = scroll_x + 160 + (16 * x) - (16 * z);
    int screen_y = scroll_y + 209 - (8 * x) - (8 * z) - (16 * y);

    uint16_t block_draw_x0 = (uint16_t)max(min(screen_x - 16, SCREEN_WIDTH), 0);
    uint16_t block_draw_x1 = (uint16_t)max(min(screen_x + 16, SCREEN_WIDTH), 0);
    uint16_t block_draw_y0 = (uint16_t)max(min(screen_y - 0, SCREEN_HEIGHT), 0);
    uint16_t block_draw_y1 = (uint16_t)max(min(screen_y + 24, SCREEN_HEIGHT), 0);

    draw_x0 = min(draw_x0, block_draw_x0);
    draw_x1 = max(draw_x1, block_draw_x1);
    draw_y0 = min(draw_y0, block_draw_y0);
    draw_y1 = max(draw_y1, block_draw_y1);
}