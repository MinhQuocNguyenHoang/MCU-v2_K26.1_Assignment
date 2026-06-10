# GPIO Example

Ví dụ điều khiển LED bằng cách truy cập trực tiếp các thanh ghi GPIO của ESP32-S3.

## Mục tiêu

- Cấu hình GPIO12 làm output.
- Hiểu cơ chế thanh ghi write-one-to-set và write-one-to-clear.
- Tạo chu kỳ LED sáng 1 giây, tắt 1 giây bằng FreeRTOS delay.

## Đấu nối

```text
ESP32-S3 GPIO12 ---- điện trở 220-330 ohm ----|>|---- GND
                                               LED
```

| Linh kiện | Kết nối |
| --- | --- |
| Chân dương LED (anode) | GPIO12 qua điện trở hạn dòng |
| Chân âm LED (cathode) | GND |

Không nối LED trực tiếp vào GPIO nếu không có điện trở hạn dòng.

## Nguyên lý hoạt động

Chương trình sử dụng GPIO base address `0x60004000` của ESP32-S3:

| Thanh ghi | Offset | Chức năng |
| --- | --- | --- |
| `GPIO_OUT_W1TS_REG` | `0x0008` | Ghi bit `1` để đưa output tương ứng lên HIGH. |
| `GPIO_OUT_W1TC_REG` | `0x000C` | Ghi bit `1` để đưa output tương ứng xuống LOW. |
| `GPIO_ENABLE_W1TS_REG` | `0x0024` | Ghi bit `1` để bật output driver cho GPIO tương ứng. |

Sau khi bật output cho GPIO12, vòng lặp lần lượt set và clear bit 12, với thời gian chờ 1000 ms giữa hai trạng thái.

## Build và chạy

Từ thư mục gốc repository:

```bash
cd Assignment/Gpio_Example
idf.py set-target esp32s3
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

Thay `/dev/ttyUSB0` bằng cổng serial thực tế của board.

## Kết quả mong đợi

LED nháy đều với chu kỳ 2 giây. Serial monitor hiển thị:

```text
LED ON
LED OFF
LED ON
LED OFF
```

### Video kết quả

<div align="center">
  <video src="https://github.com/user-attachments/assets/4c5cf7d7-7477-4bb4-ae4e-d86e9db45daf" alt="Blynk LED" height="200"></video>
</div>
