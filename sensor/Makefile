include Makefile.config

C_FLAGS=\
		  -g -Os -w -ffunction-sections -fdata-sections \
		  -MMD -mmcu=atmega328p \
		  -DF_CPU=16000000L -DARDUINO=10603 -DARDUINO_AVR_UNO \
		  -DARDUINO_ARCH_AVR \
		  -I${ARDUINO_AVR_DIR}/cores/arduino \
		  -I${ARDUINO_AVR_DIR}/variants/standard
CXX_FLAGS=\
	      -fno-exceptions -fno-threadsafe-statics
MKDIR_P=mkdir -p

obj/%.o:	${ARDUINO_AVR_DIR}/cores/arduino/%.c
	avr-gcc -c ${C_FLAGS} -o $@ $<

obj/%.o:	${ARDUINO_AVR_DIR}/cores/arduino/%.cpp
	avr-g++ -c ${C_FLAGS} ${CXX_FLAGS} -o $@ $<

obj/%.o:	%.cpp
	avr-g++ -c ${C_FLAGS} ${CXX_FLAGS} -o $@ $<

ARDUINO_CORE_DIR=${ARDUINO_AVR_DIR}/cores/arduino

core_objects := \
	$(patsubst ${ARDUINO_CORE_DIR}/%.c,obj/%.o,$(wildcard ${ARDUINO_CORE_DIR}/*.c)) \
	$(patsubst ${ARDUINO_CORE_DIR}/%.cpp,obj/%.o,$(wildcard ${ARDUINO_CORE_DIR}/*.cpp))

project_objects := $(patsubst %.cpp,obj/%.o,${PROJECT_SRC})

objects := \
	${core_objects} \
	${project_objects}

all:	obj sketch.eep sketch.hex

obj:
	${MKDIR_P} obj

core.a:	${core_objects}
	avr-ar -rc core.a ${core_objects}

sketch.elf:	core.a ${project_objects}
	avr-gcc -w -Os -Wl,--gc-sections -mmcu=atmega328p -o sketch.elf ${project_objects} core.a

sketch.eep:	sketch.elf
	avr-objcopy -O ihex -j .eeprom --set-section-flags=.eeprom=alloc,load --no-change-warnings --change-section-lma .eeprom=0 sketch.elf sketch.eep

sketch.hex:	sketch.elf
	avr-objcopy -O ihex -R .eeprom sketch.elf sketch.hex

.PHONY:	upload

upload:	sketch.hex
	avrdude -v -patmega328p -carduino -P${DEVICE} -b115200 -D -Uflash:w:sketch.hex

.PHONY:	monitor

monitor:
	cat ${DEVICE}

.PHONY:	clean

clean:
	rm -f core.a obj/*.o sketch.elf sketch.eep sketch.hex

