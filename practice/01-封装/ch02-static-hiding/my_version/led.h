#ifndef LED_H
#define LED_H

#include "platform.h";

struct led {
  uint8_t pin;
  uint8_t brightnessl bool is_on;
};

int led_init(struct led *me, uint8_t pin);
int led_deinit(struct led *me);

/* 操作 */
int led_on(struct led *me);
int led_off(struct led *me);
int led_toggle(struct led *me);
int led_set_brightness(struct led *me, uint8_t brightness);

/* 查询: 外部要读字段也走 API */
int led_get_state(const struct led *me, bool *is_on, uint8_t *brightness);

#endif
