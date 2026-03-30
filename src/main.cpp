#include <libndls.h>
#include <stdlib.h>

#include "draw.h"
#include "player.h"
// #include "ui.h"
#include "world.h"
// #include "world_io.h"
#include "worldgen.h"

void init_play(uint8_t world_id, world_t* world, player_t& player) {
    world->clear_world();
    world->init_tri_grid();

    // Try to load the world file, and otherwise generate a new one
    /*
    if (!load(world_id, *world, player)) {
        const char* options[4] = {"What type of world?", "Natural", "Flat", "Demo"};

        switch (menu(options, 3)) {
            case 0:
                generate_natural(*world, player);
                break;
            case 1:
                generate_flat(*world, player);
                break;
            case 2:
                generate_demo(*world, player);
                break;
        }

        player.scroll_to_center(scroll_x, scroll_y);
    }
    */
    generate_demo(*world, player);
    player.scroll_to_center(scroll_x, scroll_y);

    // gfx_FillScreen(1);
    // progress_bar("Building world...");

    // progress_bar("Initializing shadows...");
    //  Initialize the shadow triangle grid with data from the world
    for (int y = 0; y < WORLD_HEIGHT; y++) {
        // fill_progress_bar(y, WORLD_HEIGHT + WORLD_HEIGHT);
        for (int z = 0; z < WORLD_SIZE; z++) {
            for (int x = 0; x < WORLD_SIZE; x++) {
                if (world->blocks[y][x][z] > WATER) {
                    world->set_block_shadow(x, y, z);
                }
            }
        }
    }

    // progress_bar("Initializing blocks...");
    //  Initialize the triangle grid with data from the world
    for (int y = 0; y < WORLD_HEIGHT; y++) {
        // fill_progress_bar(y + WORLD_HEIGHT, WORLD_HEIGHT + WORLD_HEIGHT);
        for (int z = WORLD_SIZE - 1; z >= 0; z--) {
            for (int x = WORLD_SIZE - 1; x >= 0; x--) {
                if (world->blocks[y][x][z] == WATER) {
                    world->set_water(x, y, z);
                } else if (world->blocks[y][x][z] != AIR) {
                    world->set_block(x, y, z, world->blocks[y][x][z]);
                }
            }
        }
    }

    for (int i = 0; i < LCD_CNT; i++) {
        back_buffer[i] = tex_palette[SKY];
    }

    // th
    draw_x0 = 0;
    draw_y0 = 0;
    draw_x1 = SCREEN_WIDTH;
    draw_y1 = SCREEN_HEIGHT;

    // gfx_SetDrawBuffer();
    draw_tri_grid(*world);

    player.draw();
}

