# Danh sách bài tập

Thư mục này gồm các project ESP-IDF độc lập dành cho ESP32-S3 và bài nghiên cứu kiến trúc máy tính. Mỗi project thực hành có `CMakeLists.txt` riêng và cần được build từ chính thư mục của project đó.

## Bài tập hiện có

### 1. GPIO Register-Level Programming

Thư mục: [`Gpio_Example`](Gpio_Example/)

- Cấu hình GPIO12 làm output.
- Ghi các thanh ghi `GPIO_OUT_W1TS_REG` và `GPIO_OUT_W1TC_REG`.
- Bật/tắt LED theo chu kỳ 1 giây.

### 2. GPIO Interrupt

Thư mục: [`Interrupt_Example`](Interrupt_Example/)

- Dùng GPIO12 điều khiển LED.
- Dùng GPIO11 nhận tín hiệu nút nhấn.
- Cấu hình ngắt cạnh xuống trực tiếp trên thanh ghi GPIO.
- Mỗi lần nhấn nút giảm thời gian delay 100 ms; sau khi xuống dưới 50 ms, delay trở về 1000 ms.

### 3. Nghiên cứu kiến trúc CISC và RISC

Báo cáo: [`Research_CISCV_RISCV/bao_cao_so_sanh_cisc_risc.md`](Research_CISCV_RISCV/bao_cao_so_sanh_cisc_risc.md)

- Giới thiệu đặc điểm của kiến trúc CISC và RISC.
- Phân tích ưu điểm, nhược điểm và so sánh theo các tiêu chí chính.
- Trình bày ứng dụng thực tế và đánh giá kiến trúc phù hợp cho hệ thống nhúng.

## Cách chạy chung

```bash
cd Assignment/<ten_project>
idf.py set-target esp32s3
idf.py build
idf.py -p <serial_port> flash monitor
```

Ví dụ `<serial_port>` thường là `/dev/ttyUSB0`, `/dev/ttyACM0` trên Linux hoặc `COM3` trên Windows.

Xem README trong từng project thực hành để biết sơ đồ đấu nối và kết quả mong đợi.
