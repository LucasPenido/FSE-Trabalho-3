#ifndef LED_H
#define LED_H

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

#define BLINK_GPIO 2

void led_Taskpisca();
void led_piscaUmaVez();
void led_liga();
void led_desliga();

#endif
