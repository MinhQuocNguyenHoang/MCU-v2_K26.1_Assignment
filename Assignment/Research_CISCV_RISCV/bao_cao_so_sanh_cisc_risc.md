# Báo cáo: So sánh kiến trúc CISC và RISC

## 1. Giới thiệu

Trong lĩnh vực vi xử lý và vi điều khiển, kiến trúc tập lệnh là một phần rất quan trọng vì nó quy định cách CPU hiểu và thực hiện các lệnh của chương trình. Hai kiến trúc thường được nhắc đến nhiều là **CISC** và **RISC**. Đây là hai hướng thiết kế CPU khác nhau, mỗi loại có ưu điểm, nhược điểm và phạm vi ứng dụng riêng.

**CISC** là viết tắt của *Complex Instruction Set Computer*, nghĩa là máy tính có tập lệnh phức tạp. Đặc điểm chính của kiến trúc này là có nhiều lệnh, trong đó có những lệnh thực hiện được các thao tác khá phức tạp chỉ trong một câu lệnh. Ví dụ, một lệnh có thể vừa truy cập bộ nhớ, vừa xử lý dữ liệu, sau đó lưu kết quả lại. Mục tiêu ban đầu của CISC là giúp chương trình ngắn hơn, giảm số lượng lệnh mà lập trình viên hoặc trình biên dịch cần tạo ra.

**RISC** là viết tắt của *Reduced Instruction Set Computer*, nghĩa là máy tính có tập lệnh rút gọn. Kiến trúc này sử dụng các lệnh đơn giản hơn, thường có độ dài cố định và mỗi lệnh chỉ thực hiện một công việc cơ bản. Thay vì dùng một lệnh phức tạp, RISC sẽ chia công việc thành nhiều lệnh nhỏ hơn. Điều này giúp phần cứng xử lý lệnh đơn giản hơn, dễ tối ưu tốc độ và phù hợp với kỹ thuật pipeline.

Theo em hiểu, có thể xem CISC là hướng thiết kế “một lệnh làm nhiều việc”, còn RISC là hướng thiết kế “nhiều lệnh đơn giản phối hợp với nhau”. Trong thực tế hiện nay, ranh giới giữa CISC và RISC không còn hoàn toàn tách biệt như lý thuyết ban đầu, nhưng việc so sánh hai kiến trúc này vẫn rất quan trọng để hiểu cách CPU hoạt động.

## 2. Ưu điểm và nhược điểm của kiến trúc CISC

### 2.1. Ưu điểm

Ưu điểm lớn của CISC là tập lệnh rất phong phú. Vì có nhiều lệnh phức tạp nên một chương trình có thể được viết với số lượng lệnh ít hơn. Điều này giúp kích thước chương trình có thể nhỏ hơn, đặc biệt trong giai đoạn trước đây khi bộ nhớ còn đắt và dung lượng hạn chế.

CISC cũng giúp trình biên dịch có nhiều lựa chọn hơn khi chuyển đổi từ ngôn ngữ cấp cao sang mã máy. Một số thao tác phức tạp trong ngôn ngữ lập trình có thể được ánh xạ gần hơn với các lệnh có sẵn trong CPU. Ngoài ra, CISC phù hợp với các hệ thống máy tính cá nhân và máy chủ, nơi yêu cầu khả năng tương thích phần mềm qua nhiều thế hệ là rất quan trọng.

Một ví dụ điển hình của kiến trúc CISC là dòng vi xử lý **x86** của Intel và AMD. Các CPU này được sử dụng rộng rãi trong máy tính cá nhân, laptop và server. Một lý do quan trọng là hệ sinh thái phần mềm của x86 đã phát triển trong thời gian dài, nên khả năng tương thích ngược là một lợi thế rất lớn.

### 2.2. Nhược điểm

