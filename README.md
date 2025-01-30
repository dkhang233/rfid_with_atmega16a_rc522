### Tổng quan

Đọc thẻ rfid bằng rc522. Sau đó, dữ liệu được gửi về vi xử lí ATmega16a qua giao tiếp SPI. Vi xử lí ghi dữ liệu ra màn hình lcd để hiển thị.

### Cách chạy chương trình

<ul>
<li> Kết nối các chân
<ul>
<li>RC522 với ATmega16a

| RC522 Pin | SDA | SCK | MOSI | MISO | IRQ | GND | RST | 3.3 V |
| --------- | --- | --- | ---- | ---- | --- | --- | --- | ----- |
| ATmega16a | PB4 | PB7 | PB5  | PB6  | -   | GND | PB0 | 3.3 V |

<li> LCD với ATmega16a

| 1   | 2   | 3   | 4   | 5   | 6   | 7   | 8   | 9   | 10  | 11  | 12  | 13  | 14  | 15  | 16  |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Vss | Vdd | Vo  | RS  | R/W | E   | -   | -   | -   | -   | D4  | D5  | D6  | D7  | A   | K   |

</ul>
<li> Download chương trình và build bằng Microchip Studio
<li> Nạp chương trình xuống vi xử lí, có thể sử dụng các tool như: progisp
</ul>

### Demo

[](https://github.com/dkhang233/rfid_with_atmega16a_rc522/blob/main/demo.jpg)
