CROSS_COMPILE ?= arm-none-eabi-

CC = $(CROSS_COMPILE)gcc -Iinclude
OBJCOPY = $(CROSS_COMPILE)objcopy
SIZE = $(CROSS_COMPILE)size

BUILD_DIR = build
SRCS = src/main.c src/keyboard.c src/tm1637.c src/syscalls.c
ASMS = ld/startup_stm32c031c6tx.s
OBJS = $(SRCS:%.c=$(BUILD_DIR)/%.o) $(ASMS:%.s=$(BUILD_DIR)/%.o)

ELF = project.elf
HEX = project.hex

CFLAGS = -mcpu=cortex-m0plus -mthumb -Wall -O0 -g \
         -ffreestanding -specs=nano.specs
LDSCRIPT = ld/STM32C031C6TX.ld
LDFLAGS = -T $(LDSCRIPT) -lc -lgcc

all: $(BUILD_DIR) $(BUILD_DIR)/$(ELF) $(BUILD_DIR)/$(HEX)

$(BUILD_DIR):
	mkdir $(BUILD_DIR)
	mkdir $(BUILD_DIR)\ld
	mkdir $(BUILD_DIR)\src

$(BUILD_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.s
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/$(ELF): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@
	$(SIZE) $@

$(BUILD_DIR)/$(HEX): $(BUILD_DIR)/$(ELF)
	$(OBJCOPY) -O ihex $< $@

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean