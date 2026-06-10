#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "soc/soc.h"
#include "soc/periph_defs.h"
#include "esp_intr_alloc.h"

#define LED_GPIO 12
#define Button_GPIO 11

// ESP32-S3 GPIO register base
#define GPIO_BASE_ADDR        0x60004000UL

// GPIO registers
#define GPIO_OUT_REG          (*(volatile uint32_t *)(GPIO_BASE_ADDR + 0x0004))
#define GPIO_OUT_W1TS_REG     (*(volatile uint32_t *)(GPIO_BASE_ADDR + 0x0008))
#define GPIO_OUT_W1TC_REG     (*(volatile uint32_t *)(GPIO_BASE_ADDR + 0x000C))

#define GPIO_ENABLE_REG       (*(volatile uint32_t *)(GPIO_BASE_ADDR + 0x0020))
#define GPIO_ENABLE_W1TS_REG  (*(volatile uint32_t *)(GPIO_BASE_ADDR + 0x0024))
#define GPIO_ENABLE_W1TC_REG  (*(volatile uint32_t *)(GPIO_BASE_ADDR + 0x0028))

#define GPIO_IN_REG (*(volatile uint32_t *)(GPIO_BASE_ADDR + 0x003C))
#define GPIO_STATUS_REG (*(volatile uint32_t *)(GPIO_BASE_ADDR + 0x0044))
#define GPIO_STATUS_W1TC_REG (*(volatile uint32_t *)(GPIO_BASE_ADDR + 0x004C))
#define GPIO_PIN0_REG(n) (*(volatile uint32_t *)(GPIO_BASE_ADDR + 0x0074 + ((n) * 4)))

// Interrupt register
#define INTERRUPT_CORE0_GPIO_INTERRUPT_CPU_MAP_REG (*(volatile uint32_t *)(GPIO_BASE_ADDR + 0x0040))

#define GPIO_PIN_INT_TYPE_SHIFT  7
#define GPIO_PIN_INT_TYPE_MASK   (0x7 << GPIO_PIN_INT_TYPE_SHIFT)

#define GPIO_INTR_DISABLE        0
#define GPIO_INTR_POSEDGE        1
#define GPIO_INTR_NEGEDGE        2
#define GPIO_INTR_ANYEDGE        3
#define GPIO_INTR_LOW_LEVEL      4
#define GPIO_INTR_HIGH_LEVEL     5

int delay_ms = 1000;

static void IRAM_ATTR gpio_isr_handler(void* arg){
    uint32_t status = GPIO_STATUS_REG;
    if(status & (1UL << Button_GPIO)){
        GPIO_STATUS_W1TC_REG = (1UL << Button_GPIO);
        delay_ms -= 100;
        if(delay_ms < 100){
            delay_ms = 1000;
        }
    }
}

void app_main(void)
{
    // Enable GPIO12 as output
    GPIO_ENABLE_REG |= (1UL << LED_GPIO);
    GPIO_ENABLE_REG &= ~(1UL << Button_GPIO);
    GPIO_STATUS_W1TC_REG |= (1UL << Button_GPIO);

    // ngắt cạnh xuống
    GPIO_PIN0_REG(Button_GPIO) &= ~GPIO_PIN_INT_TYPE_MASK;
    GPIO_PIN0_REG(Button_GPIO) |= (GPIO_INTR_NEGEDGE << GPIO_PIN_INT_TYPE_SHIFT);

    // Gắn ISR cho nguồn ngắt GPIO
    esp_intr_alloc(
        ETS_GPIO_INTR_SOURCE,
        ESP_INTR_FLAG_IRAM,
        gpio_isr_handler,
        NULL,
        NULL
    );

    while (1) {
        
        // Set GPIO12 = 1
        GPIO_OUT_W1TS_REG = (1UL << LED_GPIO);
        printf("LED ON\n");
        vTaskDelay(pdMS_TO_TICKS(delay_ms));

        // Set GPIO12 = 0
        GPIO_OUT_W1TC_REG = (1UL << LED_GPIO);
        printf("LED OFF\n");
        vTaskDelay(pdMS_TO_TICKS(delay_ms));
    }
}
