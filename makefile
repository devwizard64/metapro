APP     ?= UNSME0
TARGET  ?= native

ifeq ($(TARGET),native)
	CC      := gcc
	LD      := $(CC)
	CC      += -ggdb3 -D _NATIVE -D _DEBUG
	LIB     := -l m -l SDL2 -l GL
else ifeq ($(TARGET),win32)
	CC      := i686-w64-mingw32-gcc -mconsole
	LD      := $(CC)
	CC      += -ggdb3 -D _NATIVE -D _DEBUG
	LIB     := -l mingw32 -l m -l SDL2main -l SDL2 -l opengl32
else ifeq ($(TARGET),3ds)
	LIBCTRU := $(DEVKITPRO)/libctru
	PICAGL  := picaGL
	CC      := arm-none-eabi-gcc -march=armv6k -mtune=mpcore
	CC      += -mfloat-abi=hard -mtp=soft -mword-relocations
	CC      += -fomit-frame-pointer -ffunction-sections
	LD      := $(CC) -specs=3dsx.specs
	CC      += -D ARM11 -D _3DS
	CC      += -I $(LIBCTRU)/include -I $(PICAGL)/include
	LD      += -L $(LIBCTRU)/lib -L $(PICAGL)/lib
	LIB     := -l picaGL -l m -l ctru
else ifeq ($(TARGET),gcn)
	LIBOGC  := $(DEVKITPRO)/libogc
	CC      := powerpc-eabi-gcc -mogc -mcpu=750 -meabi -mhard-float
	CC      += -ffunction-sections -fwrapv
	LD      := $(CC)
	CC      += -D GEKKO -D _GCN
	CC      += -I $(LIBOGC)/include
	LD      += -L $(LIBOGC)/lib/cube
	LIB     := -l fat -l m -l ogc
else ifeq ($(TARGET),wii)
	LIBOGC  := $(DEVKITPRO)/libogc
	CC      := powerpc-eabi-gcc -mrvl -mcpu=750 -meabi -mhard-float
	CC      += -ffunction-sections -fwrapv
	LD      := $(CC)
	CC      += -D GEKKO -D _WII
	CC      += -I $(LIBOGC)/include
	LD      += -L $(LIBOGC)/lib/wii
	LIB     := -l fat -l m -l ogc
endif

APP_SRC = build/$(APP)/app
BUILD   = build/$(APP)/$(TARGET)

CC      += -Wall -Wextra -Wpedantic -I src -I $(APP_SRC)
$(BUILD)/src/%.o: CC += -Ofast
$(BUILD)/app/%.o: CC += -Wno-maybe-uninitialized -Wno-uninitialized

SRC_OBJ := \
	$(BUILD)/src/main.o \
	$(BUILD)/src/demo.o \
	$(BUILD)/src/pm.o   \
	$(BUILD)/src/cpu.o  \
	$(BUILD)/src/lib.o  \
	$(BUILD)/src/gsp.o  \
	$(BUILD)/src/asp.o

APP_OBJ := $(shell python3 main.py $(APP) $(BUILD))
APP_LST := $(addprefix $(APP_SRC)/,$(notdir $(APP_OBJ:.o=.c)))

all: $(TARGET)

clean:
	rm -rf build

$(APP_SRC) $(BUILD)/src $(BUILD)/app:
	mkdir -p $@

$(APP_SRC)/app.h: main.py | $(APP_SRC)
	python3 main.py $(APP)

$(APP_LST): $(APP_SRC)/app.h

$(BUILD)/src/%.o: src/%.c $(APP_SRC)/app.h | $(BUILD)/src
	$(CC) -MMD -MF $(@:.o=.d) -c -o $@ $<

$(BUILD)/app/%.o: $(APP_SRC)/%.c | $(BUILD)/app
	$(CC) -MMD -MF $(@:.o=.d) -c -o $@ $<

$(BUILD)/app.elf: $(SRC_OBJ) $(APP_OBJ)
	$(LD) -Wl,-Map,$(@:.elf=.map) -o $@ $^ $(LIB)

$(BUILD)/app.exe: $(SRC_OBJ) $(APP_OBJ)
	$(LD) -Wl,-Map,$(@:.elf=.map) -o $@ $^ $(LIB)

$(BUILD)/%.3dsx: $(BUILD)/%.elf
	3dsxtool $< $@

$(BUILD)/%.dol: $(BUILD)/%.elf
	elf2dol $< $@

native: $(BUILD)/app.elf
win32: $(BUILD)/app.exe
3ds: $(BUILD)/app.3dsx
gcn: $(BUILD)/app.dol
wii: $(BUILD)/app.dol

-include $(SRC_OBJ:.o=.d)
-include $(APP_OBJ:.o=.d)

print-%:
	$(info $* = $(flavor $*): [$($*)]) @true

.PHONY: all clean native win32 3ds gcn
