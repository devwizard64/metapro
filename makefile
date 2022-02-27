APP     ?= UNSME0
TARGET  ?= native
DEBUG   ?= 1
ifeq ($(DEBUG),0)
	LLE ?= 0
	OPT ?= -O2
else
	LLE ?= 1
	OPT ?= -O0
endif

LIBOGC  := $(DEVKITPRO)/libogc
LIBNDS  := $(DEVKITPRO)/libnds
LIBCTRU := $(DEVKITPRO)/libctru
PICAGL  := picaGL

BUILD   := build/$(APP)/$(TARGET)

SRC_OBJ := \
	$(BUILD)/src/main.o \
	$(BUILD)/src/demo.o \
	$(BUILD)/src/tm.o   \
	$(BUILD)/src/sys.o  \
	$(BUILD)/src/mtx.o  \
	$(BUILD)/src/cpu.o  \
	$(BUILD)/src/rsp.o  \
	$(BUILD)/src/gsp.o  \
	$(BUILD)/src/asp.o

LIB_OBJ := \
	$(BUILD)/src/lib/osSpTaskStartGo.o      \
	$(BUILD)/src/lib/osInitialize.o         \
	$(BUILD)/src/lib/osContGetReadData.o    \
	$(BUILD)/src/lib/osContInit.o           \
	$(BUILD)/src/lib/osPiStartDma.o         \
	$(BUILD)/src/lib/guOrtho.o              \
	$(BUILD)/src/lib/guPerspective.o        \
	$(BUILD)/src/lib/osEPiStartDma.o        \
	$(BUILD)/src/lib/osContGetQuery.o

APP_OBJ := $(shell python3 main.py $(APP) $(BUILD)/app/)
APP_SRC := $(addprefix build/$(APP)/,$(notdir $(APP_OBJ:.o=.c)))

FLAG    += -fno-strict-aliasing -Isrc -Ibuild/$(APP) $(OPT)
ifneq ($(DEBUG),0)
	FLAG    += -ggdb3 -DDEBUG
endif
ifneq ($(LLE),0)
	FLAG    += -DLLE
endif

ifeq ($(TARGET),native)
	CC      := gcc
	LD      := gcc
	CCFLAG  := -fno-pie $(FLAG) -D__NATIVE__
	LDFLAG  := -no-pie
	ifeq ($(DEBUG),0)
		LDFLAG  += -s
	endif
	LIB     := -lm -lSDL2 -lGL
else ifeq ($(TARGET),win32)
	CC      := i686-w64-mingw32-gcc
	LD      := i686-w64-mingw32-gcc
	CCFLAG  := -mwindows $(FLAG) -D__NATIVE__
	LDFLAG  := -mwindows
	ifeq ($(DEBUG),0)
		LDFLAG  += -s
	endif
	LIB     := -lmingw32 -lm -lSDL2main -lSDL2 -lopengl32
else ifeq ($(TARGET),gcn)
	CC      := powerpc-eabi-gcc
	LD      := powerpc-eabi-gcc
	CCFLAG  := -mogc -mcpu=750 -meabi -mhard-float
	LDFLAG  := $(CCFLAG)
	CCFLAG  += -ffunction-sections -fwrapv
	CCFLAG  += -I$(LIBOGC)/include
	LDFLAG  += -L$(LIBOGC)/lib/cube
	CCFLAG  += $(FLAG) -DGEKKO -D__GCN__
	LIB     := -lfat -lm -logc
else ifeq ($(TARGET),wii)
	CC      := powerpc-eabi-gcc
	LD      := powerpc-eabi-gcc
	CCFLAG  := -mrvl -mcpu=750 -meabi -mhard-float
	LDFLAG  := $(CCFLAG)
	CCFLAG  += -ffunction-sections -fwrapv
	CCFLAG  += -I$(LIBOGC)/include
	LDFLAG  += -L$(LIBOGC)/lib/wii
	CCFLAG  += $(FLAG) -DGEKKO -D__WII__
	LIB     := -lfat -lm -logc
else ifeq ($(TARGET),nds)
	CC      := arm-none-eabi-gcc
	LD      := arm-none-eabi-gcc
	CCFLAG  := -march=armv5te -mtune=arm946e-s
	LDFLAG  := $(CCFLAG) -specs=dsi_arm9.specs
	CCFLAG  += -fomit-frame-pointer -ffunction-sections
	CCFLAG  += -I$(LIBNDS)/include
	LDFLAG  += -L$(LIBNDS)/lib
	CCFLAG  += $(FLAG) -DARM9 -D__NDS__
	LIB     := -lfat -lm -lnds9
else ifeq ($(TARGET),3ds)
	CC      := arm-none-eabi-gcc
	LD      := arm-none-eabi-gcc
	CCFLAG  := -march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft
	LDFLAG  := $(CCFLAG) -specs=3dsx.specs
	CCFLAG  += -mword-relocations -fomit-frame-pointer -ffunction-sections
	CCFLAG  += -I$(LIBCTRU)/include -I$(PICAGL)/include
	LDFLAG  += -L$(LIBCTRU)/lib -L$(PICAGL)/lib
	CCFLAG  += $(FLAG) -DARM11 -D_3DS -D__3DS__
	LIB     := -lpicaGL -lm -lctru
endif

CCFLAG  += -Wall -Wextra
WFLAG   := -Wno-uninitialized

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

$(BUILD)/app.elf $(BUILD)/app.exe: $(SRC_OBJ) $(LIB_OBJ) $(APP_OBJ)
	$(LD) $(LDFLAG) -Wl,-Map,$(basename $@).map -o $@ $^ $(LIB)

-include $(SRC_OBJ:.o=.d)
-include $(LIB_OBJ:.o=.d)
$(BUILD)/src/%.o: src/%.c build/$(APP)/app.h | $(BUILD)/src $(BUILD)/src/lib
	$(CC) $(CCFLAG) -MMD -MP -c -o $@ $<

-include $(APP_OBJ:.o=.d)
$(BUILD)/app/%.o: build/$(APP)/%.c | $(BUILD)/app
	$(CC) $(CCFLAG) $(WFLAG) -MMD -MP -c -o $@ $<

$(APP_SRC): build/$(APP)/app.h
build/$(APP)/app.h: main.py | build/$(APP)
	python3 main.py $(APP)

build/$(APP) $(BUILD)/src $(BUILD)/src/lib $(BUILD)/app:
	mkdir -p $@

build/$(APP)/3ds/app.elf: $(PICAGL)/lib/libpicaGL.a
$(PICAGL)/lib/libpicaGL.a:
	make -j1 -C picaGL

.PHONY: clean
clean:
	rm -rf build

print-%:
	$(info $* = $(flavor $*): [$($*)]) @true
