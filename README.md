# MCU Programming Assignments

Repository lưu các bài tập lập trình vi điều khiển trên **ESP32-S3**, tập trung vào điều khiển ngoại vi ở mức thanh ghi thay vì sử dụng API GPIO cấp cao của ESP-IDF.

## Nội dung

| Bài tập | Mô tả |
| --- | --- |
| [GPIO Example](Assignment/Gpio_Example/) | Cấu hình GPIO12 làm output và nháy LED mỗi giây bằng thanh ghi GPIO. |
| [Interrupt Example](Assignment/Interrupt_Example/) | Bắt ngắt cạnh xuống từ nút nhấn GPIO11 để thay đổi tốc độ nháy LED GPIO12. |

## Phần cứng và công cụ

- Board ESP32-S3-DevKitC-1
- LED, điện trở hạn dòng 220-330 ohm
- Nút nhấn và điện trở kéo lên 10 kohm
- Cáp USB có khả năng truyền dữ liệu
- ESP-IDF và toolchain dành cho ESP32-S3

## Bắt đầu nhanh

Khởi tạo môi trường ESP-IDF trước khi build. Ví dụ trên Linux:

```bash
. "$HOME/esp/esp-idf/export.sh"
```

Đường dẫn cài đặt ESP-IDF có thể khác trên từng máy. Sau đó chọn một project và chạy:

```bash
cd Assignment/Gpio_Example
idf.py set-target esp32s3
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

Thay `/dev/ttyUSB0` bằng cổng serial của board. Nhấn `Ctrl+]` để thoát monitor.

## Cấu trúc repository

```text
.
├── Assignment/
│   ├── Gpio_Example/
│   └── Interrupt_Example/
└── document/
    ├── esp32-s3_datasheet_en.pdf
    ├── esp32-s3_technical_reference_manual_en.pdf
    ├── Espressif-ESP32-S3-User-Guide-devkitc-1-26-22.pdf
    └── SCH_ESP32-S3-DevKitC-1_V1.1_20220413.pdf
```

Chi tiết đấu nối và hoạt động của từng chương trình nằm trong README của từng bài tập.

## Tài liệu tham khảo

Các datasheet, technical reference manual, user guide và schematic cần thiết đã được lưu trong thư mục [`document`](document/).

## Lưu ý

Mã nguồn truy cập trực tiếp các địa chỉ thanh ghi của ESP32-S3, vì vậy không nên dùng nguyên trạng cho dòng chip khác. Luôn kiểm tra pinout của đúng phiên bản board trước khi cấp nguồn.
