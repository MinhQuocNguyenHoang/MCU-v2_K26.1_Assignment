#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

const uint32_t *GPIO_BT_SELECT_REG = (const uint32_t *)0x0000;
const uint32_t *GPIO_OUT_REG = (const uint32_t *)0x0004;

void app_main(void)
{
    *GPIO_BT_SELECT_REG &= ~(1<<12);
    *GPIO_BT_SELECT_REG |= (1<<12);
    while(1){
        *GPIO_OUT_REG |= (1<<12);
        printf("LED ON");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        *GPIO_OUT_REG |= (0<<12);
        printf("LED OFF");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
