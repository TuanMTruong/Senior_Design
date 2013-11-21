###############################################################################
# Makefile 
###############################################################################

## General Flags
PROJECT = main
MCU = at90usb1287
TARGET = $(PROJECT).elf
CC = avr-gcc
BATCHISP = "C:/Program Files (x86)/Atmel/Flip 3.4.2/bin/batchisp.exe"

## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU)

## Compile options common for all C compilation units.
CFLAGS = $(COMMON)
CFLAGS += -D AVRGCC -Wall -gdwarf-2  -Os -fsigned-char
CFLAGS += -MD -MP -MT $(*F).o -MF dep/$(@F).d 

## Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += -x assembler-with-cpp -Wa,-gdwarf2

## Linker flags
LDFLAGS = $(COMMON)
##LDFLAGS += -Tat90usbxxx.x
LDFLAGS += -Wl,-Map=$(PROJECT).map,--cref

## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom

HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0


## Include Directories
INCLUDES = -I"./" -I"./conf" -I"../../../../at90usb647" -I"../../../../common" 
##INCLUDES = -I"./.."

## Objects that must be built in order to link
OBJECTS = main.o

## Objects explicitly added by the user
LINKONLYOBJECTS =

## Build
all: $(TARGET) $(PROJECT).hex $(PROJECT).eep size	

## Compile
main.o: main.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<
	
##Link
$(TARGET): $(OBJECTS)
	 $(CC) $(LDFLAGS) $(OBJECTS) $(LINKONLYOBJECTS) $(LIBDIRS) $(LIBS) -o $(TARGET)

%.hex: $(TARGET)
	avr-objcopy -O ihex $(HEX_FLASH_FLAGS)  $< $@

%.eep: $(TARGET)
	avr-objcopy $(HEX_EEPROM_FLAGS) -O ihex $< $@  || exit 0

%.lss: $(TARGET)
	avr-objdump -h -S $< > $@

size: ${TARGET}
	@echo
	@avr-size ${TARGET}

## Program
programlinux: all
	dfu-programmer at90usb646 erase
	dfu-programmer at90usb646 flash "$(PROJECT).hex"
	dfu-programmer at90usb646 start

doxygenlinux:
	doxygen

programmac: all
	dfu-programmer at90usb646 erase
	dfu-programmer at90usb646 flash "$(PROJECT).hex"
	dfu-programmer at90usb646 start
	
doxygenmac:
	doxygen

programwindows: all
	$(BATCHISP) -device ATXMEGA128A1 -hardware RS232 -port COM120 -baudrate 115200 -operation erase F loadbuffer "$(PROJECT).hex" program verify start noreset 0

doxygenwindows:
	doxygen

submission:
	tar -cf submission.tar main.c Makefile
	
## Clean target
.PHONY: clean
clean:
	-rm -rf $(OBJECTS) $(PROJECT).elf dep/* $(PROJECT).hex $(PROJECT).eep $(PROJECT).map
	rm -rf html

## Other dependencies
-include $(shell mkdir dep 2>/dev/null) $(wildcard dep/*)

