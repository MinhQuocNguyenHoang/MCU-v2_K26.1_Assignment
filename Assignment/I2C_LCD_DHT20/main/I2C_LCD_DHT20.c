/*
 * Assignment: Read temperature/humidity from DHT20 (I2C) and display on SSD1306 OLED (I2C)
 * Board: ESP32-S3 DevKitC v1
 * Implementation: Register-level peripheral configuration (no high-level APIs)
 *
 * Pinout Connections:
 *   ESP32-S3 GPIO1 -> I2C SCL (both DHT20 and SSD1306)
 *   ESP32-S3 GPIO2 -> I2C SDA (both DHT20 and SSD1306)
 *   External Button (GPIO4) -> Connected to GND, used for manual interrupt trigger
 *
 * Pull-up resistors:
 *   Internal pull-ups are enabled in software. However, for reliable I2C,
 *   external 4.7k ohm pull-up resistors to 3.3V are highly recommended.
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_rom_sys.h"      // For esp_rom_delay_us
#include "soc/soc.h"
#include "soc/periph_defs.h"  // For ETS_GPIO_INTR_SOURCE
#include "esp_intr_alloc.h"   // For esp_intr_alloc

static const char *TAG = "I2C_DHT20_SSD1306";

// ==========================================
// Peripheral Register Base Addresses (ESP32-S3)
// ==========================================
#define GPIO_BASE_ADDR        0x60004000UL
#define IO_MUX_BASE_ADDR      0x60009000UL

// ==========================================
// Register Offset Definitions
// ==========================================
#define GPIO_OUT_W1TS_REG     (*(volatile uint32_t *)(GPIO_BASE_ADDR + 0x0008))
#define GPIO_OUT_W1TC_REG     (*(volatile uint32_t *)(GPIO_BASE_ADDR + 0x000C))
#define GPIO_ENABLE_W1TS_REG  (*(volatile uint32_t *)(GPIO_BASE_ADDR + 0x0024))
#define GPIO_ENABLE_W1TC_REG  (*(volatile uint32_t *)(GPIO_BASE_ADDR + 0x0028))
#define GPIO_IN_REG           (*(volatile uint32_t *)(GPIO_BASE_ADDR + 0x003C))
#define GPIO_STATUS_REG       (*(volatile uint32_t *)(GPIO_BASE_ADDR + 0x0044))
#define GPIO_STATUS_W1TC_REG  (*(volatile uint32_t *)(GPIO_BASE_ADDR + 0x004C))
#define GPIO_PIN_REG(n)       (*(volatile uint32_t *)(GPIO_BASE_ADDR + 0x0074 + ((n) * 4)))

#define IO_MUX_GPIO_REG(n)    (*(volatile uint32_t *)(IO_MUX_BASE_ADDR + 0x0004 + ((n) * 4)))

// ==========================================
// Bitfield & Mask Definitions
// ==========================================
#define IO_MUX_MCU_WPD_BIT    7       // Pull-down enable
#define IO_MUX_MCU_WPU_BIT    8       // Pull-up enable
#define IO_MUX_MCU_IE_BIT     9       // Input enable
#define IO_MUX_MCU_SEL_SHIFT  12
#define IO_MUX_MCU_SEL_MASK   (0x7UL << IO_MUX_MCU_SEL_SHIFT)
#define IO_MUX_FUNC_GPIO      1       // Mux Function 1 is GPIO

#define GPIO_PIN_INT_TYPE_SHIFT   7
#define GPIO_PIN_INT_TYPE_MASK    (0x7UL << GPIO_PIN_INT_TYPE_SHIFT)
#define GPIO_PIN_INT_ENA_CPU      (1UL << 13)

#define GPIO_INTR_NEGEDGE         2   // Interrupt on falling edge

// ==========================================
// Configuration Pin Definitions
// ==========================================
#define I2C_SCL_PIN   1
#define I2C_SDA_PIN   2
#define BUTTON_GPIO   4  // External button connected to GPIO4 and GND

// ==========================================
// Font Table for SSD1306 (ASCII 0x20 to 0x7E)
// Modified: '^' (0x5E) is replaced with degree symbol (o)
// ==========================================
static const uint8_t font5x8[][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, // space (0x20)
    {0x00, 0x00, 0x5f, 0x00, 0x00}, // !
    {0x00, 0x07, 0x00, 0x07, 0x00}, // "
    {0x14, 0x7f, 0x14, 0x7f, 0x14}, // #
    {0x24, 0x2a, 0x7f, 0x2a, 0x12}, // $
    {0x23, 0x13, 0x08, 0x64, 0x62}, // %
    {0x36, 0x49, 0x55, 0x22, 0x50}, // &
    {0x00, 0x05, 0x03, 0x00, 0x00}, // '
    {0x00, 0x1c, 0x22, 0x41, 0x00}, // (
    {0x00, 0x41, 0x22, 0x1c, 0x00}, // )
    {0x14, 0x08, 0x3e, 0x08, 0x14}, // *
    {0x08, 0x08, 0x3e, 0x08, 0x08}, // +
    {0x00, 0x50, 0x30, 0x00, 0x00}, // ,
    {0x08, 0x08, 0x08, 0x08, 0x08}, // -
    {0x00, 0x60, 0x60, 0x00, 0x00}, // .
    {0x20, 0x10, 0x08, 0x04, 0x02}, // /
    {0x3e, 0x51, 0x49, 0x45, 0x3e}, // 0 (0x30)
    {0x00, 0x42, 0x7f, 0x40, 0x00}, // 1
    {0x42, 0x61, 0x51, 0x49, 0x46}, // 2
    {0x21, 0x41, 0x45, 0x4b, 0x31}, // 3
    {0x18, 0x14, 0x12, 0x7f, 0x10}, // 4
    {0x27, 0x45, 0x45, 0x45, 0x39}, // 5
    {0x3c, 0x4a, 0x49, 0x49, 0x30}, // 6
    {0x01, 0x71, 0x09, 0x05, 0x03}, // 7
    {0x36, 0x49, 0x49, 0x49, 0x36}, // 8
    {0x06, 0x49, 0x49, 0x29, 0x1e}, // 9
    {0x00, 0x36, 0x36, 0x00, 0x00}, // :
    {0x00, 0x56, 0x36, 0x00, 0x00}, // ;
    {0x08, 0x14, 0x22, 0x41, 0x00}, // <
    {0x14, 0x14, 0x14, 0x14, 0x14}, // =
    {0x00, 0x41, 0x22, 0x14, 0x08}, // >
    {0x02, 0x01, 0x51, 0x09, 0x06}, // ?
    {0x32, 0x49, 0x79, 0x41, 0x3e}, // @
    {0x7e, 0x11, 0x11, 0x11, 0x7e}, // A
    {0x7f, 0x49, 0x49, 0x49, 0x36}, // B
    {0x3e, 0x41, 0x41, 0x41, 0x22}, // C
    {0x7f, 0x41, 0x41, 0x22, 0x1c}, // D
    {0x7f, 0x49, 0x49, 0x49, 0x41}, // E
    {0x7f, 0x09, 0x09, 0x09, 0x01}, // F
    {0x3e, 0x41, 0x49, 0x49, 0x7a}, // G
    {0x7f, 0x08, 0x08, 0x08, 0x7f}, // H
    {0x00, 0x41, 0x7f, 0x41, 0x00}, // I
    {0x20, 0x40, 0x41, 0x3f, 0x01}, // J
    {0x7f, 0x08, 0x14, 0x22, 0x41}, // K
    {0x7f, 0x40, 0x40, 0x40, 0x40}, // L
    {0x7f, 0x02, 0x0c, 0x02, 0x7f}, // M
    {0x7f, 0x04, 0x08, 0x10, 0x7f}, // N
    {0x3e, 0x41, 0x41, 0x41, 0x3e}, // O
    {0x7f, 0x09, 0x09, 0x09, 0x06}, // P
    {0x3e, 0x41, 0x51, 0x21, 0x5e}, // Q
    {0x7f, 0x09, 0x19, 0x29, 0x46}, // R
    {0x46, 0x49, 0x49, 0x49, 0x31}, // S
    {0x01, 0x01, 0x7f, 0x01, 0x01}, // T
    {0x3f, 0x40, 0x40, 0x40, 0x3f}, // U
    {0x1f, 0x20, 0x40, 0x20, 0x1f}, // V
    {0x3f, 0x40, 0x38, 0x40, 0x3f}, // W
    {0x63, 0x14, 0x08, 0x14, 0x63}, // X
    {0x07, 0x08, 0x70, 0x08, 0x07}, // Y
    {0x61, 0x51, 0x49, 0x45, 0x43}, // Z
    {0x00, 0x7f, 0x41, 0x41, 0x00}, // [
    {0x02, 0x04, 0x08, 0x10, 0x20}, // backslash
    {0x00, 0x41, 0x41, 0x7f, 0x00}, // ]
    {0x06, 0x09, 0x09, 0x06, 0x00}, // ^ (Replaced with degree symbol)
    {0x40, 0x40, 0x40, 0x40, 0x40}, // _
    {0x00, 0x01, 0x02, 0x04, 0x00}, // `
    {0x20, 0x54, 0x54, 0x54, 0x78}, // a
    {0x7f, 0x48, 0x44, 0x44, 0x38}, // b
    {0x38, 0x44, 0x44, 0x44, 0x20}, // c
    {0x38, 0x44, 0x44, 0x48, 0x7f}, // d
    {0x38, 0x54, 0x54, 0x54, 0x18}, // e
    {0x08, 0x7e, 0x09, 0x01, 0x02}, // f
    {0x0c, 0x52, 0x52, 0x52, 0x3e}, // g
    {0x7f, 0x08, 0x04, 0x04, 0x78}, // h
    {0x00, 0x44, 0x7d, 0x40, 0x00}, // i
    {0x20, 0x40, 0x44, 0x3d, 0x00}, // j
    {0x7f, 0x10, 0x28, 0x44, 0x00}, // k
    {0x00, 0x41, 0x7f, 0x40, 0x00}, // l
    {0x7c, 0x04, 0x18, 0x04, 0x78}, // m
    {0x7c, 0x08, 0x04, 0x04, 0x78}, // n
    {0x38, 0x44, 0x44, 0x44, 0x38}, // o
    {0x7c, 0x14, 0x14, 0x14, 0x08}, // p
    {0x08, 0x14, 0x14, 0x18, 0x7c}, // q
    {0x7c, 0x08, 0x04, 0x04, 0x08}, // r
    {0x48, 0x54, 0x54, 0x54, 0x20}, // s
    {0x04, 0x3f, 0x44, 0x40, 0x20}, // t
    {0x3c, 0x40, 0x40, 0x20, 0x7c}, // u
    {0x1c, 0x20, 0x40, 0x20, 0x1c}, // v
    {0x3c, 0x40, 0x30, 0x40, 0x3c}, // w
    {0x44, 0x28, 0x10, 0x28, 0x44}, // x
    {0x0c, 0x50, 0x50, 0x50, 0x3c}, // y
    {0x44, 0x64, 0x54, 0x4c, 0x44}, // z
    {0x00, 0x08, 0x36, 0x41, 0x00}, // {
    {0x00, 0x00, 0x7f, 0x00, 0x00}, // |
    {0x00, 0x41, 0x36, 0x08, 0x00}, // }
    {0x02, 0x01, 0x02, 0x04, 0x02}, // ~
};

// ==========================================
// Low-Level I2C Software Driver
// ==========================================

static inline void sda_high(void) {
    GPIO_OUT_W1TS_REG = (1UL << I2C_SDA_PIN);
}

static inline void sda_low(void) {
    GPIO_OUT_W1TC_REG = (1UL << I2C_SDA_PIN);
}

static inline void scl_high(void) {
    GPIO_OUT_W1TS_REG = (1UL << I2C_SCL_PIN);
}

static inline void scl_low(void) {
    GPIO_OUT_W1TC_REG = (1UL << I2C_SCL_PIN);
}

static inline int read_sda(void) {
    return (GPIO_IN_REG & (1UL << I2C_SDA_PIN)) ? 1 : 0;
}

// Clock stretching support: wait for SCL pin to actually float high
static inline void scl_high_stretch(void) {
    scl_high();
    int timeout = 1000;
    while (!(GPIO_IN_REG & (1UL << I2C_SCL_PIN)) && timeout > 0) {
        esp_rom_delay_us(1);
        timeout--;
    }
}

static void i2c_delay(void) {
    esp_rom_delay_us(5); // 5us delay gives approx 100kHz I2C speed
}

void i2c_init(void) {
    // Configure SCL as open-drain, pull-up, input enabled
    IO_MUX_GPIO_REG(I2C_SCL_PIN) &= ~IO_MUX_MCU_SEL_MASK;
    IO_MUX_GPIO_REG(I2C_SCL_PIN) |= (IO_MUX_FUNC_GPIO << IO_MUX_MCU_SEL_SHIFT);
    IO_MUX_GPIO_REG(I2C_SCL_PIN) |= (1UL << IO_MUX_MCU_IE_BIT);
    IO_MUX_GPIO_REG(I2C_SCL_PIN) |= (1UL << IO_MUX_MCU_WPU_BIT);
    IO_MUX_GPIO_REG(I2C_SCL_PIN) &= ~(1UL << IO_MUX_MCU_WPD_BIT);
    GPIO_PIN_REG(I2C_SCL_PIN) |= (1UL << 2); // Set PAD_DRIVER bit for open-drain
    GPIO_ENABLE_W1TS_REG = (1UL << I2C_SCL_PIN);

    // Configure SDA as open-drain, pull-up, input enabled
    IO_MUX_GPIO_REG(I2C_SDA_PIN) &= ~IO_MUX_MCU_SEL_MASK;
    IO_MUX_GPIO_REG(I2C_SDA_PIN) |= (IO_MUX_FUNC_GPIO << IO_MUX_MCU_SEL_SHIFT);
    IO_MUX_GPIO_REG(I2C_SDA_PIN) |= (1UL << IO_MUX_MCU_IE_BIT);
    IO_MUX_GPIO_REG(I2C_SDA_PIN) |= (1UL << IO_MUX_MCU_WPU_BIT);
    IO_MUX_GPIO_REG(I2C_SDA_PIN) &= ~(1UL << IO_MUX_MCU_WPD_BIT);
    GPIO_PIN_REG(I2C_SDA_PIN) |= (1UL << 2); // Set PAD_DRIVER bit for open-drain
    GPIO_ENABLE_W1TS_REG = (1UL << I2C_SDA_PIN);

    // Release bus lines
    sda_high();
    scl_high();
    i2c_delay();
    ESP_LOGI(TAG, "I2C initialized at register level (Software Open-Drain, SCL: GPIO%d, SDA: GPIO%d)", I2C_SCL_PIN, I2C_SDA_PIN);
}

void i2c_start(void) {
    sda_high();
    scl_high_stretch();
    i2c_delay();
    sda_low();
    i2c_delay();
    scl_low();
    i2c_delay();
}

void i2c_stop(void) {
    sda_low();
    i2c_delay();
    scl_high_stretch();
    i2c_delay();
    sda_high();
    i2c_delay();
}

// Write byte. Returns 0 for ACK, 1 for NACK
int i2c_write_byte(uint8_t byte) {
    for (int i = 0; i < 8; i++) {
        if (byte & 0x80) {
            sda_high();
        } else {
            sda_low();
        }
        byte <<= 1;
        i2c_delay();
        scl_high_stretch();
        i2c_delay();
        scl_low();
        i2c_delay();
    }

    // Read ACK/NACK bit
    sda_high(); // Release SDA so slave can control it
    i2c_delay();
    scl_high_stretch();
    i2c_delay();
    int ack = read_sda();
    scl_low();
    i2c_delay();
    return ack;
}

// Read byte. Set ack = 1 to send ACK, ack = 0 to send NACK
uint8_t i2c_read_byte(int ack) {
    uint8_t byte = 0;
    sda_high(); // Release SDA
    for (int i = 0; i < 8; i++) {
        i2c_delay();
        scl_high_stretch();
        i2c_delay();
        byte <<= 1;
        if (read_sda()) {
            byte |= 1;
        }
        scl_low();
        i2c_delay();
    }

    // Write ACK/NACK bit
    if (ack) {
        sda_low();
    } else {
        sda_high();
    }
    i2c_delay();
    scl_high_stretch();
    i2c_delay();
    scl_low();
    i2c_delay();
    sda_high(); // Release SDA
    return byte;
}

int i2c_write_device(uint8_t dev_addr, const uint8_t *data, size_t len) {
    i2c_start();
    if (i2c_write_byte(dev_addr << 1) != 0) {
        i2c_stop();
        return -1;
    }
    for (size_t i = 0; i < len; i++) {
        if (i2c_write_byte(data[i]) != 0) {
            i2c_stop();
            return -1;
        }
    }
    i2c_stop();
    return 0;
}

int i2c_read_device(uint8_t dev_addr, uint8_t *data, size_t len) {
    i2c_start();
    if (i2c_write_byte((dev_addr << 1) | 1) != 0) {
        i2c_stop();
        return -1;
    }
    for (size_t i = 0; i < len; i++) {
        data[i] = i2c_read_byte(i < len - 1); // ACK for all but last
    }
    i2c_stop();
    return 0;
}

// ==========================================
// DHT20 Sensor Driver
// ==========================================
#define DHT20_ADDR 0x38

int dht20_init(void) {
    uint8_t status = 0;
    vTaskDelay(pdMS_TO_TICKS(100)); // Delay for DHT20 bootup

    if (i2c_read_device(DHT20_ADDR, &status, 1) != 0) {
        ESP_LOGE(TAG, "DHT20 read status failed");
        return -1;
    }

    // If calibration bit (bit 3) is 0, initialize device
    if ((status & 0x08) == 0) {
        uint8_t cal_cmd[] = {0xBE, 0x08, 0x00};
        if (i2c_write_device(DHT20_ADDR, cal_cmd, 3) != 0) {
            ESP_LOGE(TAG, "DHT20 calibration cmd failed");
            return -1;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    ESP_LOGI(TAG, "DHT20 sensor initialized successfully");
    return 0;
}

int dht20_read(float *temp, float *hum) {
    uint8_t trigger_cmd[] = {0xAC, 0x33, 0x00};
    if (i2c_write_device(DHT20_ADDR, trigger_cmd, 3) != 0) {
        return -1;
    }

    vTaskDelay(pdMS_TO_TICKS(80)); // Wait 80ms for conversion to complete

    uint8_t data[6];
    int retry = 10;
    while (retry > 0) {
        if (i2c_read_device(DHT20_ADDR, data, 6) != 0) {
            return -1;
        }
        // Check if bit 7 (Busy) of status byte is 0
        if ((data[0] & 0x80) == 0) {
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
        retry--;
    }

    if (retry == 0) {
        ESP_LOGW(TAG, "DHT20 sensor busy timeout");
        return -2;
    }

    // Convert raw reading to temperature & humidity
    uint32_t raw_hum = ((uint32_t)data[1] << 12) | ((uint32_t)data[2] << 4) | ((data[3] & 0xF0) >> 4);
    uint32_t raw_temp = (((uint32_t)data[3] & 0x0F) << 16) | ((uint32_t)data[4] << 8) | data[5];

    *hum = ((float)raw_hum / 1048576.0f) * 100.0f;
    *temp = (((float)raw_temp / 1048576.0f) * 200.0f) - 50.0f;

    return 0;
}

// ==========================================
// SSD1306 OLED Display Driver
// ==========================================
#define SSD1306_ADDR 0x3C

void ssd1306_write_cmd(uint8_t cmd) {
    uint8_t buf[2] = {0x00, cmd};
    i2c_write_device(SSD1306_ADDR, buf, 2);
}

void ssd1306_write_data(const uint8_t *data, size_t len) {
    i2c_start();
    i2c_write_byte(SSD1306_ADDR << 1);
    i2c_write_byte(0x40); // Control byte: Co = 0, D/C# = 1 (Data stream)
    for (size_t i = 0; i < len; i++) {
        i2c_write_byte(data[i]);
    }
    i2c_stop();
}

void ssd1306_set_cursor(uint8_t page, uint8_t col) {
    ssd1306_write_cmd(0xB0 + (page & 0x07)); // Set Page Address
    ssd1306_write_cmd(0x00 + (col & 0x0F));  // Set Column Lower Nibble
    ssd1306_write_cmd(0x10 + ((col >> 4) & 0x0F)); // Set Column Upper Nibble
}

void ssd1306_clear(void) {
    uint8_t zero_buf[128];
    memset(zero_buf, 0, sizeof(zero_buf));
    for (uint8_t page = 0; page < 8; page++) {
        ssd1306_set_cursor(page, 0);
        ssd1306_write_data(zero_buf, 128);
    }
}

void ssd1306_init(void) {
    vTaskDelay(pdMS_TO_TICKS(100)); // Delay for display start

    ssd1306_write_cmd(0xAE); // Display off
    ssd1306_write_cmd(0xD5); ssd1306_write_cmd(0x80); // Set clock divide ratio
    ssd1306_write_cmd(0xA8); ssd1306_write_cmd(0x3F); // Mux ratio 1/64
    ssd1306_write_cmd(0xD3); ssd1306_write_cmd(0x00); // Display offset = 0
    ssd1306_write_cmd(0x40); // Set start line = 0
    ssd1306_write_cmd(0x8D); ssd1306_write_cmd(0x14); // Enable charge pump (required)
    ssd1306_write_cmd(0x20); ssd1306_write_cmd(0x02); // Memory mode: Page Addressing Mode
    ssd1306_write_cmd(0xA1); // Segment re-map (Horizontal Flip)
    ssd1306_write_cmd(0xC8); // COM scan direction (Vertical Flip)
    ssd1306_write_cmd(0xDA); ssd1306_write_cmd(0x12); // COM hardware pins config
    ssd1306_write_cmd(0x81); ssd1306_write_cmd(0xCF); // Contrast control
    ssd1306_write_cmd(0xD9); ssd1306_write_cmd(0xF1); // Pre-charge period
    ssd1306_write_cmd(0xDB); ssd1306_write_cmd(0x40); // VCOMH deselect level
    ssd1306_write_cmd(0xA4); // Output follows RAM contents
    ssd1306_write_cmd(0xA6); // Normal display
    ssd1306_write_cmd(0xAF); // Display on

    ssd1306_clear();
    ESP_LOGI(TAG, "SSD1306 OLED initialized successfully");
}

void ssd1306_draw_char(uint8_t page, uint8_t col, char c) {
    if (c < 0x20 || c > 0x7E) {
        c = ' ';
    }
    uint8_t idx = c - 0x20;
    ssd1306_set_cursor(page, col);
    ssd1306_write_data(font5x8[idx], 5);
    
    // Draw 1-column spacing between characters
    uint8_t spacer = 0x00;
    ssd1306_write_data(&spacer, 1);
}

void ssd1306_draw_string(uint8_t page, uint8_t col, const char *str) {
    while (*str) {
        ssd1306_draw_char(page, col, *str);
        col += 6;
        if (col >= 128 - 6) {
            col = 0;
            page++;
            if (page >= 8) break;
        }
        str++;
    }
}

// ==========================================
// External Button Interrupt Handler
// ==========================================
static volatile int button_pressed_flag = 0;
static volatile int interrupt_counter = 0;

static void IRAM_ATTR button_isr_handler(void *arg) {
    // Read GPIO interrupt status
    uint32_t status = GPIO_STATUS_REG;
    if (status & (1UL << BUTTON_GPIO)) {
        // Clear interrupt status for External button

        GPIO_STATUS_W1TC_REG = (1UL << BUTTON_GPIO);
        
        button_pressed_flag = 1;
        interrupt_counter++;
    }
}

void configure_button_interrupt(void) {
    // 1. Configure External Button (GPIO4) as input with pull-up
    IO_MUX_GPIO_REG(BUTTON_GPIO) &= ~IO_MUX_MCU_SEL_MASK;
    IO_MUX_GPIO_REG(BUTTON_GPIO) |= (IO_MUX_FUNC_GPIO << IO_MUX_MCU_SEL_SHIFT);
    IO_MUX_GPIO_REG(BUTTON_GPIO) |= (1UL << IO_MUX_MCU_IE_BIT);
    IO_MUX_GPIO_REG(BUTTON_GPIO) |= (1UL << IO_MUX_MCU_WPU_BIT);
    IO_MUX_GPIO_REG(BUTTON_GPIO) &= ~(1UL << IO_MUX_MCU_WPD_BIT);
    GPIO_ENABLE_W1TC_REG = (1UL << BUTTON_GPIO); // Disable output

    // 2. Configure GPIO falling-edge interrupt
    GPIO_STATUS_W1TC_REG = (1UL << BUTTON_GPIO); // Clear pending interrupt
    GPIO_PIN_REG(BUTTON_GPIO) &= ~GPIO_PIN_INT_TYPE_MASK;
    GPIO_PIN_REG(BUTTON_GPIO) |= (GPIO_INTR_NEGEDGE << GPIO_PIN_INT_TYPE_SHIFT); // Falling edge
    GPIO_PIN_REG(BUTTON_GPIO) |= GPIO_PIN_INT_ENA_CPU; // Enable CPU interrupt

    // 3. Register CPU Interrupt Source for GPIO
    esp_intr_alloc(
        ETS_GPIO_INTR_SOURCE,
        ESP_INTR_FLAG_IRAM,
        button_isr_handler,
        NULL,
        NULL
    );
    ESP_LOGI(TAG, "External Button (GPIO4) Interrupt configured");
}

// ==========================================
// Main Application Loop
// ==========================================

void app_main(void) {
    ESP_LOGI(TAG, "Starting Environment Monitor (Register Level)...");

    // Initialize I2C, SSD1306, and DHT20
    i2c_init();
    ssd1306_init();
    dht20_init();

    // Configure manual interrupt trigger (External button)
    configure_button_interrupt();

    // Display splash screen on LCD
    ssd1306_draw_string(1, 10, "====================");
    ssd1306_draw_string(2, 20, "ESP32-S3 MONITOR");
    ssd1306_draw_string(3, 16, "DHT20 & SSD1306");
    ssd1306_draw_string(4, 20, "Register Level");
    ssd1306_draw_string(5, 10, "====================");
    vTaskDelay(pdMS_TO_TICKS(2000));
    ssd1306_clear();

    // Display UI Outline
    ssd1306_draw_string(0, 0, "--- ENV MONITOR ---");
    ssd1306_draw_string(6, 0, "Last: Bootup");

    char lcd_buf[32];
    float temp = 0.0f;
    float hum = 0.0f;
    TickType_t last_sensor_read_tick = 0;

    // Perform initial reading
    if (dht20_read(&temp, &hum) == 0) {
        snprintf(lcd_buf, sizeof(lcd_buf), "Temp:  %.1f ^C", temp);
        ssd1306_draw_string(2, 0, lcd_buf);
        snprintf(lcd_buf, sizeof(lcd_buf), "Humid: %.1f %%", hum);
        ssd1306_draw_string(4, 0, lcd_buf);
        ESP_LOGI(TAG, "Sensor Data -> Temp: %.1f C, Humidity: %.1f%%", temp, hum);
    } else {
        ssd1306_draw_string(2, 0, "Temp:  ERROR");
        ssd1306_draw_string(4, 0, "Humid: ERROR");
    }

    while (1) {
        TickType_t current_tick = xTaskGetTickCount();
        int trigger_read = 0;
        const char *update_source = "Periodic";

        // Check if 5 seconds (5000 ms) have passed since the last sensor read
        if ((current_tick - last_sensor_read_tick) >= pdMS_TO_TICKS(5000)) {
            trigger_read = 1;
            update_source = "Periodic";
        }

        // Check if External button interrupt occurred
        if (button_pressed_flag) {
            button_pressed_flag = 0; // Clear flag
            trigger_read = 1;
            update_source = "Button ISR";
            ESP_LOGI(TAG, "[Interrupt] External button pressed! Reading sensor immediately. Press count: %d", interrupt_counter);
        }

        if (trigger_read) {
            last_sensor_read_tick = current_tick;

            // Read sensor and update LCD
            if (dht20_read(&temp, &hum) == 0) {
                snprintf(lcd_buf, sizeof(lcd_buf), "Temp:  %.1f ^C", temp);
                ssd1306_draw_string(2, 0, lcd_buf);
                snprintf(lcd_buf, sizeof(lcd_buf), "Humid: %.1f %%", hum);
                ssd1306_draw_string(4, 0, lcd_buf);
                
                snprintf(lcd_buf, sizeof(lcd_buf), "Last: %-9s (%d)", update_source, interrupt_counter);
                ssd1306_draw_string(6, 0, lcd_buf);
                
                ESP_LOGI(TAG, "[%s] Temp: %.1f C, Humidity: %.1f%%, ISR count: %d", 
                         update_source, temp, hum, interrupt_counter);
            } else {
                ssd1306_draw_string(2, 0, "Temp:  ERROR");
                ssd1306_draw_string(4, 0, "Humid: ERROR");
                snprintf(lcd_buf, sizeof(lcd_buf), "Last: ERROR (%d)", interrupt_counter);
                ssd1306_draw_string(6, 0, lcd_buf);
                ESP_LOGE(TAG, "Sensor read failed during %s update", update_source);
            }
        }

        // Yield to other tasks
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
