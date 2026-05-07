/* Minimal syntax stub for fsl_gpio.h - syntax-check only, NOT for build */
#ifndef FSL_GPIO_H
#define FSL_GPIO_H

#include <stdint.h>

typedef int status_t;
#define kStatus_Success 0

typedef struct {
	uint32_t volatile DR;
	uint32_t volatile GDIR;
} GPIO_Type;

typedef enum {
	kGPIO_DigitalInput = 0,
	kGPIO_DigitalOutput = 1,
} gpio_pin_direction_t;

typedef struct {
	gpio_pin_direction_t direction;
	uint8_t              outputLogic;
} gpio_pin_config_t;

#define GPIO1 ((GPIO_Type *)0x401B8000UL)
#define GPIO2 ((GPIO_Type *)0x401BC000UL)
#define GPIO3 ((GPIO_Type *)0x401C0000UL)
#define GPIO4 ((GPIO_Type *)0x401C4000UL)
#define GPIO5 ((GPIO_Type *)0x400C0000UL)

void     GPIO_PinInit(GPIO_Type *port, uint32_t pin, gpio_pin_config_t *cfg);
void     GPIO_PinWrite(GPIO_Type *port, uint32_t pin, uint8_t value);
uint32_t GPIO_PinRead(GPIO_Type *port, uint32_t pin);

#endif