void play(uint8_t world_id) {
    // We store the world data at the contiguous 69K of Safe RAM
    // as listed here:
    // https://wikiti.brandonw.net/index.php?title=Category:84PCE:RAM:By_Address
    // world_t* world = (world_t*)0xD05350;

    // Idk if the nspire has something like that, let's use malloc!
    world_t* world = (world_t*)malloc(sizeof(world_t));
    if (!world) {
        _show_msgbox("Error",
                     "There was an error while allocating memory for the world. Do you need to restart your device in "
                     "order to free enough RAM to use this program?",
                     0);
        return;
    }
    player_t player;
    player.current_block = STONE;

    // ohg
    player.x = 0;
    player.y = 0;
    player.z = 0;

    init_play(world_id, world, player);

    int scroll_goal_x = scroll_x;
    int scroll_goal_y = scroll_y;

    player.world = world;

    lcd_blit(back_buffer, current_lcd_type);
    do {
        wait_key_pressed();

        // Viewport scrolling
        if (isKeyPressed(KEY_NSPIRE_LEFT)) {
            scroll_goal_x += SCROLL_SPEED;
        } else if (isKeyPressed(KEY_NSPIRE_RIGHT)) {
            scroll_goal_x -= SCROLL_SPEED;
        }
        if (isKeyPressed(KEY_NSPIRE_DOWN)) {
            scroll_goal_y -= SCROLL_SPEED;
        } else if (isKeyPressed(KEY_NSPIRE_UP)) {
            scroll_goal_y += SCROLL_SPEED;
        }

        // Player movement
        if (isKeyPressed(KEY_NSPIRE_7)) {
            player.move(0, 0, 1);
            player.scroll_to_contain(scroll_goal_x, scroll_goal_y);
        } else if (isKeyPressed(KEY_NSPIRE_8)) {
            player.move(1, 0, 1);
            player.scroll_to_contain(scroll_goal_x, scroll_goal_y);
        } else if (isKeyPressed(KEY_NSPIRE_9)) {
            player.move(1, 0, 0);
            player.scroll_to_contain(scroll_goal_x, scroll_goal_y);
        } else if (isKeyPressed(KEY_NSPIRE_4)) {
            player.move(-1, 0, 1);
            player.scroll_to_contain(scroll_goal_x, scroll_goal_y);
        } else if (isKeyPressed(KEY_NSPIRE_6)) {
            player.move(1, 0, -1);
            player.scroll_to_contain(scroll_goal_x, scroll_goal_y);
        } else if (isKeyPressed(KEY_NSPIRE_1)) {
            player.move(-1, 0, 0);
            player.scroll_to_contain(scroll_goal_x, scroll_goal_y);
        } else if (isKeyPressed(KEY_NSPIRE_2)) {
            player.move(-1, 0, -1);
            player.scroll_to_contain(scroll_goal_x, scroll_goal_y);
        } else if (isKeyPressed(KEY_NSPIRE_3)) {
            player.move(0, 0, -1);
            player.scroll_to_contain(scroll_goal_x, scroll_goal_y);
        } else if (isKeyPressed(KEY_NSPIRE_MULTIPLY)) {
            player.move(0, 1, 0);
            player.scroll_to_contain(scroll_goal_x, scroll_goal_y);
        } else if (isKeyPressed(KEY_NSPIRE_MINUS)) {
            player.move(0, -1, 0);
            player.scroll_to_contain(scroll_goal_x, scroll_goal_y);
        }

        // Block placement or removal
        if (isKeyPressed(KEY_NSPIRE_5)) {
            // Initialize our update region to be empty
            empty_draw_region();

            // Place or remove block will expand the update region to contain all updated
            // blocks (where a shadow is cast or uncast)
            if (player.current_block != WATER) {
                if (world->blocks[player.y][player.x][player.z] == AIR) {
                    world->place_block(player.x, player.y, player.z, player.current_block);
                }
                // Just replace water with solid blocks when placing (double tap 5 to replace water with air)
                else if (world->blocks[player.y][player.x][player.z] == WATER) {
                    world->remove_block(player.x, player.y, player.z);
                    world->place_block(player.x, player.y, player.z, player.current_block);
                } else {
                    world->remove_block(player.x, player.y, player.z);
                }
            } else {
                if (world->blocks[player.y][player.x][player.z] == AIR) {
                    world->set_water(player.x, player.y, player.z);
                    expand_draw_region(player.x, player.y, player.z);
                } else {
                    world->remove_block(player.x, player.y, player.z);
                }
            }

            // Redraw the section of the screen where updates occurred and the cursor on top of that
            draw_tri_grid(*world);
            player.draw();
        }

        // Change the currently selected block
        if (isKeyPressed(KEY_NSPIRE_ENTER)) {
            // TODO: implement
            // player.current_block = block_select(player.current_block);
        }

        // Compute the motion needed to reach our scroll target
        int scroll_step_x = scroll_goal_x - scroll_x;
        int scroll_step_y = scroll_goal_y - scroll_y;

        // Clamp it to the max scroll speed
        if (scroll_step_x > SCROLL_SPEED) scroll_step_x = SCROLL_SPEED;
        if (scroll_step_x < -SCROLL_SPEED) scroll_step_x = -SCROLL_SPEED;

        if (scroll_step_y > SCROLL_SPEED) scroll_step_y = SCROLL_SPEED;
        if (scroll_step_y < -SCROLL_SPEED) scroll_step_y = -SCROLL_SPEED;

        if (scroll_step_x != 0 || scroll_step_y != 0) {
            scroll_view(*world, scroll_step_x, scroll_step_y);
            // player.draw();
        }

        if (isKeyPressed(KEY_NSPIRE_A)) {
            for (int i = 0; i < LCD_CNT; i++) {
                back_buffer[i] = tex_palette[SKY];
            }

            draw_x0 = 0;
            draw_x1 = SCREEN_WIDTH;
            draw_y0 = 0;
            draw_y1 = SCREEN_HEIGHT;

            draw_tri_grid(*world);
        }

        lcd_blit(back_buffer, current_lcd_type);

    } while (!isKeyPressed(KEY_NSPIRE_ESC));

    // init_ui_palette();
    // gfx_SetDrawScreen();

    // gfx_FillScreen(1);
    // progress_bar("Saving...");

    // save(world_id, *world, player);

    free(world);
}

