# Bài tập: Đọc nhiệt độ/độ ẩm từ DHT20 (I2C) và hiển thị lên màn hình LCD SSD1306 (I2C) ở mức thanh ghi (Register Level)

Bài tập này thực hiện cấu hình trực tiếp các thanh ghi GPIO, IO_MUX, Interrupt Matrix và CPU Interrupt để triển khai giao thức I2C phần mềm (Software I2C) điều khiển cảm biến DHT20 và màn hình OLED SSD1306 trên nền tảng **ESP32-S3 DevKitC v1** (ESP-IDF SDK) mà không sử dụng bất kỳ thư viện driver API cao cấp nào (`driver/i2c.h` hoặc `driver/gpio.h`).

---

## 1. Sơ đồ kết nối phần cứng (Hardware Connections)

| Thiết bị | Chân ESP32-S3 DevKitC v1 | Ghi chú |
| :--- | :--- | :--- |
| **SSD1306 & DHT20 VCC** | **3.3V** | Nguồn nuôi cảm biến & màn hình |
| **SSD1306 & DHT20 GND** | **GND** | Nguồn chung |
| **SSD1306 & DHT20 SCL** | **GPIO 1** | Đường truyền Clock I2C (Bus chung) |
| **SSD1306 & DHT20 SDA** | **GPIO 2** | Đường truyền Data I2C (Bus chung) |
| **Nút nhấn rời** | **GPIO 4** | Nút nhấn ngoài dùng kích hoạt ngắt ngoài (nối GPIO 4 với GND) |

> [!IMPORTANT]
> **Điện trở kéo lên (Pull-up resistors):**
> Code cấu hình đã bật điện trở kéo lên nội bộ (internal pull-ups) cho chân GPIO 1 và 2. Tuy nhiên, để đảm bảo giao tiếp I2C chạy ổn định và tránh lỗi nhiễu tín hiệu, bạn nên đấu song song **2 điện trở kéo lên ngoài 4.7kΩ** từ chân SDA/SCL lên nguồn 3.3V.

---

## 2. Giải pháp thiết kế & Xử lý thanh ghi (Register-Level Design)

### 2.1. Cấu hình GPIO & IO_MUX làm Open-Drain I2C
* Để mô phỏng đúng chuẩn giao tiếp vật lý của I2C, các chân SDA và SCL phải được cấu hình ở chế độ **Open-Drain**.
* Ghi đè cấu hình IO_MUX của GPIO1 và GPIO2:
  * Chọn GPIO function bằng cách xóa trường `MCU_SEL` về `1` (GPIO function).
  * Bật bit `IE` (Input Enable - bit 9) để cho phép đọc dữ liệu điện áp phản hồi trực tiếp từ chân qua thanh ghi `GPIO_IN_REG` (ngay cả khi chân đang là Output).
  * Bật bit `WPU` (Pull-up Enable - bit 8) và tắt bit `WPD` (Pull-down Enable - bit 7).
* Kích hoạt chế độ Open-Drain bằng cách set bit `PAD_DRIVER` (bit 2) trong thanh ghi cấu hình của từng chân `GPIO_PIN_REG(n)`.
* Bật Output driver cho cả hai chân thông qua thanh ghi `GPIO_ENABLE_W1TS_REG`.

### 2.2. Trình điều khiển giao thức I2C bằng phần mềm (Software I2C Driver)
* Triển khai các hàm điều khiển cơ bản bằng cách thao tác thanh ghi bật/tắt GPIO:
  * `i2c_start()`: SDA kéo xuống LOW trước khi SCL xuống LOW.
  * `i2c_stop()`: SDA kéo lên HIGH sau khi SCL lên HIGH.
  * `i2c_write_byte(uint8_t byte)`: Gửi từng bit ra SDA, nhịp SCL, sau đó giải phóng SDA và đọc phản hồi ACK/NACK từ slave.
  * `i2c_read_byte(int ack)`: Đọc từng bit từ SDA ứng với mỗi nhịp SCL, gửi phản hồi ACK/NACK từ Master.
* Hỗ trợ cơ chế **Clock Stretching**: SCL Master sẽ chờ (đợi SCL pin vật lý lên mức cao thực sự) để đề phòng trường hợp thiết bị Slave kéo thấp SCL để có thêm thời gian xử lý dữ liệu.

### 2.3. Cấu hình Ngắt ngoài (External Interrupt) trên nút nhấn rời
* Sử dụng nút nhấn ngoài nối với **GPIO 4** và GND để tạo ngắt:
  * Cấu hình GPIO 4 làm Input, bật điện trở kéo lên nội bộ (Pull-up).
  * Cấu hình kiểu ngắt cạnh xuống (Falling Edge) bằng thanh ghi `GPIO_PIN_REG(BUTTON_GPIO)`.
  * Đăng ký ISR handler cho nguồn ngắt `ETS_GPIO_INTR_SOURCE` với CPU thông qua hàm `esp_intr_alloc()`.
  * Khi nhấn nút, chương trình nhảy vào ISR, xóa cờ ngắt `GPIO_STATUS_W1TC_REG`, tăng biến đếm ngắt và kích hoạt cờ đọc cảm biến lập tức.

---

## 3. Hoạt động của Chương trình (Application Flow)

1. **Khởi động:** Khởi tạo I2C phần mềm, cấu hình màn hình SSD1306 (Page Addressing Mode), cấu hình cảm biến DHT20, cài đặt ngắt nút nhấn ngoài.
2. **Splash Screen:** Màn hình OLED hiển thị tên bài học trong 2 giây rồi xóa màn hình.
3. **Vòng lặp chính:**
   * **Chu kỳ 5 giây:** Tự động đọc nhiệt độ và độ ẩm từ DHT20.
   * **Ngắt Button:** Khi nhấn nút nhấn ngoài (kéo GPIO 4 xuống GND), ngắt xảy ra ngay lập tức, bỏ qua thời gian chờ 5s, thực hiện đo nhiệt độ/độ ẩm mới và hiển thị nguồn kích hoạt là `Button ISR`.
   * **Hiển thị LCD:**
     * Dòng 1: Tiêu đề `--- ENV MONITOR ---`
     * Dòng 2: Giá trị nhiệt độ đọc được (ví dụ `Temp:  28.5 °C`)
     * Dòng 3: Giá trị độ ẩm đọc được (ví dụ `Humid: 65.2 %`)
     * Dòng 4: Nguồn cập nhật cuối và số lần nhấn nút (ví dụ `Last: Button ISR (3)`)

---

## 4. Biên dịch và Nạp chương trình (Build & Flash)

Để biên dịch và nạp chương trình xuống ESP32-S3 DevKitC v1, thực hiện các lệnh sau tại thư mục của bài tập:

```bash
# 1. Kích hoạt môi trường ESP-IDF (nếu chưa cấu hình)
. $HOME/esp/esp-idf/export.sh

# 2. Thiết lập chip target
idf.py set-target esp32s3

# 3. Biên dịch chương trình
idf.py build

# 4. Nạp code và mở màn hình giám sát Serial
idf.py -p <PORT> flash monitor
# Ví dụ: idf.py -p /dev/ttyUSB0 flash monitor
```

Nhấn nút nhấn ngoài (nối chân GPIO 4 vào GND) để kiểm tra tính năng ngắt ngoài (đọc dữ liệu tức thời và cập nhật nguồn lên màn hình).