Nhược điểm của CISC là phần cứng CPU thường phức tạp hơn. Vì tập lệnh có nhiều lệnh với độ phức tạp khác nhau, bộ điều khiển bên trong CPU phải xử lý nhiều trường hợp hơn. Điều này làm cho việc thiết kế, kiểm thử và tối ưu CPU trở nên khó hơn.

Một số lệnh CISC có thể mất nhiều chu kỳ máy để hoàn thành. Do đó, việc dự đoán thời gian thực thi của từng lệnh không đơn giản bằng RISC. Ngoài ra, vì lệnh có thể có độ dài khác nhau nên việc giải mã lệnh cũng phức tạp hơn.

Trong các hệ thống nhúng nhỏ, nơi yêu cầu tiết kiệm năng lượng và chi phí thấp, kiến trúc CISC không phải lúc nào cũng là lựa chọn tối ưu. Phần cứng phức tạp hơn có thể làm tăng mức tiêu thụ điện và diện tích chip.

## 3. Ưu điểm và nhược điểm của kiến trúc RISC

### 3.1. Ưu điểm

Ưu điểm nổi bật của RISC là tập lệnh đơn giản, dễ giải mã và dễ thực thi. Mỗi lệnh thường thực hiện một thao tác cơ bản, ví dụ như cộng, trừ, nạp dữ liệu từ bộ nhớ hoặc lưu dữ liệu xuống bộ nhớ. Do lệnh đơn giản và thường có độ dài cố định, CPU có thể xử lý lệnh nhanh hơn và dễ áp dụng pipeline.

Pipeline là kỹ thuật chia quá trình xử lý lệnh thành nhiều giai đoạn như nạp lệnh, giải mã lệnh, thực thi và ghi kết quả. Kiến trúc RISC rất phù hợp với kỹ thuật này vì các lệnh có cấu trúc tương đối đồng đều. Nhờ đó, hiệu năng xử lý có thể được cải thiện.

RISC cũng thường có phần cứng đơn giản hơn so với CISC. Điều này giúp giảm diện tích chip, giảm tiêu thụ năng lượng và giảm chi phí sản xuất. Vì vậy, RISC được sử dụng rất nhiều trong hệ thống nhúng, điện thoại, thiết bị IoT và vi điều khiển.

Một ví dụ quen thuộc là kiến trúc **ARM**, được dùng trong rất nhiều vi điều khiển như STM32, NXP, Nordic, cũng như trong điện thoại thông minh. Ngoài ra, **RISC-V** cũng là một kiến trúc RISC mở đang được quan tâm nhiều trong nghiên cứu, giáo dục và thiết kế chip.

### 3.2. Nhược điểm

Nhược điểm của RISC là do mỗi lệnh khá đơn giản nên để thực hiện một công việc phức tạp, chương trình có thể cần nhiều lệnh hơn so với CISC. Điều này có thể làm kích thước chương trình tăng lên trong một số trường hợp.

Ngoài ra, RISC phụ thuộc khá nhiều vào trình biên dịch. Vì CPU không cung cấp quá nhiều lệnh phức tạp, trình biên dịch phải tối ưu cách sắp xếp các lệnh đơn giản để chương trình chạy hiệu quả. Nếu trình biên dịch tối ưu không tốt, hiệu năng thực tế có thể chưa đạt như mong muốn.

Tuy nhiên, với sự phát triển của công nghệ trình biên dịch hiện nay, nhược điểm này đã được giảm đi khá nhiều. Các trình biên dịch như GCC, LLVM có khả năng tối ưu tốt cho nhiều kiến trúc RISC khác nhau.

## 4. So sánh CISC và RISC theo các tiêu chí

