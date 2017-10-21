/*
 * main_chip.c
 * must select before the user kit
 */

#include "system.h"
#include "navswitch.h"
#include "pacer.h"
#include "tinygl.h"
#include "../fonts/font5x7_1.h"
#include "../fonts/font3x5_1.h"
#include "ir_uart.h"
#include "led.h"
#include "ledmat.h"
#include "string.h"
#include "button.h"

#define PACER_RATE 450
#define MESSAGE_RATE 10

void welcome(void)
{
    tinygl_text_dir_set (TINYGL_TEXT_DIR_ROTATE);
    tinygl_font_set (&font3x5_1);
    tinygl_text("GO");

    int game_on_flag = 0;
    while(game_on_flag == 0) {
        tinygl_update();
        if(button_pressed_p()) {
            game_on_flag = 1;
        }
        pacer_wait();
    }
}

static char select_loop(void)
{
    int select_flag = 0;
    int select_index = 0;
    char const operation_character[3] = {'P', 'R', 'S'};
    tinygl_font_set (&font5x7_1);
    tinygl_text_dir_set (TINYGL_TEXT_DIR_NORMAL);

    while (select_flag == 0) {
        navswitch_update();
        if(navswitch_push_event_p(NAVSWITCH_NORTH)) {
            select_index++;
        } else if (navswitch_push_event_p(NAVSWITCH_SOUTH)) {
            if (select_index >= 1) {
                select_index--;
            }
        } else if (navswitch_push_event_p(NAVSWITCH_PUSH)) {
            return operation_character[select_index % 3];
        }
        tinygl_text(&operation_character[select_index % 3]);
        pacer_wait();
        tinygl_update();
    }
}

static char get_other_select(void)
{
    char other_select = '\0';
    tinygl_font_set (&font3x5_1);
    tinygl_text_dir_set (TINYGL_TEXT_DIR_ROTATE);
    tinygl_text("OK");
    while(other_select == '\0') {
        pacer_wait();
        tinygl_update ();
        if(ir_uart_read_ready_p()) {
            other_select = ir_uart_getc();
        }
    }

    return other_select;
}

int win_or_lose(char my_select, char other_select)
{
    int result = 2;
    if (my_select == 'R') {
        if (other_select == 'S') {
            result = 1;
        } else if (other_select == 'P') {
            result = 0;
        }

    } else if (my_select == 'S') {
        if (other_select == 'R') {
            result = 0;
        } else if (other_select == 'P') {
            result = 1;
        }
    } else if (my_select == 'P') {
        if (other_select == 'R') {
            result = 1;
        } else if (other_select == 'S') {
            result = 0;
        }
    }
    ir_uart_putc (result);
    return result; // even
}

void loser_winner(int is_me_win)
{
    tinygl_text_mode_set (TINYGL_TEXT_MODE_SCROLL);
    if (is_me_win == 1) {
        tinygl_text ("WINNER! ");
        while(1) {
            led_toggles();
            pacer_wait();
            tinygl_update();
        }
    } else if (is_me_win == 0) {
        tinygl_text ("LOSER! ");
        while(1) {
            led_on();
            tinygl_update();
            pacer_wait();
            tinygl_update();
        }
    } else {
        tinygl_text ("EVEN! ");
        while(1) {
            tinygl_update();
            pacer_wait();
            tinygl_update();
        }
    }
}

void init_all(void)
{
    system_init (); // init system
    navswitch_init(); // init switch
    ledmat_init (); // init ledmat
    led_init (); // init LED
    ir_uart_init(); // init ir
    button_init(); // init button
    tinygl_init (PACER_RATE); // init display
    tinygl_text_speed_set (MESSAGE_RATE);
    tinygl_text_mode_set (TINYGL_TEXT_MODE_STEP);

    pacer_init (PACER_RATE); // init pacer
}

int main (void)
{
    init_all();
    welcome();

    int is_me_win = 0;

    while (1) {
        char my_select = '\0';
        char other_select = '\0';

        my_select = select_loop();  // select

        other_select = get_other_select(); // read from other

        is_me_win = win_or_lose(my_select, other_select); // calc result

        loser_winner(is_me_win); // print out and active LED
    }
    return 0;
}
