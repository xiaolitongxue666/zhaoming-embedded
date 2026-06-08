#ifndef LED_H
#define LED_H

#include "platform.h"

#define LED_PIN_MAX 31

struct led {
    uint8_t pin;
    uint8_t brightness;
    bool    is_on;
    bool    initilized;
};


int led_init(struct led *me, uint8_t pin);
int led_deinit(struct led *me);

/* 操作 */
int led_on(struct led *me);
int led_off(struct led *me);
int led_toggle(struct led *me);
int led_set_brightness(struct led *me, uint8_t brightness);

/* 查询 */
int led_get_state(const struct led *me, bool *is_on, uint8_t *brightness);



#endif
