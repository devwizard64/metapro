APP     ?= UNSME0
TARGET  ?= native

LIBCTRU := $(DEVKITPRO)/libctru
LIBOGC  := $(DEVKITPRO)/libogc
PICAGL  := picaGL

BUILD   := build/$(APP)/$(TARGET)

SRC_OBJ := \
	$(BUILD)/src/main.o \
	$(BUILD)/src/demo.o \
	$(BUILD)/src/tm.o   \
	$(BUILD)/src/cpu.o  \
	$(BUILD)/src/lib.o  \
	$(BUILD)/src/gsp.o  \
	$(BUILD)/src/asp.o

APP_OBJ := $(shell python3 main.py $(APP) $(BUILD)/app/)
APP_SRC := $(addprefix build/$(APP)/,$(notdir $(APP_OBJ:.o=.c)))

FLAG    := -I src -I build/$(APP)
FLAG    += -Wall -Wextra -Wpedantic -fno-strict-aliasing -Ofast
FLAG    += -ggdb3 -D __DEBUG__

ifeq ($(TARGET),native)
	CC      := gcc -fno-pie $(FLAG) -D __NATIVE__
	LD      := gcc -no-pie
	LIB     := -l m -l SDL2 -l GL
else ifeq ($(TARGET),win32)
	CC      := i686-w64-mingw32-gcc -mwindows
	LD      := $(CC)
	CC      += $(FLAG) -D __NATIVE__
	LIB     := -l mingw32 -l m -l SDL2main -l SDL2 -l opengl32
else ifeq ($(TARGET),3ds)
	CC      := arm-none-eabi-gcc -march=armv6k -mtune=mpcore
	CC      += -mfloat-abi=hard -mtp=soft
	LD      := $(CC) -specs=3dsx.specs
	CC      += -mword-relocations -fomit-frame-pointer -ffunction-sections
	CC      += -I $(LIBCTRU)/include -I $(PICAGL)/include
	LD      += -L $(LIBCTRU)/lib -L $(PICAGL)/lib
	CC      += $(FLAG) -D ARM11 -D _3DS -D __3DS__
	LIB     := -l picaGL -l m -l ctru
else ifeq ($(TARGET),gcn)
	CC      := powerpc-eabi-gcc -mogc -mcpu=750 -meabi -mhard-float
	LD      := $(CC) -L $(LIBOGC)/lib/cube
	CC      += -ffunction-sections -fwrapv
	CC      += -I $(LIBOGC)/include $(FLAG) -D GEKKO -D __GCN__
	LIB     := -l fat -l m -l ogc
else ifeq ($(TARGET),wii)
	CC      := powerpc-eabi-gcc -mrvl -mcpu=750 -meabi -mhard-float
	LD      := $(CC) -L $(LIBOGC)/lib/wii
	CC      += -ffunction-sections -fwrapv
	CC      += -I $(LIBOGC)/include $(FLAG) -D GEKKO -D __WII__
	LIB     := -l fat -l m -l ogc
endif

.PHONY: default native win32 3ds gcn wii
default: $(TARGET)
native: $(BUILD)/app.elf
win32:  $(BUILD)/app.exe
3ds:    $(BUILD)/app.3dsx
gcn:    $(BUILD)/app.dol
wii:    $(BUILD)/app.dol

$(BUILD)/%.3dsx: $(BUILD)/%.elf
	3dsxtool $< $@

$(BUILD)/%.dol: $(BUILD)/%.elf
	elf2dol $< $@

$(BUILD)/app.elf $(BUILD)/app.exe: $(SRC_OBJ) $(APP_OBJ)
	$(LD) -Wl,-Map,$(@:.elf=.map) -o $@ $^ $(LIB)

-include $(SRC_OBJ:.o=.d)
$(BUILD)/src/%.o: src/%.c build/$(APP)/app.h | $(BUILD)/src
	$(CC) -MMD -MP -c -o $@ $<

-include $(APP_OBJ:.o=.d)
$(BUILD)/app/%.o: build/$(APP)/%.c | $(BUILD)/app
	$(CC) -Wno-maybe-uninitialized -Wno-uninitialized -MMD -MP -c -o $@ $<

$(APP_SRC): build/$(APP)/app.h
build/$(APP)/app.h: main.py | build/$(APP)
	python3 main.py $(APP)

build/$(APP) $(BUILD)/src $(BUILD)/app:
	mkdir -p $@

build/$(APP)/3ds/app.elf: $(PICAGL)/lib/libpicaGL.a
$(PICAGL)/lib/libpicaGL.a:
	make -j1 -C picaGL

.PHONY: clean
clean:
	rm -rf build

print-%:
	$(info $* = $(flavor $*): [$($*)]) @true
