// Bài tập 1.2 & 1.3 - Điều khiển LED qua UART
// Board: ESP32-S3 DevKitC v1
// Dùng PuTTY hoặc Hercules để gửi lệnh từ PC
//
// Đấu nối:
//   GPIO12 -> LED thường (330 ohm -> GND)
//   GPIO13 -> RGB đỏ     (330 ohm -> GND)
//   GPIO14 -> RGB xanh lá (330 ohm -> GND)
//   GPIO15 -> RGB xanh dương (330 ohm -> GND)
//   UART0 (GPIO43=TX, GPIO44=RX) nối sẵn với USB trên board rồi

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "driver/uart.h"
#include "esp_log.h"
#include "soc/uart_reg.h"

// --- Pin LED ---
#define LED_GPIO        12
#define RGB_RED_GPIO    13
#define RGB_GREEN_GPIO  14
#define RGB_BLUE_GPIO   15

// --- Địa chỉ thanh ghi GPIO (tra trong TRM ESP32-S3) ---
#define GPIO_BASE_ADDR       0x60004000UL

// W1TS = write 1 to set, W1TC = write 1 to clear
#define GPIO_OUT_W1TS_REG    (*(volatile uint32_t *)(GPIO_BASE_ADDR + 0x0008))
#define GPIO_OUT_W1TC_REG    (*(volatile uint32_t *)(GPIO_BASE_ADDR + 0x000C))
#define GPIO_ENABLE_W1TS_REG (*(volatile uint32_t *)(GPIO_BASE_ADDR + 0x0024))
#define GPIO_ENABLE_W1TC_REG (*(volatile uint32_t *)(GPIO_BASE_ADDR + 0x0028))

// IO_MUX dùng để chọn function cho từng chân
#define IO_MUX_BASE_ADDR     0x60009000UL
#define IO_MUX_GPIO_REG(n)   (*(volatile uint32_t *)(IO_MUX_BASE_ADDR + 0x0004 + ((n) * 4)))

#define IO_MUX_MCU_SEL_SHIFT 12
#define IO_MUX_MCU_SEL_MASK  (0x7UL << IO_MUX_MCU_SEL_SHIFT)
#define IO_MUX_FUNC_GPIO     1  // function 1 là GPIO

// --- Cấu hình UART ---
#define UART_PORT_NUM   UART_NUM_0  // UART0 nối với USB
#define UART_BAUD_RATE  115200
#define UART_BUF_SIZE   256
#define CMD_MAX_LEN     32

static const char *TAG = "UART_LED";

// buffer để gom từng ký tự nhận được thành 1 chuỗi lệnh
static char rx_buf[CMD_MAX_LEN + 1];
static int  rx_idx = 0;

// --- Hàm cấu hình GPIO output (dùng thanh ghi thẳng) ---
static void gpio_set_output(int gpio_num)
{
    // chọn function GPIO cho chân này (bit MCU_SEL trong IO_MUX)
    IO_MUX_GPIO_REG(gpio_num) &= ~IO_MUX_MCU_SEL_MASK;
    IO_MUX_GPIO_REG(gpio_num) |=  (IO_MUX_FUNC_GPIO << IO_MUX_MCU_SEL_SHIFT);

    // bật output enable
    GPIO_ENABLE_W1TS_REG = (1UL << gpio_num);
}

// bật LED (set pin lên HIGH)
static inline void gpio_high(int gpio_num)
{
    GPIO_OUT_W1TS_REG = (1UL << gpio_num);
}

// tắt LED (kéo pin xuống LOW)
static inline void gpio_low(int gpio_num)
{
    GPIO_OUT_W1TC_REG = (1UL << gpio_num);
}

// --- Bỏ khoảng trắng và \r\n ở đầu/cuối chuỗi ---
static void trim_cmd(char *s)
{
    // xóa cuối
    int len = (int)strlen(s);
    while (len > 0 && (s[len-1] == '\r' || s[len-1] == '\n' ||
                        s[len-1] == ' '  || s[len-1] == '\t')) {
        s[--len] = '\0';
    }
    // xóa đầu
    int start = 0;
    while (s[start] == ' ' || s[start] == '\t') start++;
    if (start > 0) memmove(s, s + start, len - start + 1);
}

