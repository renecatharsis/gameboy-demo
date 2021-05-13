#include <stdio.h>
#include <rand.h>
#include <gb/gb.h>
#include <gb/font.h>
#include "gbtplayer/gbt_player.h"

#include "sprites/Hero.c"
#include "sprites/Items.c"
#include "sprites/SplashFont.c"
#include "sprites/BackgroundSprites.c"
#include "backgrounds/Backgrounds.c"
#include "backgrounds/splash_data.c"
#include "backgrounds/splash_map.c"
#include "windows/Window.c"
#include "HeroCharacter.c"
#include "Coin.c"

extern const unsigned char * song_Data[];
struct HeroCharacter hero;
struct Coin coins[5];

UBYTE hero_direction_left = 0;
UBYTE hero_sprite_size = 8;
UBYTE splash_text_displayed = 1;
UINT8 splash_text_timer = 0;
UINT8 coin_drop_timer = 0;
UINT8 coin_drop_iterator = 0;
UINT8 coin_min_x = 8;
UINT8 coin_max_x = 136; // save 8 pixels for the sprite itself
UINT8 coin_min_y = 40;
UINT8 coin_max_y = 128;
UINT8 coin_min_y_visibility = 48;
UINT8 coin_max_y_visibility = 112;
UINT8 random_seed[100] = {
    10,46,79,135,129,124,47,5,44,125,43,14,87,105,78,99,63,21,44,13,
    84,65,113,94,60,92,124,78,35,48,32,33,63,48,123,96,61,110,21,7,
    118,102,31,46,24,65,38,101,58,32,66,42,75,55,11,94,8,42,20,118,
    5,66,0,9,52,61,10,65,26,68,45,74,83,26,72,28,15,93,3,30,
    62,49,43,113,32,131,48,64,126,74,12,80,71,61,125,44,11,10,2,86
};
UBYTE jumping = 0;
UINT8 jump_max_y = 104;
INT8 gravity = -2;
UINT16 current_speed_y;

void wait(UINT8 duration);
void fadein();
void fadeout();
void init_hero();
void init_coins();
void move_hero(struct HeroCharacter* hero, UINT8 x, UINT8 y);
void init_music();
void init_splash();
void clear_splash();
void toggle_splash_text(int sprite, UBYTE on);
void interrupt_tim_splash();
void interrupt_tim_coins();
void init_gamescreen();
void jump(struct HeroCharacter* hero);
INT8 wouldhitsurface(INT16 projectedYPosition);

void main() {
    init_splash();

    SHOW_SPRITES;
    SHOW_BKG;
    DISPLAY_ON;

    waitpad(J_START | J_A);

    clear_splash();
    fadeout();

    init_music();
    init_gamescreen();

    SHOW_WIN;

    fadein();

    while (1) {
        if (joypad() & J_LEFT) {
            move_hero(&hero, hero.x - 1, hero.y);
        } else if (joypad() & J_RIGHT) {
            move_hero(&hero, hero.x + 1, hero.y);
        }

        if ((joypad() & J_A) || (joypad() & J_B) || jumping == 1) {
            jump(&hero);
        }

        wait(1);
        gbt_update();
    }
}

void wait(UINT8 duration) {
    UINT8 i;

    for (i = 0; i < duration; i++) {
        wait_vbl_done();
    }
}

void fadeout() {
    UINT8 i;

    for (i = 0; i < 4; i++) {
        switch (i) {
            case 0:
                BGP_REG = 0xE4;
                break;
            case 1:
                BGP_REG = 0xF9;
                break;            
            case 2:
                BGP_REG = 0xFE;
                break;            
            case 3:
                BGP_REG = 0xFF;
                break;            
        }

        wait(10);
    }
}

void fadein() {
    UINT8 i;

    for (i = 0; i < 3; i++) {
        switch (i) {
            case 0:
                BGP_REG = 0xFE;
                break;
            case 1:
                BGP_REG = 0xF9;
                break;            
            case 2:
                BGP_REG = 0xE4;
                break;            
        }

        wait(10);
    }
}

void init_music() {
    disable_interrupts();

    gbt_play(song_Data, 2, 7);
    gbt_loop(1);

    set_interrupts(VBL_IFLAG);
    enable_interrupts();    
}

void move_hero(struct HeroCharacter* hero, UINT8 x, UINT8 y) {
    if ((hero_direction_left && hero->x < x) || (!hero_direction_left && hero->x > x)) {
        UBYTE tmp_top = hero->sprites[0];
        UBYTE tmp_bottom = hero->sprites[2];

        hero->sprites[0] = hero->sprites[1];
        hero->sprites[1] = tmp_top;
        hero->sprites[2] = hero->sprites[3];
        hero->sprites[3] = tmp_bottom;

        UINT8 i;
        if (hero_direction_left) {
            for (i = 0; i < 4; i++) {
                set_sprite_prop(hero->sprites[i], get_sprite_prop(hero->sprites[i]) & ~S_FLIPX);
            }
        } else {
            for (i = 0; i < 4; i++) {
                set_sprite_prop(hero->sprites[i], get_sprite_prop(hero->sprites[i]) | S_FLIPX);
            }            
        }

        hero_direction_left = !hero_direction_left;
    }

    hero->x = x;
    hero->y = y;

    move_sprite(hero->sprites[0], x, y);
    move_sprite(hero->sprites[1], x + hero_sprite_size, y);
    move_sprite(hero->sprites[2], x, y + hero_sprite_size);
    move_sprite(hero->sprites[3], x + hero_sprite_size, y + hero_sprite_size);
}

INT8 wouldhitsurface(INT16 target_y) {
    if (target_y >= jump_max_y) {
        return jump_max_y;
    }

    return -1;
}

