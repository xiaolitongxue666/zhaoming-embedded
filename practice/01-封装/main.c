#include "led.h"
#include <stdio.h>

int main(void) {
  struct led red_led;
  struct led green_led;
  struct led blue_led;

  printf("========================================\n");
  printf("  Three LEDs, one set of code.\n");
  printf("  me pointer decides who to operate.\n");
  printf("========================================\n\n");

  printf("--- Init ---\n");
    led_init(&red_led, PIN_NUM('A', 13);
    led_init(&green_led, PIN_NUM('A', 14);
    led_init(&blue_led, PIN_NUM('A', 15);


    printf("\n---Turn on RED -- \n");
    led_on(&red_led);


    printf("\n---Turn on GREEN -- \n");
    led_on(&green_led);

    printf("\n---Turn on BLUE -- \n");
    led_on(&blue_led);

    led_off(&red_led);

    let_set_brightness(&blue_led, 0);

    printf("\n--- Cleanup ---\n");
    led_deinit(&red_led);
    led_deinit(&green_led);
    led_deinit(&blue_led);

    return 0;
}
