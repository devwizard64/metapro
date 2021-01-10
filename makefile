APP     ?= UNSME0
TARGET  ?= native

LIBCTRU := $(DEVKITPRO)/libctru
LIBOGC  := $(DEVKITPRO)/libogc
PICAGL  := picaGL

BUILD   := build/$(APP)/$(TARGET)

SRC_OBJ := \
	$(BUILD)/src/main.o \
	$(BUILD)/src/demo.o \
	$(BUILD)/src/pm.o   \
	$(BUILD)/src/cpu.o  \
	$(BUILD)/src/lib.o  \
	$(BUILD)/src/gsp.o  \
	$(BUILD)/src/asp.o

APP_OBJ := $(shell python3 main.py $(APP) $(BUILD)/app/)
APP_SRC := $(addprefix build/$(APP)/,$(notdir $(APP_OBJ:.o=.c)))

CCFLAG  := -Wall -Wextra -Wpedantic -ggdb3 -D _DEBUG
LDFLAG  :=
IFLAG   := -I src -I build/$(APP)
LFLAG   :=

ifeq ($(TARGET),$(filter $(TARGET),native win32))
	ifeq ($(TARGET),native)
		CC      := gcc
		LIB     := -l m -l SDL2 -l GL
	else
		CC      := i686-w64-mingw32-gcc -mwindows
		LIB     := -l mingw32 -l m -l SDL2main -l SDL2 -l opengl32
	endif
	LD      := $(CC)
	CCFLAG  += -D _NATIVE
else ifeq ($(TARGET),3ds)
	CC      := arm-none-eabi-gcc -march=armv6k -mtune=mpcore
	CC      += -mfloat-abi=hard -mtp=soft
	LD      := $(CC) -specs=3dsx.specs
	CCFLAG  += -mword-relocations -fomit-frame-pointer -ffunction-sections
	CCFLAG  += -D ARM11 -D _3DS
	IFLAG   += -I $(LIBCTRU)/include -I $(PICAGL)/include
	LFLAG   += -L $(LIBCTRU)/lib -L $(PICAGL)/lib
	LIB     := -l picaGL -l m -l ctru
else ifeq ($(TARGET),$(filter $(TARGET),gcn wii))
	CC      := powerpc-eabi-gcc
	CCFLAG  += -ffunction-sections -fwrapv -D GEKKO
	IFLAG   += -I $(LIBOGC)/include
	LIB     := -l fat -l m -l ogc
	ifeq ($(TARGET),gcn)
		CC      += -mogc
		CCFLAG  += -D _GCN
		LFLAG   += -L $(LIBOGC)/lib/cube
	else
		CC      += -mrvl
		CCFLAG  += -D _WII
		LFLAG   += -L $(LIBOGC)/lib/wii
	endif
	CC      += -mcpu=750 -meabi -mhard-float
	LD      := $(CC)
endif

# $(BUILD)/src/%.o: CCFLAG += -Ofast
$(BUILD)/app/%.o: CCFLAG += -Wno-maybe-uninitialized -Wno-uninitialized

all: $(TARGET)

clean:
	@rm -rf build

print-%:
	$(info $* = $(flavor $*): [$($*)]) @true

$(PICAGL)/lib/libpicaGL.a:
	@make -j1 -C picaGL

build/$(APP)/3ds/app.elf: $(PICAGL)/lib/libpicaGL.a

build/$(APP) $(BUILD)/src $(BUILD)/app:
	@mkdir -p $@

build/$(APP)/app.h: main.py | build/$(APP)
	@python3 main.py $(APP)

$(APP_SRC): build/$(APP)/app.h

$(BUILD)/src/%.o: src/%.c build/$(APP)/app.h | $(BUILD)/src
	@$(CC) $(CCFLAG) $(IFLAG) -MMD -MP -MF $(@:.o=.d) -c -o $@ $<

$(BUILD)/app/%.o: build/$(APP)/%.c | $(BUILD)/app
	@$(CC) $(CCFLAG) $(IFLAG) -MMD -MP -MF $(@:.o=.d) -c -o $@ $<

$(BUILD)/app.elf $(BUILD)/app.exe: $(SRC_OBJ) $(APP_OBJ)
	@$(LD) $(LDFLAG) $(LFLAG) -Wl,-Map,$(@:.elf=.map) -o $@ $^ $(LIB)

$(BUILD)/%.3dsx: $(BUILD)/%.elf
	@3dsxtool $< $@

$(BUILD)/%.dol: $(BUILD)/%.elf
	@elf2dol $< $@

native: $(BUILD)/app.elf
win32: $(BUILD)/app.exe
3ds: $(BUILD)/app.3dsx
gcn: $(BUILD)/app.dol
wii: $(BUILD)/app.dol

-include $(SRC_OBJ:.o=.d)
-include $(APP_OBJ:.o=.d)

.PHONY: all clean native win32 3ds gcn
