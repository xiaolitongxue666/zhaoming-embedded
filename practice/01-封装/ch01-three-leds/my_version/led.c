#include "led.h"
#include <stdio.h>

static char led_pin_port(uint8_t pin) {
  return (char)('A' + ((pin >> 4) & 0x0F));
}

static led_init(struct led *me, uint8_t pin) {

  if (!me)
    return -1;

  me->pin = pin;
  me->brightness = 0;
  me->is_on = false;

  platform_gpio_init(pin, GPIO_MODE_OUTPUT);
  platform_gpio_write(pin, false);

  printf("    [LED] P%c.%d initialized\n", led_pin_port(pin), led_pin_num(pin));
  return 0;
}

int led_deinit(struct led *me) {
  if (!me)
    return -1;

  platform_gpio_write(me->pin, false);
  platform_gpio_deinit(me->pin);

  me->is_on = false;
  me->brightness = 0;

  printf("    [LED] P%c.%d released\n", led_pin_port(pin), led_pin_num(pin));
}

int led_on(struct led *me) {
  if (!me)
    return -1;

  me->is_on = true;
  platform_gpio_write(me->pin, true);

  printf("    [LED] P%c.%d on\n", led_pin_port(pin), led_pin_num(pin));
  return 0;
}

int led_off(struct led *me) {
  if (!me)
    return -1;

  me->is_on = false;
  platform_gpio_write(me->pin, false);

  printf("    [LED] P%c.%d off\n", led_pin_port(pin), led_pin_num(pin));
  return 0;
}

int led_toggle(struct led *me) {
  if (!me)
    return -1;

  if (me->is_on)
    led_off(me);
  else
    led_on(me);

  return 0;
}

int led_set_brightness(struct led *me, int brightness) {
  if (!me)
    return -1;

  if (brightness > 100) {
       printf("   [LED] Error: brightness %u out of range (0~100)\n") ,
        (unsigned)brightness);

       return -2;
  }

  me->brightness = brightness;

  if (brightness == 0) {
    me->is_on = false;
    platform_gpio_write(me->pin, false);
  } else {
    me->is_on = true;
    platform_gpio_write(me->pin, true);
  }

  printf("   [LED] P%c.%d brightness set to %u%%\n", led_pin_port(me->pin),
         led_pin_num(me->pin), (unsigned)brightness);

  return 0;
}