| Tiêu chí | CISC | RISC |
|---|---|---|
| Cấu trúc tập lệnh | Tập lệnh lớn, nhiều lệnh phức tạp, độ dài lệnh có thể thay đổi | Tập lệnh nhỏ hơn, lệnh đơn giản, thường có độ dài cố định |
| Tốc độ xử lý | Một số lệnh mạnh nhưng có thể mất nhiều chu kỳ máy | Lệnh đơn giản, dễ pipeline, thường thực thi nhanh và đều hơn |
| Kích thước chương trình | Có thể nhỏ hơn vì một lệnh làm được nhiều việc | Có thể lớn hơn vì cần nhiều lệnh đơn giản để làm việc phức tạp |
| Độ phức tạp phần cứng | Phần cứng phức tạp hơn, giải mã lệnh khó hơn | Phần cứng đơn giản hơn, dễ thiết kế và tối ưu |
| Ứng dụng thực tế | Máy tính cá nhân, laptop, server; ví dụ Intel x86, AMD x86 | Vi điều khiển, điện thoại, hệ thống nhúng, IoT; ví dụ ARM, RISC-V, MIPS |

### 4.1. Cấu trúc tập lệnh

CISC có tập lệnh lớn và đa dạng. Một lệnh có thể làm nhiều thao tác, thậm chí làm việc trực tiếp với dữ liệu trong bộ nhớ. Điều này giúp chương trình có thể ngắn hơn, nhưng làm cho CPU phải có bộ giải mã lệnh phức tạp.

Ngược lại, RISC dùng tập lệnh rút gọn. Các lệnh thường chỉ làm một thao tác đơn giản. Ví dụ, muốn xử lý dữ liệu trong bộ nhớ thì CPU thường phải nạp dữ liệu vào thanh ghi trước, sau đó mới thực hiện phép tính. Cách này làm chương trình có vẻ dài hơn, nhưng phần cứng lại dễ tối ưu hơn.

### 4.2. Tốc độ xử lý

Nếu xét từng lệnh riêng lẻ, một lệnh CISC có thể làm được nhiều việc hơn một lệnh RISC. Tuy nhiên, lệnh CISC phức tạp nên có thể mất nhiều chu kỳ để hoàn thành. Trong khi đó, RISC hướng tới việc thực hiện lệnh nhanh, đều và dễ dự đoán hơn.

Theo em, điểm mạnh của RISC không chỉ nằm ở từng lệnh đơn giản, mà còn ở khả năng tổ chức pipeline tốt. Khi các lệnh có cấu trúc giống nhau, CPU dễ chia quá trình xử lý thành nhiều giai đoạn, từ đó tăng thông lượng xử lý.

### 4.3. Kích thước chương trình

CISC thường có lợi thế về kích thước chương trình vì một lệnh có thể thay thế cho nhiều lệnh nhỏ. Điều này đặc biệt có ý nghĩa trong thời kỳ bộ nhớ còn hạn chế.

RISC có thể cần nhiều lệnh hơn để thực hiện cùng một chức năng, nên kích thước chương trình đôi khi lớn hơn. Tuy nhiên, trong các hệ thống hiện nay, bộ nhớ đã rẻ hơn trước và trình biên dịch cũng tối ưu tốt hơn, nên sự khác biệt này không phải lúc nào cũng quá lớn.

### 4.4. Độ phức tạp phần cứng

CISC có phần cứng phức tạp hơn do phải hỗ trợ nhiều kiểu lệnh, nhiều chế độ địa chỉ và các lệnh có độ dài khác nhau. Việc giải mã và điều khiển thực thi lệnh vì vậy cũng khó hơn.

RISC có phần cứng đơn giản hơn do tập lệnh ít và có cấu trúc rõ ràng. Điều này giúp CPU dễ thiết kế, dễ kiểm thử và có thể tiết kiệm điện năng hơn. Đây là lý do RISC rất phù hợp với vi điều khiển và thiết bị nhúng.

### 4.5. Ứng dụng thực tế

Trong thực tế, CISC thường xuất hiện trong các bộ xử lý x86 của Intel và AMD. Các CPU này được dùng nhiều trong máy tính cá nhân, laptop và server. Ưu điểm chính là hiệu năng cao và khả năng chạy được rất nhiều phần mềm cũ lẫn mới.

