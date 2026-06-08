
#include <stdio.h>
#include "led.h"
#include "motor.h"

int main(void)
{
	struct led red, green;
	struct motor fan;
	int ret;

	printf("========================================\n");
	printf("  Same pattern, two classes side by side.\n");
	printf("========================================\n\n");

	printf("--- led_init / motor_init: open for business ---\n");
	led_init(&red,   PIN_NUM('A', 13));   /* 0x0D = PA.13 */
	led_init(&green, PIN_NUM('A', 14));   /* 0x0E = PA.14 */
	motor_init(&fan, PIN_NUM('A', 5));    /* 0x05 = PA.5  */

	printf("\n--- LED operations ---\n");
	led_on(&red);
	led_set_brightness(&red, 80);
	led_on(&green);
	led_toggle(&green);

	printf("\n--- Motor operations ---\n");
	motor_set_speed(&fan, 60);
	motor_set_direction(&fan, true);
	motor_start(&fan);
	motor_stop(&fan);

	printf("\n--- Skip init: catch the mistake at API level ---\n");
	struct motor uninit = {0};
	ret = motor_start(&uninit);
	printf("  motor_start(uninit) returned %d (-3 = not initialized)\n",
	       ret);

	struct led uninit_led = {0};
	ret = led_on(&uninit_led);
	printf("  led_on(uninit) returned %d (-3 = not initialized)\n", ret);

	printf("\n--- Out-of-range arguments rejected ---\n");
	struct led bad_led;
	ret = led_init(&bad_led, 200);
	printf("  led_init(_, 200) returned %d (-2 = pin out of range)\n", ret);

	ret = motor_set_speed(&fan, 250);
	printf("  motor_set_speed(_, 250) returned %d (-2 = duty out of range)\n",
	       ret);

	printf("\n--- led_deinit / motor_deinit: closed ---\n");
	led_deinit(&red);
	led_deinit(&green);
	motor_deinit(&fan);

	printf("\n========================================\n");
	printf("  led_  prefix = LED class\n");
	printf("  motor_ prefix = Motor class\n");
	printf("  init / deinit = ctor / dtor (manual call in C)\n");
	printf("========================================\n");

	printf("\nPress Enter to exit...\n");
	getchar();
	return 0;
}
