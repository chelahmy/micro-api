export PATH := $(HOME)/esp-open-sdk/xtensa-lx106-elf/bin:$(PATH)
SDK_BIN = $(HOME)/esp-open-sdk/sdk/bin
CC = xtensa-lx106-elf-gcc
CFLAGS = -I. -DICACHE_FLASH -mlongcalls -fdata-sections -ffunction-sections -I../libesphttpd/include
LDLIBS = -nostdlib -Wl,--gc-sections -Wl,--wrap,strcasecmp \
	-Wl,--start-group -lmain -lnet80211 -lwpa -llwip -lpp -lphy -lc -Wl,--end-group \
	-lgcc ../libesphttpd/libesphttpd.a
LDFLAGS = -Teagle.app.v6.ld 

micro-api-0x00000.bin: micro-api
	esptool.py elf2image $^

micro-api: micro-api.o session.o api.o

micro-api.o: micro-api.c
session.o: session.c session.h
api.o: api.c api.h

flash: micro-api-0x00000.bin
	esptool.py write_flash 0 micro-api-0x00000.bin 0x10000 micro-api-0x10000.bin

erase:	
	esptool.py erase_flash
	
init:	
	esptool.py write_flash 0x3FC000 $(SDK_BIN)/esp_init_data_default.bin

clean:
	rm -f micro-api *.o micro-api-0x00000.bin micro-api-0x10000.bin
	
connect:
	picocom -b 115200 --omap crcrlf /dev/ttyUSB0
