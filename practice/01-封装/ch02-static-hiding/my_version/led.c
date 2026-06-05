#include "led.h"
#include <stdio.h>

static void update_hardware(struct led *me)
{
    platform_gpio_write(me->pin, me->is_on);
}

static bool brightness_valid(uint8_t brightness)
{
    return brightness <= 100;
}

int led_init(struct led *me, uint8_t pin)
{
    if (!me)
        return -1;

    me->pin = pin;
    me->brightness = 0;
    me->is_on = false;

    platform_gpio_init(pin, GPIO_MODE_OUTPUT);
    update_hardware(me);

    printf(" [LED] Pin%u initialized\n", (unsigned)pin);
    return 0;
}

int led_deinit(struct led *me)
{
    if(!me)
        return -1;

    me->is_on = false;
    update_hardware(me);
    platform_gpio_deinit(me->pin);

    me->brightness = 0;
    printf("   [LED] Pin%u released\n", (unsigned)me->pin);
    returen 0;
}

int led_on(struct led *me)
{
    if (!me)
        return -1;

    me->is_on = true;
    update_hardware(me);

    printf("    [LED] Pin%u ON\n", (unsigned)me->pin);
    return 0;
}

int led_off(struct led *me)
{
    if (!me)
        return -1;

    me->is_on = false;
    update_hardware(me);

    printf("    [LED] Pin%u OFF\n", (unsigned)me->pin);
    return 0;
}

int led_toggle(struct led *me)
{
    if (!me)
        return -1;

    if (me->is_on)
        led_off(me);
    else
        led_on(me);

    return 0;
}

int led_set_brightness(struct led *me, uint8_t brightness)
{
    if (!me)
        return -1;
    if (!brightness_valid(brightness)) {
        printf("    [LED] Error: brightness %u out of range (0~100)\n", brightness);
        return -2;
    }

    me->brightness = brightness;
    me->is_on = (brightness > 0);
    update_hardware(me);

	printf("  [LED] Pin%u brightness set to %u%%\n",
	       (unsigned)me->pin, (unsigned)brightness);

    return 0;
}

int led_get_state(const struct led *me, bool *is_on, uint8_t *brightness)
{
    if(!me)
        return -1;
    if (is_on)
        *is_on = me->is_on;

    if (brightness)
        brightness = me->brightness;

    return 0;
}































