PREFIX ?= arm-none-eabi
CC = $(PREFIX)-gcc
OBJCOPY = $(PREFIX)-objcopy

ARCH_FLAGS = -mthumb -mcpu=cortex-m3 -msoft-float -mfix-cortex-m3-ldrd

CFLAGS = -g -Wall
CFLAGS += -fno-common -ffunction-sections -fdata-sections
CFLAGS += -IInclude -ILibOS/Include -ILibOS/STM32F103MIDIUSBBoard/Include -ILibOS/STM32F1xxCMSIS/Include -ILibMIDI/Include -ILibOS/STM32f1xxUSB/Include
CFLAGS += -DUSE_HAL_DRIVER -DSTM32F103xB

LDFLAGS = --static -nostartfiles -Tstm32f103x8.ld
LDFLAGS	+= -Wl,-Map=$(*).map
LDFLAGS	+= -Wl,--gc-sections

LDLIBS = -Wl,--start-group -lc -lgcc -lnosys -Wl,--end-group

OBJS = $(patsubst %.c,%.o,$(wildcard LibOS/STM32F103MIDIUSBBoard/Source/*.c))
OBJS += $(patsubst %.c,%.o,$(wildcard LibOS/Source/*.c))
OBJS += $(patsubst %.c,%.o,$(wildcard LibOS/STM32f1xxUSB/Source/*.c))
OBJS += $(patsubst %.c,%.o,$(wildcard LibOS/STM32F1xxCMSIS/Source/*.c))
OBJS += $(patsubst %.c,%.o,$(wildcard LibOS/STM32F1xxCommon/*.c))
OBJS += $(patsubst %.c,%.o,$(wildcard LibMIDI/Source/*.c))
OBJS += LibOS/STM32F1xxCMSIS/Startup/startup_stm32f103xb.o
OBJS += Source/appMain.o

all: midiusb.bin

%.o: %.c
	$(CC) $(CFLAGS) $(ARCH_FLAGS) -o $@ -c $<

%.o: %.s
	$(CC) $(CFLAGS) $(ARCH_FLAGS) -o $@ -c $<

%.elf %.map: $(OBJS)
	$(CC) $(LDFLAGS) $(ARCH_FLAGS) $(OBJS) $(LDLIBS) -o $(*).elf

%.bin: %.elf
	$(OBJCOPY) -Obinary $(*).elf $(*).bin

clean:
	rm -f $(OBJS)

.PHONY: clean
