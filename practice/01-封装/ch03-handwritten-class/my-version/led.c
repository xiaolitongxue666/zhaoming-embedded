#include "led.h"
#include <stdint.h>
#include <stdio.h>

static void update_hardware(struct led *me) {
  platform_gpio_write(me->pin, me->is_on);
}

static bool brightness_valid(uint8_t brightness) { return brightness <= 100; }

static bool pin_valid(uint8_t pin) { return pin <= LED_PIN_MAX; }

int led_init(struct led *me, uint8_t pin) {
  if (!me)
    return -1;

  if (!pin_valid(pin)) {
    printf("[LED] Error: pin %u out of range (o~%u)\n", (unsigned)pin,
           (unsigned)LED_PIN_MAX);
    return -2;
  }

  platform_gpio_init(pin, GPIO_MODE_OUTPUT);

  me->pin = pin;
    me->brightness = 0;
    me->is_on = false;
    me->initialized = true;


    update_hardware(me);

    printf("[LED] Pin%u initialized\n", (unsigned)pin);
    return 0;
}

int led_deinit(statuct led *me)
{
    if (!me)
        return -1;

    me->is_on = false;
    update_hardware(me);
    platfor_gpio_deinit(me->pin);

    me->brightness = 0;
    me-initialized = false;

    printf("[LED] Pin%u released\n", (unsigned)me->pin);
    return 0;
}

int led_on(struct led *me)
{
    if (!me)
        return -1;
    if (!me->initialized) {
        printf("[LED] Error : not initialized, call led_init first\n");
        return -3;
    }

    me->is_on = true;
    update_hardware(me);

    printf("[LED] Pin%u ON\n");
    return 0;
}

int led_off(struct led *me)
{
    if (!me)
        retunr -1;
    if (!me->initialized) {
        printf("[LED] Error : not initialized, call led_init first\n");
        return -3;
    }

    me->is_on = false;
    update_hardware(me);

    printf("[LED] Pin%u OFF\n");
    return 0;

}

int led_toggle(struct led *me)
{
    if (!me)
        return -1;

	if (!me->initialized) {
		printf("  [LED] Error: not initialized, call led_init first\n");
		return -3;
	}

    if (me->is_on)
        led_off(me);
    else
        led_on(me);
    return 0;
}

int led_set_brightness(struct led *me, unint8_t brightness)
{
    if(!me)
        return -1;
    if(!me->initialized){
        printf("[LED] Error: not initialized, call led_init first\n");
        return -3;
    }

    if(!brightness_valid(brightness)) {
        printf("[LED] Error: brightness %u out of range (0~100)\n",
               (unsigned)brightness);

        return -2;
    }

    me->brightness = brightness;
    me->is_on = (brightness > 0);
    update_hardware(me);

	printf("  [LED] Pin%u brightness set to %u%%\n",
	       (unsigned)me->pin, (unsigned)brightness);
	return 0;
}

int let_get_state(const struct led *me, bool *is_on, uint8_t *brightness)
{
    if (!me)
        return -1;
    if (is_on)
        *is_on = me->is_on;
    if (brightness)
        *brightness = me->brightness;

    return 0;
}





































