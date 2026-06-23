# UART LED Control

**Bài tập 1.2 & 1.3** – Cấu hình GPIO, UART, Interrupt và điều khiển LED thông qua lệnh gửi từ PC.

Phần cứng: **ESP32-S3 DevKitC v1**  
Terminal: PuTTY, Hercules, hoặc `idf.py monitor`

---

## Sơ đồ đấu nối

| GPIO | Chức năng    | Nối với                        |
|------|-------------|--------------------------------|
| 12   | LED thường  | Anode → 330 Ω → GND           |
| 13   | RGB – Đỏ   | Anode → 330 Ω → GND           |
| 14   | RGB – Xanh lá | Anode → 330 Ω → GND        |
| 15   | RGB – Xanh dương | Anode → 330 Ω → GND     |
| 43   | UART0 TX    | USB-Serial (mặc định DevKitC)  |
| 44   | UART0 RX    | USB-Serial (mặc định DevKitC)  |

> **Lưu ý:** GPIO43/44 là chân UART0 mặc định trên ESP32-S3 DevKitC v1.  
> Kết nối qua cổng USB của board là đủ, không cần dây thêm.

---

## Thông số UART

| Tham số     | Giá trị     |
|-------------|-------------|
| Baud rate   | 115 200     |
| Data bits   | 8           |
| Parity      | None        |
| Stop bits   | 1           |
| Flow control| None        |

---

## Tập lệnh

Gõ lệnh trên terminal rồi nhấn **Enter** (xuống dòng `\n`).

| Lệnh        | Mô tả                        |
|-------------|------------------------------|
| `LED_ON`    | Bật LED thường (GPIO12)      |
| `LED_OFF`   | Tắt LED thường (GPIO12)      |
| `RED_ON`    | Bật LED đỏ (GPIO13)          |
| `RED_OFF`   | Tắt LED đỏ (GPIO13)          |
| `GREEN_ON`  | Bật LED xanh lá (GPIO14)     |
| `GREEN_OFF` | Tắt LED xanh lá (GPIO14)     |
| `BLUE_ON`   | Bật LED xanh dương (GPIO15)  |
| `BLUE_OFF`  | Tắt LED xanh dương (GPIO15)  |

ESP32 sẽ phản hồi `OK: <lệnh>` khi thành công, hoặc `ERR: Unknown command` nếu lệnh không hợp lệ.

---

## Thiết kế phần mềm

```
app_main()
├── gpio_set_output()   ← Cấu hình GPIO12-15 làm output (register-level)
├── uart_param_config() ← Cấu hình UART0 (baud, 8N1)
├── uart_driver_install()← Cài UART driver với interrupt + ring-buffer nội bộ
└── xTaskCreate(uart_rx_task)
        └── uart_read_bytes() ← Đọc từng byte, gom thành lệnh
                └── execute_command() ← Phân tích chuỗi và điều khiển LED
```

- **GPIO**: Cấu hình trực tiếp trên thanh ghi `IO_MUX_GPIO_REG`, `GPIO_ENABLE_W1TS_REG`, `GPIO_OUT_W1TS_REG`, `GPIO_OUT_W1TC_REG` (register-level, nhất quán với các bài trước).
- **UART**: Dùng `uart_driver_install()` – driver tự dùng ngắt nội bộ (FIFO full / timeout) để nạp byte vào ring-buffer. Task đọc ring-buffer bằng `uart_read_bytes()`.
- **Interrupt**: Ngắt UART được ESP-IDF driver quản lý nội bộ. Driver kích hoạt `UART_INTR_RXFIFO_FULL` và `UART_INTR_RXFIFO_TOUT` để không mất ký tự.
- **Echo + Prompt**: Board echo lại ký tự nhận được và in `>> ` sau mỗi lệnh, tiện dùng với PuTTY ở chế độ "Line" hoặc "Character".

---

## Cách build và flash

```bash
cd Assignment/UART_LED_Control
idf.py set-target esp32s3
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

Thay `/dev/ttyUSB0` bằng cổng COM thực tế (Linux: `/dev/ttyACM0`, Windows: `COM3`).

---

## Kết quả mong đợi

```
=== UART LED Control (ESP32-S3) ===
Commands: LED_ON/OFF  RED_ON/OFF  GREEN_ON/OFF  BLUE_ON/OFF
>> LED_ON
OK: LED ON
>> RED_ON
OK: RED ON
>> GREEN_OFF
OK: GREEN OFF
>> HELLO
ERR: Unknown command "HELLO"
>>
```
