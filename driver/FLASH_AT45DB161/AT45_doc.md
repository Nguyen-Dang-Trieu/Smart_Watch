Pin AT45DB161
- Pin CS: Chip select (active Low) 
- Pin SO: Serial Ouput (MISO)
- Pin SI: Serial Input (MOSI)
- Pin WP: Write Protect (actice LOW) - thuong keo HIGH neu khong dung.
- Pin SCK: Xung Clock
- Pin RESET: Reset IC
- Pin VCC, GND

Giao thuc SPI dung 4 chan: CS, SO, SI, SCK

## Thu tu truyen bit du lieu trong AT45DB161
* Tat ca cac lenh command, dia chi, du lieu deu duoc xu ly theo kieu MSB (Most Significant Bit). Dieu nay co nghia la bit co trong so cau nhat
se ra khoi MCU truoc

VD: Command 0xD2 (0b 1101 0010)
- Bit 7 = 1 -> Se duoc gui di truoc roi lan luot toi cac bit khac.

## Tinh nang va cac thanh ghi cua flash
### Additional Commands
#### Status Register Read
Dung de xac dinh
- Trang thai ready/busy cua device. (Bit 7) - User can phai chuyen doi chan SCK de kiem tra ready/busy.
- Kich thuoc page
- Ket qua so sanh cua Main memory va buffer
- Trang thai Sector protection
- So luong thiet bi.

Thanh ghi nay co the doc bat ki thoi diem nao. Dieu kien kich hoat: CS = 0, opcode = 0xD7 dua vao device.   
Byte du lieu trong thanh ghi bat dau tu bit MSB (Bit 7) se di ra chan SO. Sau khi byte cua status reg duoc output, qua trinh se lap lai (mien 
la CS = 0, va xung CLK van hoat dong).


Sau khi Power-on/Reset
- Flash se hoat dong o MODE 3. -> Neu MCU cau hinh SPI Mode 0, dan den hoat dong sai. Can cau hinh cho dung.
- SO se o trang thai tro khang cao -> Chi khi CS duoc keo tu HIGH -> LOW, flash moi bat dau nhan lenh.