/*
void world_select() {
    uint8_t selection = 0;

    while (true) {
        gfx_SetDrawScreen();

        gfx_SetColor(4);
        gfx_FillRectangle(UI_BORDER, UI_BORDER, LCD_WIDTH - 2 * UI_BORDER, LCD_HEIGHT - 2 * UI_BORDER);

        gfx_SetColor(0);
        gfx_Rectangle(UI_BORDER, UI_BORDER, LCD_WIDTH - 2 * UI_BORDER, LCD_HEIGHT - 2 * UI_BORDER);

        char name[8] = "World A";
        char filename[8] = "WORLDA";

        for (uint8_t i = 0; i < SAVE_CNT; i++) {
            name[6] = 'A' + i;
            filename[5] = 'A' + i;

            ti_var_t var = ti_Open(filename, "r");
            if (var == 0) {
                gfx_SetTextFGColor(3);
                gfx_PrintStringXY(name, UI_BORDER + 16, UI_BORDER + 16 + i * 32);
                gfx_SetTextFGColor(3);
                gfx_PrintStringXY("( Empty )", UI_BORDER + 24, UI_BORDER + 24 + i * 32);
            } else {
                gfx_SetTextFGColor(0);
                gfx_PrintStringXY(name, UI_BORDER + 16, UI_BORDER + 16 + i * 32);
                gfx_SetTextFGColor(3);
                gfx_PrintStringXY("48x16x48", UI_BORDER + 24, UI_BORDER + 24 + i * 32);
                ti_Close(var);
            }
        }

        gfx_SetTextFGColor(0);
        gfx_PrintStringXY("Quit", UI_BORDER + 16, LCD_HEIGHT - UI_BORDER - 16 - 8);

        uint8_t selection_old = selection ^ 1;

        sk_key_t key;

        do {
            if (selection != selection_old) {
                gfx_SetColor(4);
                gfx_Rectangle(UI_BORDER + 8, UI_BORDER + 12 + selection_old * 32,
                              LCD_WIDTH - UI_BORDER - UI_BORDER - 8 - 8, 24);

                gfx_SetColor(3);
                gfx_Rectangle(UI_BORDER + 8, UI_BORDER + 12 + selection * 32, LCD_WIDTH - UI_BORDER - UI_BORDER - 8 - 8,
                              24);
            }

            selection_old = selection;

            key = os_GetCSC();

            switch (key) {
                case sk_Down:
                    if (selection < SAVE_CNT) selection++;
                    break;
                case sk_Up:
                    if (selection > 0) selection--;
                    break;
                case sk_Enter:
                    // Break from the program when the last option (quit) is selected
                    if (selection == SAVE_CNT) return;
                    play(selection);
                    break;

                case sk_Del: {
                    const char* options[3] = {"Are you sure?", "No", "Yes"};

                    if (menu(options, 2)) erase(selection);
                } break;

                default:
                    break;
            }

            // Exit the inner loop every time something gets selected
        } while (key != sk_Enter && key != sk_Del);
    }
}*/

void init() {
    // Set right face textures to always to be in shadow
    for (int i = 0; i < TEX_CNT; i++) {
        for (int j = 0; j < TEX_SIZE; j++) {
            textures[i][RIGHT_FACE * 2 + 0][j] |= SHADOW;
            textures[i][RIGHT_FACE * 2 + 1][j] |= SHADOW;
        }
    }

    /* Initialize graphics drawing */
    // gfx_Begin();
    // gfx_SetDrawScreen();

    // init_ui_palette();

    // world_select();
    play(0);
}

const char* scr_type_to_string(scr_type_t type) {
    switch (type) {
        case SCR_320x240_565:
            return "SCR_320x240_565";
        case SCR_320x240_4:
            return "SCR_320x240_4";
        case SCR_240x320_565:
            return "SCR_240x320_565";
        case SCR_320x240_16:
            return "SCR_320x240_16";
        case SCR_320x240_8:
            return "SCR_320x240_8";
        case SCR_320x240_555:
            return "SCR_320x240_555";
        case SCR_240x320_555:
            return "SCR_240x320_555";
        case SCR_TYPE_INVALID:
            return "SCR_TYPE_INVALID";
        default:
            return "UNKNOWN_SCR_TYPE";
    }
}

int main(void) {
    // Unnecessary (I think?) for the Ti Nspire
    // static_assert(sizeof(world_t) < 69090, "World_t size is too big for the specified SafeRAM area!");

    current_lcd_type = lcd_type();

    lcd_init(current_lcd_type);

    if (current_lcd_type != SCR_320x240_565 && current_lcd_type != SCR_320x240_16) {
        _show_msgbox("Incorrect screen type",
                     "This program requires your screen to be SCR_320x240_565 or SCR_320x240_16, your screen is: "
                     "(press OK to continue)",
                     0);
        _show_msgbox("Your screen is:", scr_type_to_string(current_lcd_type), 0);

        lcd_init(SCR_TYPE_INVALID);  // Reset screen mode
        return 0;
    }

    // char string[20];
    // sprintf(string, "%lu", (unsigned long)sizeof(int));
    //_show_msgbox("sizeof(int) is equal to:", string, 0);

    init();

    // gfx_End();

    lcd_init(SCR_TYPE_INVALID);  // Reset screen mode
    return 0;
}