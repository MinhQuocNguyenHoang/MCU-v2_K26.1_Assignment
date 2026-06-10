#include <stdio.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "soc/soc.h"
#include "soc/periph_defs.h"
#include "esp_intr_alloc.h"

#define LED_GPIO     12
#define BUTTON_GPIO  11

// =========================
// ESP32-S3 GPIO register base
// =========================
#define GPIO_BASE_ADDR        0x60004000UL

#define GPIO_OUT_W1TS_REG     (*(volatile uint32_t *)(GPIO_BASE_ADDR + 0x0008))
#define GPIO_OUT_W1TC_REG     (*(volatile uint32_t *)(GPIO_BASE_ADDR + 0x000C))

#define GPIO_ENABLE_W1TS_REG  (*(volatile uint32_t *)(GPIO_BASE_ADDR + 0x0024))
#define GPIO_ENABLE_W1TC_REG  (*(volatile uint32_t *)(GPIO_BASE_ADDR + 0x0028))

#define GPIO_IN_REG           (*(volatile uint32_t *)(GPIO_BASE_ADDR + 0x003C))
#define GPIO_STATUS_REG       (*(volatile uint32_t *)(GPIO_BASE_ADDR + 0x0044))
#define GPIO_STATUS_W1TC_REG  (*(volatile uint32_t *)(GPIO_BASE_ADDR + 0x004C))

// GPIO_PINn_REG = GPIO_BASE + 0x0074 + 4*n
#define GPIO_PIN_REG(n)       (*(volatile uint32_t *)(GPIO_BASE_ADDR + 0x0074 + ((n) * 4)))

// =========================
// ESP32-S3 IO_MUX register base
// =========================
#define IO_MUX_BASE_ADDR      0x60009000UL

// Theo RM: IO_MUX_GPIOn_REG = IO_MUX_BASE + 0x0004 + 4*n
#define IO_MUX_GPIO_REG(n)    (*(volatile uint32_t *)(IO_MUX_BASE_ADDR + 0x0004 + ((n) * 4)))

// IO_MUX_GPIOn_REG bit fields
#define IO_MUX_MCU_WPD_BIT    7       // pull-down enable
#define IO_MUX_MCU_WPU_BIT    8       // pull-up enable
#define IO_MUX_MCU_IE_BIT     9       // input enable

#define IO_MUX_MCU_SEL_SHIFT  12
#define IO_MUX_MCU_SEL_MASK   (0x7UL << IO_MUX_MCU_SEL_SHIFT)

// Function GPIO
#define IO_MUX_FUNC_GPIO      1

// =========================
// GPIO_PINn_REG interrupt fields
// =========================
#define GPIO_PIN_INT_TYPE_SHIFT   7
#define GPIO_PIN_INT_TYPE_MASK    (0x7UL << GPIO_PIN_INT_TYPE_SHIFT)

#define GPIO_PIN_INT_ENA_CPU      (1UL << 13)

#define GPIO_INTR_DISABLE         0
#define GPIO_INTR_POSEDGE         1
#define GPIO_INTR_NEGEDGE         2
#define GPIO_INTR_ANYEDGE         3
#define GPIO_INTR_LOW_LEVEL       4
#define GPIO_INTR_HIGH_LEVEL      5

volatile int delay_ms = 1000;
volatile int interrupt_flag = 0;

static void IRAM_ATTR gpio_isr_handler(void *arg)
{
    uint32_t status = GPIO_STATUS_REG;

    if (status & (1UL << BUTTON_GPIO)) {
        // Clear interrupt flag của GPIO11
        GPIO_STATUS_W1TC_REG = (1UL << BUTTON_GPIO);

        delay_ms -= 100;
        if (delay_ms < 100) {
            delay_ms = 1000;
        }

        interrupt_flag = 1;
    }
}

void app_main(void)
{
    // =========================
    // GPIO12 output config
    // =========================

    // Chọn GPIO12 là GPIO function
    IO_MUX_GPIO_REG(LED_GPIO) &= ~IO_MUX_MCU_SEL_MASK;
    IO_MUX_GPIO_REG(LED_GPIO) |=  (IO_MUX_FUNC_GPIO << IO_MUX_MCU_SEL_SHIFT);

    // Enable GPIO12 output
    GPIO_ENABLE_W1TS_REG = (1UL << LED_GPIO);

    // =========================
    // GPIO11 input pull-up config
    // =========================

    // Chọn GPIO11 là GPIO function
    IO_MUX_GPIO_REG(BUTTON_GPIO) &= ~IO_MUX_MCU_SEL_MASK;
    IO_MUX_GPIO_REG(BUTTON_GPIO) |=  (IO_MUX_FUNC_GPIO << IO_MUX_MCU_SEL_SHIFT);

    // Bật input enable cho GPIO11
    IO_MUX_GPIO_REG(BUTTON_GPIO) |= (1UL << IO_MUX_MCU_IE_BIT);

    // Bật pull-up nội cho GPIO11
    IO_MUX_GPIO_REG(BUTTON_GPIO) |= (1UL << IO_MUX_MCU_WPU_BIT);

    // Tắt pull-down nội cho GPIO11
    IO_MUX_GPIO_REG(BUTTON_GPIO) &= ~(1UL << IO_MUX_MCU_WPD_BIT);

    // Disable output cho GPIO11
    GPIO_ENABLE_W1TC_REG = (1UL << BUTTON_GPIO);

    // =========================
    // GPIO11 interrupt config
    // =========================

    // Clear pending interrupt trước
    GPIO_STATUS_W1TC_REG = (1UL << BUTTON_GPIO);

    // Disable interrupt type trước khi cấu hình
    GPIO_PIN_REG(BUTTON_GPIO) &= ~GPIO_PIN_INT_TYPE_MASK;

    // Chọn interrupt cạnh xuống
    GPIO_PIN_REG(BUTTON_GPIO) |= (GPIO_INTR_NEGEDGE << GPIO_PIN_INT_TYPE_SHIFT);

    // Enable CPU interrupt cho GPIO11
    GPIO_PIN_REG(BUTTON_GPIO) |= GPIO_PIN_INT_ENA_CPU;

    // Gắn ISR cho nguồn ngắt GPIO
    esp_intr_alloc(
        ETS_GPIO_INTR_SOURCE,
        ESP_INTR_FLAG_IRAM,
        gpio_isr_handler,
        NULL,
        NULL
    );

    while (1) {
        if (interrupt_flag) {
            interrupt_flag = 0;
            printf("Interrupt occur, delay_ms = %d\n", delay_ms);
        }

        GPIO_OUT_W1TS_REG = (1UL << LED_GPIO);
        printf("LED ON\n");
        vTaskDelay(pdMS_TO_TICKS(delay_ms));

        GPIO_OUT_W1TC_REG = (1UL << LED_GPIO);
        printf("LED OFF\n");
        vTaskDelay(pdMS_TO_TICKS(delay_ms));
    }
}