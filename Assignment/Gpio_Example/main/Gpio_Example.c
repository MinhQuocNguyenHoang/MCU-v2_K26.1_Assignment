#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define LED_GPIO 12

// ESP32-S3 GPIO register base
#define GPIO_BASE_ADDR        0x60004000UL

// GPIO registers
#define GPIO_OUT_REG          (*(volatile uint32_t *)(GPIO_BASE_ADDR + 0x0004))
#define GPIO_OUT_W1TS_REG     (*(volatile uint32_t *)(GPIO_BASE_ADDR + 0x0008))
#define GPIO_OUT_W1TC_REG     (*(volatile uint32_t *)(GPIO_BASE_ADDR + 0x000C))

#define GPIO_ENABLE_REG       (*(volatile uint32_t *)(GPIO_BASE_ADDR + 0x0020))
#define GPIO_ENABLE_W1TS_REG  (*(volatile uint32_t *)(GPIO_BASE_ADDR + 0x0024))
#define GPIO_ENABLE_W1TC_REG  (*(volatile uint32_t *)(GPIO_BASE_ADDR + 0x0028))

void app_main(void)
{
    // Enable GPIO12 as output
    GPIO_ENABLE_W1TS_REG = (1UL << LED_GPIO);

    while (1) {
        // Set GPIO12 = 1
        GPIO_OUT_W1TS_REG = (1UL << LED_GPIO);
        printf("LED ON\n");
        vTaskDelay(pdMS_TO_TICKS(1000));

        // Set GPIO12 = 0
        GPIO_OUT_W1TC_REG = (1UL << LED_GPIO);
        printf("LED OFF\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
