CC      := gcc
# Thêm -Itest để compiler tìm thấy các file header nếu có trong thư mục test
CFLAGS  := -Wall -Wextra -Idebug -IFLASH -IFTL -IS-FAT -Itest
OBJ_DIR := build
TARGET  := run.exe

# Sửa lại SRCS để quét toàn bộ các thư mục, bao gồm cả thư mục test
SRCS := $(wildcard debug/*.c FLASH/*.c FTL/*.c S-FAT/*.c main.c)
OBJS := $(SRCS:%.c=$(OBJ_DIR)/%.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(dir $@)
	@echo "[LD] Linking $@"
	$(CC) $(OBJS) -o $@

$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	@echo "[CC] Compiling $<"
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(TARGET)

.PHONY: all clean