RISC lại phổ biến trong hệ thống nhúng và thiết bị di động. Kiến trúc ARM được dùng rất rộng rãi trong vi điều khiển STM32, điện thoại, tablet và nhiều thiết bị IoT. RISC-V cũng đang phát triển mạnh vì có tính mở, phù hợp cho nghiên cứu và thiết kế vi xử lý tùy chỉnh.

## 5. Quan điểm cá nhân về kiến trúc phù hợp cho hệ thống nhúng hiện nay

Theo quan điểm cá nhân của em, trong bối cảnh phát triển hệ thống nhúng hiện nay, kiến trúc **RISC** phù hợp hơn so với CISC. Lý do chính là hệ thống nhúng thường có các yêu cầu như tiết kiệm điện năng, chi phí thấp, kích thước nhỏ, độ ổn định cao và khả năng đáp ứng thời gian thực.

Đối với các vi điều khiển như STM32, ESP32 hoặc các dòng ARM Cortex-M, tài nguyên phần cứng thường không quá lớn như máy tính cá nhân. Vì vậy, một kiến trúc có tập lệnh đơn giản, phần cứng gọn và dễ tối ưu như RISC sẽ phù hợp hơn. Khi học lập trình MCU, em thấy các thao tác thường xoay quanh GPIO, UART, SPI, I2C, Timer, ADC và Interrupt. Những tác vụ này cần phản hồi nhanh, ổn định và tiết kiệm năng lượng hơn là cần một tập lệnh quá phức tạp.

Ngoài ra, hệ thống nhúng ngày nay xuất hiện rất nhiều trong thiết bị di động, cảm biến, robot nhỏ, thiết bị IoT và các bộ điều khiển công nghiệp. Các thiết bị này thường chạy bằng pin hoặc cần hoạt động liên tục trong thời gian dài. Vì vậy, việc giảm tiêu thụ điện là rất quan trọng. Kiến trúc RISC, đặc biệt là ARM Cortex-M, đã chứng minh được ưu thế trong lĩnh vực này.

Tuy nhiên, điều đó không có nghĩa là CISC không còn quan trọng. Với máy tính cá nhân, laptop hoặc server, kiến trúc x86 vẫn rất mạnh nhờ hiệu năng cao và hệ sinh thái phần mềm lớn. Nhưng nếu xét riêng trong phạm vi hệ thống nhúng, em cho rằng RISC là lựa chọn hợp lý hơn.

## 6. Kết luận

CISC và RISC là hai hướng thiết kế kiến trúc CPU khác nhau. CISC tập trung vào tập lệnh phong phú, mỗi lệnh có thể thực hiện nhiều thao tác, giúp chương trình có thể ngắn hơn nhưng làm phần cứng phức tạp hơn. RISC tập trung vào các lệnh đơn giản, dễ thực thi, dễ pipeline và phù hợp với phần cứng gọn nhẹ.

Qua việc so sánh, có thể thấy CISC phù hợp với các hệ thống cần khả năng tương thích phần mềm mạnh như máy tính cá nhân và server. Trong khi đó, RISC phù hợp hơn với hệ thống nhúng, thiết bị di động và vi điều khiển vì ưu điểm về hiệu năng trên mỗi watt, độ đơn giản phần cứng và khả năng tối ưu cho thời gian thực.

Theo em, đối với sinh viên học về MCU và hệ thống nhúng, việc hiểu kiến trúc RISC là rất cần thiết vì hầu hết các vi điều khiển phổ biến hiện nay đều dựa trên hướng thiết kế này. Tuy nhiên, việc nắm được sự khác nhau giữa CISC và RISC cũng giúp em có cái nhìn tổng quan hơn về cách CPU được thiết kế và vì sao mỗi kiến trúc lại phù hợp với từng loại ứng dụng khác nhau.
