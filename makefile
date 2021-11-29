APP     ?= UNSME0
TARGET  ?= native
DEBUG   ?= 1
OPT     ?= -O2

LIBOGC  := $(DEVKITPRO)/libogc
LIBNDS  := $(DEVKITPRO)/libnds
LIBCTRU := $(DEVKITPRO)/libctru
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

FLAG    := -Isrc -Ibuild/$(APP)
FLAG    += -fno-strict-aliasing -Wall -Wextra -Wpedantic $(OPT)
ifneq ($(DEBUG),0)
	FLAG    += -ggdb3 -D__DEBUG__
endif

ifeq ($(TARGET),native)
	CC      := gcc -fno-pie $(FLAG) -D__NATIVE__
	LD      := gcc -no-pie -s
	LIB     := -lm -lSDL2 -lGL
else ifeq ($(TARGET),win32)
	CC      := i686-w64-mingw32-gcc -mwindows $(FLAG) -D__NATIVE__
	LD      := i686-w64-mingw32-gcc -mwindows -s
	LIB     := -lmingw32 -lm -lSDL2main -lSDL2 -lopengl32
else ifeq ($(TARGET),gcn)
	CC      := powerpc-eabi-gcc -mogc -mcpu=750 -meabi -mhard-float
	LD      := $(CC)
	CC      += -ffunction-sections -fwrapv
	CC      += -I$(LIBOGC)/include
	LD      += -L$(LIBOGC)/lib/cube
	CC      += $(FLAG) -DGEKKO -D__GCN__
	LIB     := -lfat -lm -logc
else ifeq ($(TARGET),wii)
	CC      := powerpc-eabi-gcc -mrvl -mcpu=750 -meabi -mhard-float
	LD      := $(CC)
	CC      += -ffunction-sections -fwrapv
	CC      += -I$(LIBOGC)/include
	LD      += -L$(LIBOGC)/lib/wii
	CC      += $(FLAG) -DGEKKO -D__WII__
	LIB     := -lfat -lm -logc
else ifeq ($(TARGET),nds)
	CC      := arm-none-eabi-gcc -march=armv5te -mtune=arm946e-s
	LD      := $(CC) -specs=dsi_arm9.specs
	CC      += -fomit-frame-pointer -ffunction-sections
	CC      += -I$(LIBNDS)/include
	LD      += -L$(LIBNDS)/lib
	CC      += $(FLAG) -DARM9 -D__NDS__
	LIB     := -lfat -lm -lnds9
else ifeq ($(TARGET),3ds)
	CC      := arm-none-eabi-gcc -march=armv6k -mtune=mpcore
	CC      += -mfloat-abi=hard -mtp=soft
	LD      := $(CC) -specs=3dsx.specs
	CC      += -mword-relocations -fomit-frame-pointer -ffunction-sections
	CC      += -I$(LIBCTRU)/include -I$(PICAGL)/include
	LD      += -L$(LIBCTRU)/lib -L$(PICAGL)/lib
	CC      += $(FLAG) -DARM11 -D_3DS -D__3DS__
	LIB     := -lpicaGL -lm -lctru
endif

.PHONY: default native win32 gcn wii nds 3ds
default: $(TARGET)
native: $(BUILD)/app.elf
win32:  $(BUILD)/app.exe
gcn:    $(BUILD)/app.dol
wii:    $(BUILD)/app.dol
nds:    $(BUILD)/app.nds
3ds:    $(BUILD)/app.3dsx

$(BUILD)/%.dol: $(BUILD)/%.elf
	elf2dol $< $@

$(BUILD)/%.nds: $(BUILD)/%.elf
	ndstool -c $@ -9 $<

$(BUILD)/%.3dsx: $(BUILD)/%.elf
	3dsxtool $< $@

$(BUILD)/app.elf $(BUILD)/app.exe: $(SRC_OBJ) $(APP_OBJ)
	$(LD) -Wl,-Map,$(basename $@).map -o $@ $^ $(LIB)

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
