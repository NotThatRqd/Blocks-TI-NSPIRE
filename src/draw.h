#pragma once
#include <libndls.h>
#include <string.h>

#include "textures.h"
#include "world.h"

#define BLOCK_WIDTH 32
#define BLOCK_HALF_WIDTH (BLOCK_WIDTH / 2)
#define BLOCK_HEIGHT 31

#define SCROLL_SPEED 8

#define LCD_CNT (SCREEN_WIDTH * SCREEN_HEIGHT)

#define BUFFER_1 0xD40000
#define BUFFER_2 0xD52C00
#define BUFFER_SWP (BUFFER_1 ^ BUFFER_2)

extern scr_type_t current_lcd_type;

extern uint16_t back_buffer[LCD_CNT];

extern int scroll_x;
extern int scroll_y;

// Bounds to draw within
extern uint16_t draw_x0;
extern uint16_t draw_y0;
extern uint16_t draw_x1;
extern uint16_t draw_y1;

// Draws a left facing triangle and checks every pixel to ensure nothing gets drawn out of bounds
void draw_left_triangle_clipped(int x0, int y0, uint8_t *tex, uint8_t *shadow_mask, uint8_t *water_mask);

// Draws a right facing triangle and checks every pixel to ensure nothing gets drawn out of bounds
void draw_right_triangle_clipped(int x0, int y0, uint8_t *tex, uint8_t *shadow_mask, uint8_t *water_mask);

// Draws a left facing triangle
void draw_left_triangle(int x0, int y0, uint8_t *tex, uint8_t *shadow_mask, uint8_t *water_mask);

// Draws a right facing triangle
void draw_right_triangle(int x0, int y0, uint8_t *tex, uint8_t *shadow_mask, uint8_t *water_mask);

// Draws a left facing filled triangle and checks every pixel to ensure nothing gets drawn out of bounds
void draw_left_triangle_clipped(int x0, int y0, uint8_t *water_mask);

// Draws a right facing filled triangle and checks every pixel to ensure nothing gets drawn out of bounds
void draw_right_triangle_clipped(int x0, int y0, uint8_t *water_mask);

// Draws a left facing filled triangle
void draw_left_triangle(int x0, int y0, uint8_t *water_mask);

// Draws a right facing filled triangle
void draw_right_triangle(int x0, int y0, uint8_t *water_mask);

// Draws a left facing triangle
void draw_left_triangle(int x0, int y0, uint8_t *tex, uint8_t flags);

// Draws a right facing triangle
void draw_right_triangle(int x0, int y0, uint8_t *tex, uint8_t flags);

void draw_block(int x, int y, uint8_t *tex);

void draw_block(uint8_t x, uint8_t y, uint8_t z, uint8_t *tex);

void draw_tri_grid(world_t &world);

void scroll_view(world_t &world, int x, int y);

void dim_screen();

// void draw_num(int x, int y, uint8_t n);

void empty_draw_region(void);

void expand_draw_region(uint8_t x, uint8_t y, uint8_t z);