// --- So sánh lệnh nhận được và điều khiển LED tương ứng ---
static void execute_command(const char *cmd)
{
    if (strcmp(cmd, "LED_ON") == 0) {
        gpio_high(LED_GPIO);
        ESP_LOGI(TAG, "LED ON");
        uart_write_bytes(UART_PORT_NUM, "OK: LED ON\r\n", 12);

    } else if (strcmp(cmd, "LED_OFF") == 0) {
        gpio_low(LED_GPIO);
        ESP_LOGI(TAG, "LED OFF");
        uart_write_bytes(UART_PORT_NUM, "OK: LED OFF\r\n", 13);

    } else if (strcmp(cmd, "RED_ON") == 0) {
        gpio_high(RGB_RED_GPIO);
        ESP_LOGI(TAG, "RED ON");
        uart_write_bytes(UART_PORT_NUM, "OK: RED ON\r\n", 12);

    } else if (strcmp(cmd, "RED_OFF") == 0) {
        gpio_low(RGB_RED_GPIO);
        ESP_LOGI(TAG, "RED OFF");
        uart_write_bytes(UART_PORT_NUM, "OK: RED OFF\r\n", 13);

    } else if (strcmp(cmd, "GREEN_ON") == 0) {
        gpio_high(RGB_GREEN_GPIO);
        ESP_LOGI(TAG, "GREEN ON");
        uart_write_bytes(UART_PORT_NUM, "OK: GREEN ON\r\n", 14);

    } else if (strcmp(cmd, "GREEN_OFF") == 0) {
        gpio_low(RGB_GREEN_GPIO);
        ESP_LOGI(TAG, "GREEN OFF");
        uart_write_bytes(UART_PORT_NUM, "OK: GREEN OFF\r\n", 15);

    } else if (strcmp(cmd, "BLUE_ON") == 0) {
        gpio_high(RGB_BLUE_GPIO);
        ESP_LOGI(TAG, "BLUE ON");
        uart_write_bytes(UART_PORT_NUM, "OK: BLUE ON\r\n", 13);

    } else if (strcmp(cmd, "BLUE_OFF") == 0) {
        gpio_low(RGB_BLUE_GPIO);
        ESP_LOGI(TAG, "BLUE OFF");
        uart_write_bytes(UART_PORT_NUM, "OK: BLUE OFF\r\n", 14);

    } else {
        // lệnh lạ, không hiểu
        char err_msg[64];
        int n = snprintf(err_msg, sizeof(err_msg), "ERR: khong hieu lenh \"%s\"\r\n", cmd);
        uart_write_bytes(UART_PORT_NUM, err_msg, n);
        ESP_LOGW(TAG, "lenh khong hop le: %s", cmd);
    }
}

// --- Task liên tục đọc UART và xử lý lệnh ---
static void uart_rx_task(void *arg)
{
    uint8_t byte;
    int len;

    // in thông báo khi board mới khởi động
    const char *banner =
        "\r\n=== UART LED Control (ESP32-S3) ===\r\n"
        "Lenh: LED_ON/OFF  RED_ON/OFF  GREEN_ON/OFF  BLUE_ON/OFF\r\n"
        ">> ";
    uart_write_bytes(UART_PORT_NUM, banner, strlen(banner));

    while (1) {
        // đọc 1 byte từ UART, chờ tối đa 20ms
        len = uart_read_bytes(UART_PORT_NUM, &byte, 1, pdMS_TO_TICKS(20));

        if (len <= 0) continue;  // chưa có gì thì bỏ qua

        // echo lại để thấy mình đang gõ gì
        uart_write_bytes(UART_PORT_NUM, (const char *)&byte, 1);

        if (byte == '\n' || byte == '\r') {
            // nhấn Enter -> xử lý lệnh
            if (rx_idx > 0) {
                rx_buf[rx_idx] = '\0';
                trim_cmd(rx_buf);

                if (strlen(rx_buf) > 0) {
                    execute_command(rx_buf);
                }

                // reset buffer cho lần nhập tiếp theo
                rx_idx = 0;
                memset(rx_buf, 0, sizeof(rx_buf));
                uart_write_bytes(UART_PORT_NUM, "\r\n>> ", 5);
            }

        } else if (byte == 0x08 || byte == 0x7F) {
            // backspace -> xóa ký tự cuối
            if (rx_idx > 0) {
                rx_idx--;
                rx_buf[rx_idx] = '\0';
                uart_write_bytes(UART_PORT_NUM, " \b", 2);
            }

        } else {
            // ký tự bình thường thì lưu vào buffer
            if (rx_idx < CMD_MAX_LEN) {
                rx_buf[rx_idx++] = (char)byte;
            }
            // nếu buffer đầy thì thôi, bỏ qua
        }
    }
}

void app_main(void)
{
    // cấu hình 4 chân LED làm output
    gpio_set_output(LED_GPIO);
    gpio_set_output(RGB_RED_GPIO);
    gpio_set_output(RGB_GREEN_GPIO);
    gpio_set_output(RGB_BLUE_GPIO);

    // tắt hết LED lúc mới bật nguồn
    gpio_low(LED_GPIO);
    gpio_low(RGB_RED_GPIO);
    gpio_low(RGB_GREEN_GPIO);
    gpio_low(RGB_BLUE_GPIO);

    // cấu hình UART0: 115200 baud, 8N1, không flow control
    const uart_config_t uart_config = {
        .baud_rate           = UART_BAUD_RATE,
        .data_bits           = UART_DATA_8_BITS,
        .parity              = UART_PARITY_DISABLE,
        .stop_bits           = UART_STOP_BITS_1,
        .flow_ctrl           = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 0,
        .source_clk          = UART_SCLK_DEFAULT,
    };

    ESP_ERROR_CHECK(uart_param_config(UART_PORT_NUM, &uart_config));

    // cài driver để UART dùng interrupt + ring buffer nội bộ
    ESP_ERROR_CHECK(uart_driver_install(
        UART_PORT_NUM,
        UART_BUF_SIZE * 2,  // rx buffer
        UART_BUF_SIZE * 2,  // tx buffer
        0, NULL, 0
    ));

    // tạo task để xử lý dữ liệu UART
    xTaskCreate(uart_rx_task, "uart_rx_task", 4096, NULL, 5, NULL);

    ESP_LOGI(TAG, "khoi dong xong, cho lenh tu PC...");
}
