
TARGET_NAME := gps_time

SOURCES += main.c start.c vendor/system_nrf51.c
OBJECTS += main.o start.o gcc_startup.o vendor/system_nrf51.o
ASM_SOURCES += gcc_startup.s
LDSCRIPT := gcc_nrf51822_gpstime.ld
LDFLAGS := -Wl,--gc-sections -Wl,-T,$(LDSCRIPT) -Wl,-Map,$(TARGET_NAME).map

# Primary output target is elf
TARGETS := $(TARGET_NAME).elf
TARGETS += $(TARGET_NAME).hex

# Include paths
VENDOR_PATH := vendor/
CMSIS_PATH := cmsis/

# Compiler paths
CFLAGS := -Wall -std=gnu99 -mthumb -ggdb -Os -ffunction-sections -fdata-sections
CFLAGS += -Werror -lnosys -lgcc --specs=nano.specs -nostartfiles
CFLAGS += -I$(VENDOR_PATH)
CFLAGS += -I$(CMSIS_PATH)

# Set MCU type
CFLAGS += -DNRF51

define BUILD_HEX_FILE
    echo "Converting $(2) -> $(1)"
    arm-none-eabi-objcopy $(2) -O ihex $(1)
endef

define LINK
	arm-none-eabi-gcc $(CFLAGS) $(LDFLAGS) -o gps_time.elf $^
endef

.PHONY: all
all: $(TARGETS)

clean:
	rm -f $(OBJECTS) $(TARGETS)

gps_time.elf: $(OBJECTS)
	$(call LINK,$@,$^)
	
%.hex: %.elf
	$(call BUILD_HEX_FILE,$@,$<)

%.o: %.c
	arm-none-eabi-gcc $(CFLAGS) -c -o $@ $<

%.o: %.s
	arm-none-eabi-gcc $(CFLAGS) -c -o $@ $<