void jump(struct HeroCharacter* hero) {
    INT8 lowest_floor_y;

    if (jumping == 0) {
        jumping = 1;
        current_speed_y = 10;
    }

    // work out current speed - effect of gravities accelleration down
    current_speed_y += gravity;    
    hero->y -= current_speed_y;

    lowest_floor_y = wouldhitsurface(hero->y);

    if (lowest_floor_y != -1){
        jumping = 0;
        move_hero(hero, hero->x, lowest_floor_y);
    } else{
        move_hero(hero, hero->x, hero->y);
    }
}

void init_hero() {
    set_sprite_data(0, 5, Hero);
    set_sprite_tile(0, 0);
    set_sprite_tile(1, 1);
    set_sprite_tile(2, 2);
    set_sprite_tile(3, 3);
    set_sprite_tile(4, 4);

    hero.x = 80;
    hero.y = 104;
    hero.width = 16;
    hero.height = 16;
    hero.sprites[0] = 1;
    hero.sprites[1] = 2;
    hero.sprites[2] = 3;
    hero.sprites[3] = 4;

    move_hero(&hero, hero.x, hero.y);
}

void toggle_splash_text(int sprite, UBYTE on) {
    if (on) {
        set_sprite_prop(sprite, get_sprite_prop(sprite) | S_PRIORITY);
    } else {
        set_sprite_prop(sprite, get_sprite_prop(sprite) & ~S_PRIORITY);
    }
}

void interrupt_tim_splash() {
    splash_text_timer++;
    int counter = 0;

    if (splash_text_timer == 16) {
        for (counter = 0; counter < 11; counter++) {
            toggle_splash_text(counter, splash_text_displayed);
        }

        splash_text_displayed = !splash_text_displayed;
        splash_text_timer = 0;
    }
}

void init_splash() {
    set_bkg_data(0, 120, splash_data);
    set_bkg_tiles(0, 0, 20, 18, splash_map);

    set_sprite_data(0, 7, SplashFont);
    set_sprite_tile(0, 3);
    set_sprite_tile(1, 4);
    set_sprite_tile(2, 2);
    set_sprite_tile(3, 5);
    set_sprite_tile(4, 5);
    set_sprite_tile(5, 5);
    set_sprite_tile(6, 6);
    set_sprite_tile(8, 1);
    set_sprite_tile(9, 4);
    set_sprite_tile(10, 6);

    move_sprite(0, 45, 145);
    move_sprite(1, 53, 145);
    move_sprite(2, 61, 145);
    move_sprite(3, 69, 145);
    move_sprite(4, 77, 145);
    move_sprite(5, 93, 145);
    move_sprite(6, 101, 145);
    move_sprite(8, 109, 145);
    move_sprite(9, 117, 145);
    move_sprite(10, 125, 145);

    TMA_REG = 0x00U;
    TAC_REG = 0x04U;
    disable_interrupts();
    add_TIM(interrupt_tim_splash);
    enable_interrupts();

    set_interrupts(VBL_IFLAG | TIM_IFLAG);
}

void clear_splash() {
    disable_interrupts();
    remove_TIM(interrupt_tim_splash);
    enable_interrupts();

    set_interrupts(VBL_IFLAG);

    int i;
    for (i = 0; i < 11; i++) {
        toggle_splash_text(i, 1);
        set_sprite_tile(i, 0);
    }
}

void interrupt_tim_coins() {
    UINT8 i;
    UINT8 x;

    coin_drop_timer++;
    // sprite 5-9 are the coins
    for (i = 5; i < 10; i++) {
        if (coins[i - 5].x == NULL) {
            x = random_seed[coin_drop_iterator];
            
            if (x < coin_min_x || x >= coin_max_x) {
                x = coin_min_x;
            }

            coins[i - 5].x = x;
        }

        if (coins[i - 5].y == NULL) {
            coins[i - 5].y = coin_min_y;
        }

        coins[i - 5].y += coin_drop_timer;

        if (coins[i - 5].y < coin_min_y_visibility || coins[i - 5].y > coin_max_y_visibility) {
            set_sprite_prop(coins[i - 5].sprite, get_sprite_prop(coins[i - 5].sprite) | S_PRIORITY);
        } else {
            set_sprite_prop(coins[i - 5].sprite, get_sprite_prop(coins[i - 5].sprite) & ~S_PRIORITY); 
        }

        if (coins[i - 5].y >= coin_max_y) {
            coin_drop_timer = 0;

            coins[i - 5].x = NULL;
            coins[i - 5].y = NULL;
        }

        move_sprite(coins[i - 5].sprite, coins[i - 5].x, coins[i - 5].y);
    }

    coin_drop_iterator++;
    if (coin_drop_iterator > 100) {
        coin_drop_iterator = 0;
    }
}

void init_coins() {
    UINT8 i;
    set_sprite_data(5, 2, Items);

    for (i = 5; i < 10; i++) {
        coins[i - 5].sprite = i;

        set_sprite_tile(i, 6);
        set_sprite_prop(i, get_sprite_prop(i) & ~S_PRIORITY);
    }

    TMA_REG = 0x00U;
    TAC_REG = 0x04U;
    disable_interrupts();
    add_TIM(interrupt_tim_coins);
    enable_interrupts();

    set_interrupts(VBL_IFLAG | TIM_IFLAG);
}

void init_gamescreen() {
    font_t min_font;

    font_init();
    min_font = font_load(font_min);
    font_set(min_font);

    set_bkg_data(37, 5, BackgroundSprites);
    set_bkg_tiles(0, 0, 20, 18, Backgrounds);

    set_win_tiles(17, 0, 3, 1, WindowUpper);
    set_win_tiles(0, 1, 40, 1, WindowLower);
    move_win(7, 128);    

    init_hero();
    init_coins();
}