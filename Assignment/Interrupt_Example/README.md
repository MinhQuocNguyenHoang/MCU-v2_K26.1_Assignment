# Interrupt Example

Ví dụ cấu hình ngắt GPIO ở mức thanh ghi trên ESP32-S3. Nút nhấn tạo ngắt cạnh xuống để thay đổi tốc độ nháy của LED.

## Mục tiêu

- Cấu hình GPIO12 làm output và GPIO11 làm input.
- Cấu hình ngắt cạnh xuống cho GPIO11.
- Đăng ký ISR bằng `esp_intr_alloc()`.
- Xóa cờ ngắt và cập nhật thời gian delay trong ISR.

## Đấu nối

### LED

```text
ESP32-S3 GPIO12 ---- điện trở 220-330 ohm ----|>|---- GND
                                               LED
```

### Nút nhấn

```text
3.3 V ---- điện trở kéo lên 10 kohm ----+---- GPIO11
                                        |
                                     nút nhấn
                                        |
                                       GND
```

Code không bật pull-up nội, vì vậy GPIO11 cần điện trở kéo lên bên ngoài để tránh trạng thái input trôi. Chỉ sử dụng mức logic 3.3 V với ESP32-S3.

## Nguyên lý hoạt động

1. GPIO12 được bật output để điều khiển LED.
2. GPIO11 được cấu hình input và nhận ngắt cạnh xuống.
3. Khi nhấn nút, tín hiệu GPIO11 chuyển từ HIGH xuống LOW và gọi ISR.
4. ISR xóa cờ ngắt, sau đó giảm `delay_ms` đi 100 ms.
5. Khi delay nhỏ hơn 100 ms, giá trị được đưa về 1000 ms.

Tốc độ LED thay đổi theo chuỗi `1000, 900, 800, ... 100, 1000` ms cho mỗi trạng thái ON hoặc OFF.

## Các thanh ghi chính

| Thanh ghi | Chức năng |
| --- | --- |
| `GPIO_ENABLE_REG` | Bật hoặc tắt output driver. |
| `GPIO_OUT_W1TS_REG` | Đưa output lên HIGH. |
| `GPIO_OUT_W1TC_REG` | Đưa output xuống LOW. |
| `GPIO_STATUS_REG` | Đọc trạng thái pending của ngắt GPIO. |
| `GPIO_STATUS_W1TC_REG` | Xóa cờ ngắt bằng cách ghi `1`. |
| `GPIO_PIN0_REG(n)` | Cấu hình kiểu ngắt cho từng GPIO. |

## Build và chạy

Từ thư mục gốc repository:

```bash
cd Assignment/Interrupt_Example
idf.py set-target esp32s3
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

Thay `/dev/ttyUSB0` bằng cổng serial thực tế của board.

## Kết quả mong đợi

LED nháy chậm ở lần khởi động đầu tiên. Mỗi lần nhấn nút, LED nháy nhanh hơn; sau mức nhanh nhất, tốc độ quay lại giá trị ban đầu. Serial monitor liên tục hiển thị:

```text
LED ON
LED OFF
```

## Lưu ý

- Nút nhấn cơ khí có thể tạo nhiều cạnh do hiện tượng dội phím (button bounce). Chương trình hiện chưa xử lý debounce nên một lần nhấn thực tế có thể làm delay giảm nhiều bước.
- ISR phải được giữ ngắn gọn và không nên gọi các hàm blocking.
