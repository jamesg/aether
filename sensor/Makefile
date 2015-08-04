#
# Makefile for compiling and uploading Arduino sketches.
#
# You will need to have the gcc-avr and avrdude packages for your distribution
# installed.  List the source files to be compiled for the Arduino (you should
# define setup() and loop() as for any Arduino sketch).  Copy Arduino libraries
# into the same directory as the source files and add their .cpp source files
# to the list; this makefile doesn't find libraries from your Arduino
# installation.
#
# Run 'make' to compile your sketch and the libraries.  If everything compiled,
# run 'make upload' to copy the sketch to the microcontroller.
#

# Uncomment these variable lines or copy them into 'Makefile.config' and
# uncomment the line 'include Makefile.config'.

#
## Set ARDUINO_DIR to the location of the Arduino IDE.
#ARDUINO_DIR=/home/james/programs/arduino-1.6.3
#
## ARDUINO_AVR_DIR contains the Arduino C and C++ framework.
#ARDUINO_AVR_DIR=${ARDUINO_DIR}/hardware/arduino/avr
#
## Set PROJECT_SRC to a list of C++ files to build as part of this project.
## ArduinoJson.cpp includes the library .cpp files from ArduinoJson/ .
#PROJECT_SRC=sketch.cpp ArduinoJson.cpp LiquidCrystal.cpp OneWire.cpp
#
## Set DEVICE to the ttyUSB device representing the Arduino Uno board.
#DEVICE=/dev/ttyUSB0
#
## Type of Arduino board: DUE for Arduino Duemilanove or UNO for Arduino Uno.
#TARGET=UNO
#
#C_FLAGS+=\
#	-Iinclude
#
#

include Makefile.config

ifeq (${TARGET},DUE)
	PROGRAMMER=arduino
	PROGRAM_BAUDRATE=57600
	TARGET_C_FLAGS=\
		-DARDUINO=105 \
		-DF_CPU=16000000L
endif

ifeq (${TARGET},UNO)
	PROGRAMMER=arduino
	PROGRAM_BAUDRATE=115200
	TARGET_C_FLAGS=\
		-DARDUINO=10603 -DARDUINO_AVR_UNO \
		-DF_CPU=16000000L \
		-DARDUINO_ARCH_AVR
endif

ifeq (${TARGET},BARE)
	PROGRAMMER=stk500v1
	PROGRAM_BAUDRATE=19200
	TARGET_C_FLAGS=\
		-DARDUINO=10603 -DARDUINO_AVR_UNO \
		-DF_CPU=1000000L \
		-DARDUINO_ARCH_AVR
endif


C_FLAGS+=\
		-g -Os -w -ffunction-sections -fdata-sections \
		-MMD -mmcu=atmega328p \
		-DUSB_VID=null -DUSB_PID=null \
		${TARGET_C_FLAGS} \
		-D__PROG_TYPES_COMPAT__ \
		-I${ARDUINO_AVR_DIR}/cores/arduino \
		-I${ARDUINO_AVR_DIR}/variants/standard

CXX_FLAGS+=\
	      -std=c++11 -fno-exceptions -fno-threadsafe-statics

%.o:	${ARDUINO_AVR_DIR}/cores/arduino/%.c
	avr-gcc -c ${C_FLAGS} -o $@ $<

%.o:	%.c
	avr-gcc -c ${C_FLAGS} -o $@ $<

%.o:	${ARDUINO_AVR_DIR}/cores/arduino/%.cpp
	avr-g++ -c ${C_FLAGS} ${CXX_FLAGS} -o $@ $<

%.o:	%.cpp
	avr-g++ -c ${C_FLAGS} ${CXX_FLAGS} -o $@ $<

ARDUINO_CORE_DIR=${ARDUINO_AVR_DIR}/cores/arduino

core_objects := \
	$(patsubst ${ARDUINO_CORE_DIR}/%.c,%.o,$(wildcard ${ARDUINO_CORE_DIR}/*.c)) \
	$(patsubst ${ARDUINO_CORE_DIR}/%.cpp,%.o,$(wildcard ${ARDUINO_CORE_DIR}/*.cpp))

project_objects := $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,${PROJECT_SRC}))

objects := \
	${core_objects} \
	${project_objects}

dependencies := $(patsubst %.o,%.d,${objects})

all:	sketch.eep sketch.hex

core.a:	${core_objects}
	avr-ar -rc core.a ${core_objects}

sketch.elf:	core.a ${project_objects}
	avr-gcc -w -Os -Wl,--gc-sections -mmcu=atmega328p -o sketch.elf ${project_objects} core.a -lm

sketch.eep:	sketch.elf
	avr-objcopy -O ihex -j .eeprom --set-section-flags=.eeprom=alloc,load --no-change-warnings --change-section-lma .eeprom=0 sketch.elf sketch.eep

sketch.hex:	sketch.elf
	avr-objcopy -O ihex -R .eeprom sketch.elf sketch.hex

.PHONY:	upload

# Upload the sketch using the Arduino-as-ISP interface.
upload:	sketch.hex
	avrdude -v -patmega328p -c${PROGRAMMER} -P${DEVICE} -b${PROGRAM_BAUDRATE} \
		-D -Uflash:w:sketch.hex:i -F

.PHONY:	setfuses

# Set the fuses to the default values, except the clock, which uses an external
# crystal and the /8 divider.
setfuses:
	avrdude -v -patmega328p -cstk500v1 -P${DEVICE} -b${PROGRAM_BAUDRATE} -D \
		-U lfuse:w:0x7f:m -U hfuse:w:0xd9:m -U efuse:w:0xff:m -F

.PHONY:	bootloader

# Burn the Arduino bootloader to the chip.  Should not be used under normal
# circumstances.
bootloader:
	avrdude -patmega328p -cstk500v1 -P${DEVICE} -b${PROGRAM_BAUDRATE} -D -e -F \
		-U flash:w:optiboot_atmega328_96_1.hex

.PHONY:	monitor

monitor:
	cat ${DEVICE}

.PHONY:	clean

clean:
	rm -f core.a ${dependencies} ${objects} sketch.elf sketch.eep sketch.hex
