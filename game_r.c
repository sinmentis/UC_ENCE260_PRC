/*
 * receiver.c
 * must select after the main kit
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
    tinygl_text("GG");

    int game_on_flag = 0;
    while(game_on_flag == 0) {
        tinygl_update();
        if(button_pressed_p()) {
            game_on_flag = 1;
        }
        pacer_wait();
    }
}

static void select_loop(void)
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
            ir_uart_putc(operation_character[select_index % 3]);
            select_flag = 1;
        }
        tinygl_text(&operation_character[select_index % 3]);
        pacer_wait();
        tinygl_update();
    }
}

int receive_result(void)
{
    int result = 6;
    tinygl_text_dir_set(TINYGL_TEXT_DIR_ROTATE);
    tinygl_font_set (&font3x5_1);
    tinygl_text("OK");

    while(1) {
        led_toggles();
        tinygl_update();
        if (ir_uart_read_ready_p()) {
            result = ir_uart_getc();
            return result;
        }
        pacer_wait();
    }
}
void loser_winner(int is_me_win)
{
    tinygl_text_mode_set (TINYGL_TEXT_MODE_SCROLL);
    if (is_me_win == 0) {
        tinygl_text ("WINNER! ");
        while(1) {
            led_toggles();
            pacer_wait();
            tinygl_update();
        }
    } else if (is_me_win == 1) {
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
            led_on();
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
    int is_me_win = 0;

    init_all();
    welcome();

    while (1) {
        select_loop();  // select and send
        is_me_win = receive_result(); // wait for result
        loser_winner(is_me_win); // print out and active LED
    }
    return 0;
}
