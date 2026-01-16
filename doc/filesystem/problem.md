
nếu copy số lượng byte lớn vào vào kích thước của buffer sẽ gây ra bug segment fault

Mỗi lần dùng hàm memcpy() thì hãy thận trọng, bởi vì rất dễ xảy ra tình trạng là segment fault 
- Hãy chú ý tham số thứ 3 của hàm, tránh copy vùng nhớ không hợp lệ