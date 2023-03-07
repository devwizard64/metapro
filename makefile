APP     ?= UNSME00
DEBUG   ?= 1
ifneq ($(DEBUG),0)
	LLE ?= 1
	OPT ?= -O0
else
	LLE ?= 0
	OPT ?= -O2
endif

LIBOGC  := $(DEVKITPRO)/libogc
LIBNDS  := $(DEVKITPRO)/libnds
LIBCTRU := $(DEVKITPRO)/libctru
PICAGL  := picaGL

SRC_OBJ := \
	src/main.o  \
	src/demo.o  \
	src/dbtm.o  \
	src/sys.o   \
	src/mtx.o   \
	src/cpu.o   \
	src/rsp.o   \
	src/gsp.o   \
	src/asp.o   \
	src/seq.o

LIB_OBJ := \
	src/lib/osSpTaskStartGo.o   \
	src/lib/osInitialize.o      \
	src/lib/osContGetReadData.o \
	src/lib/osContInit.o        \
	src/lib/osPiStartDma.o      \
	src/lib/guOrtho.o           \
	src/lib/guPerspective.o     \
	src/lib/osEPiStartDma.o     \
	src/lib/osContGetQuery.o

APP_OBJ := $(shell python3 main.py $(APP) app/)

BUILD   := build/$(APP)
FLAG    := -fno-strict-aliasing -Isrc -I$(BUILD) $(OPT)
ifneq ($(DEBUG),0)
	FLAG    += -ggdb3 -DDEBUG
endif
ifneq ($(LLE),0)
	FLAG    += -DLLE
endif
FLAG    += -Wall -Wextra
WARN    := -Wno-uninitialized

CC      := gcc
LD      := gcc
CCFLAG  := -fno-pie $(FLAG) -D__NATIVE__
LDFLAG  := -no-pie

W32_CC      := i686-w64-mingw32-gcc
W32_LD      := i686-w64-mingw32-gcc
W32_ARCH    := -mwindows
W32_CCFLAG  := $(W32_ARCH) $(FLAG) -D__NATIVE__
W32_LDFLAG  := $(W32_ARCH)

OSX_CC      := gcc
OSX_LD      := gcc
OSX_CCFLAG  := -F$(HOME)/Library/Frameworks $(FLAG) -D__NATIVE__
OSX_LDFLAG  := -F$(HOME)/Library/Frameworks

DOL_CC      := powerpc-eabi-gcc
DOL_LD      := powerpc-eabi-gcc
DOL_ARCH    := -mogc -mcpu=750 -meabi -mhard-float
DOL_CCFLAG  := $(DOL_ARCH) -ffunction-sections -fwrapv
DOL_CCFLAG  += -I$(LIBOGC)/include $(FLAG) -DGEKKO -D__GCN__
DOL_LDFLAG  := $(DOL_ARCH) -L$(LIBOGC)/lib/cube

RVL_CC      := powerpc-eabi-gcc
RVL_LD      := powerpc-eabi-gcc
RVL_ARCH    := -mrvl -mcpu=750 -meabi -mhard-float
RVL_CCFLAG  := $(RVL_ARCH) -ffunction-sections -fwrapv
RVL_CCFLAG  += -I$(LIBOGC)/include $(FLAG) -DGEKKO -D__WII__
RVL_LDFLAG  := $(RVL_ARCH) -L$(LIBOGC)/lib/wii

NTR_CC      := arm-none-eabi-gcc
NTR_LD      := arm-none-eabi-gcc
NTR_ARCH    := -march=armv5te -mtune=arm946e-s
NTR_CCFLAG  := $(NTR_ARCH) -fomit-frame-pointer -ffunction-sections
NTR_CCFLAG  += -I$(LIBNDS)/include $(FLAG) -DARM9 -D__NDS__
NTR_LDFLAG  := $(NTR_ARCH) -specs=dsi_arm9.specs -L$(LIBNDS)/lib

CTR_CC      := arm-none-eabi-gcc
CTR_LD      := arm-none-eabi-gcc
CTR_ARCH    := -march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft
CTR_CCFLAG  := $(CTR_ARCH) -mword-relocations -fomit-frame-pointer -ffunction-sections
CTR_CCFLAG  += -I$(LIBCTRU)/include -I$(PICAGL)/include $(FLAG) -DARM11 -D_3DS -D__3DS__
CTR_LDFLAG  := $(CTR_ARCH) -specs=3dsx.specs -L$(LIBCTRU)/lib -L$(PICAGL)/lib

ifeq ($(DEBUG),0)
	LDFLAG  += -s
	W32_LDFLAG  += -s
endif

OBJ     := $(addprefix $(BUILD)/native/,$(SRC_OBJ) $(LIB_OBJ) $(APP_OBJ))
W32_OBJ := $(addprefix $(BUILD)/win32/,$(SRC_OBJ) $(LIB_OBJ) $(APP_OBJ))
OSX_OBJ := $(addprefix $(BUILD)/osx/,$(SRC_OBJ) $(LIB_OBJ) $(APP_OBJ))
DOL_OBJ := $(addprefix $(BUILD)/gcn/,$(SRC_OBJ) $(LIB_OBJ) $(APP_OBJ))
RVL_OBJ := $(addprefix $(BUILD)/wii/,$(SRC_OBJ) $(LIB_OBJ) $(APP_OBJ))
NTR_OBJ := $(addprefix $(BUILD)/nds/,$(SRC_OBJ) $(LIB_OBJ) $(APP_OBJ))
CTR_OBJ := $(addprefix $(BUILD)/3ds/,$(SRC_OBJ) $(LIB_OBJ) $(APP_OBJ))

.PHONY: native win32 osx gcn wii nds 3ds
native: $(BUILD)/native/app.elf
win32:  $(BUILD)/win32/app.exe
osx:    $(BUILD)/osx/app.elf
gcn:    $(BUILD)/gcn/app.dol
wii:    $(BUILD)/wii/app.dol
nds:    $(BUILD)/nds/app.nds
3ds:    $(BUILD)/3ds/app.3dsx

.PHONY: clean
clean:
	rm -f -r build

$(BUILD)/native/app.elf: $(OBJ)
	$(LD) $(LDFLAG) -Wl,-Map,$(@:.elf=.map) -o $@ $(OBJ) -lm -lSDL2 -lGL
$(BUILD)/native/src/%.o: src/%.c $(BUILD)/app.h
	@mkdir -p $(dir $@)
	$(CC) $(CCFLAG) -MMD -MP -c -o $@ $<
$(BUILD)/native/app/%.o: $(BUILD)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CCFLAG) $(WARN) -MMD -MP -c -o $@ $<
-include $(OBJ:.o=.d)

$(BUILD)/win32/app.exe: $(W32_OBJ)
	$(W32_LD) $(W32_LDFLAG) -Wl,-Map,$(@:.exe=.map) -o $@ $(W32_OBJ) -lmingw32 -lm -lSDL2main -lSDL2 -lopengl32
$(BUILD)/win32/src/%.o: src/%.c $(BUILD)/app.h
	@mkdir -p $(dir $@)
	$(W32_CC) $(W32_CCFLAG) -MMD -MP -c -o $@ $<
$(BUILD)/win32/app/%.o: $(BUILD)/%.c
	@mkdir -p $(dir $@)
	$(W32_CC) $(W32_CCFLAG) $(WARN) -MMD -MP -c -o $@ $<
-include $(W32_OBJ:.o=.d)

$(BUILD)/osx/app.elf: $(OSX_OBJ)
	$(OSX_LD) $(OSX_LDFLAG) -o $@ $(OSX_OBJ) -lm -framework SDL2 -framework OpenGL
	install_name_tool -add_rpath $(HOME)/Library/Frameworks $@
$(BUILD)/osx/src/%.o: src/%.c $(BUILD)/app.h
	@mkdir -p $(dir $@)
	$(OSX_CC) $(OSX_CCFLAG) -MMD -MP -c -o $@ $<
$(BUILD)/osx/app/%.o: $(BUILD)/%.c
	@mkdir -p $(dir $@)
	$(OSX_CC) $(OSX_CCFLAG) $(WARN) -MMD -MP -c -o $@ $<
-include $(OSX_OBJ:.o=.d)

$(BUILD)/gcn/app.dol: $(DOL_OBJ)
	$(DOL_LD) $(DOL_LDFLAG) -Wl,-Map,$(@:.dol=.map) -o $(@:.dol=.elf) $(DOL_OBJ) -lfat -lm -logc
	elf2dol $(@:.dol=.elf) $@
$(BUILD)/gcn/src/%.o: src/%.c $(BUILD)/app.h
	@mkdir -p $(dir $@)
	$(DOL_CC) $(DOL_CCFLAG) -MMD -MP -c -o $@ $<
$(BUILD)/gcn/app/%.o: $(BUILD)/%.c
	@mkdir -p $(dir $@)
	$(DOL_CC) $(DOL_CCFLAG) $(WARN) -MMD -MP -c -o $@ $<
-include $(DOL_OBJ:.o=.d)

$(BUILD)/wii/app.dol: $(RVL_OBJ)
	$(RVL_LD) $(RVL_LDFLAG) -Wl,-Map,$(@:.dol=.map) -o $(@:.dol=.elf) $(RVL_OBJ) -lfat -lm -logc
	elf2dol $(@:.dol=.elf) $@
$(BUILD)/wii/src/%.o: src/%.c $(BUILD)/app.h
	@mkdir -p $(dir $@)
	$(RVL_CC) $(RVL_CCFLAG) -MMD -MP -c -o $@ $<
$(BUILD)/wii/app/%.o: $(BUILD)/%.c
	@mkdir -p $(dir $@)
	$(RVL_CC) $(RVL_CCFLAG) $(WARN) -MMD -MP -c -o $@ $<
-include $(RVL_OBJ:.o=.d)

$(BUILD)/nds/app.nds: $(NTR_OBJ)
	$(NTR_LD) $(NTR_LDFLAG) -Wl,-Map,$(@:.nds=.map) -o $(@:.nds=.elf) $(NTR_OBJ) -lfat -lm -lnds9
	ndstool -c $@ -9 $(@:.nds=.elf)
$(BUILD)/nds/src/%.o: src/%.c $(BUILD)/app.h
	@mkdir -p $(dir $@)
	$(NTR_CC) $(NTR_CCFLAG) -MMD -MP -c -o $@ $<
$(BUILD)/nds/app/%.o: $(BUILD)/%.c
	@mkdir -p $(dir $@)
	$(NTR_CC) $(NTR_CCFLAG) $(WARN) -MMD -MP -c -o $@ $<
-include $(NTR_OBJ:.o=.d)

$(BUILD)/3ds/app.3dsx: $(CTR_OBJ) $(PICAGL)/lib/libpicaGL.a
	$(CTR_LD) $(CTR_LDFLAG) -Wl,-Map,$(@:.3dsx=.map) -o $(@:.3dsx=.elf) $(CTR_OBJ) -lpicaGL -lm -lctru
	3dsxtool $(@:.3dsx=.elf) $@
$(BUILD)/3ds/src/%.o: src/%.c $(BUILD)/app.h
	@mkdir -p $(dir $@)
	$(CTR_CC) $(CTR_CCFLAG) -MMD -MP -c -o $@ $<
$(BUILD)/3ds/app/%.o: $(BUILD)/%.c
	@mkdir -p $(dir $@)
	$(CTR_CC) $(CTR_CCFLAG) $(WARN) -MMD -MP -c -o $@ $<
-include $(CTR_OBJ:.o=.d)

$(addprefix $(BUILD)/,$(notdir $(APP_OBJ:.o=.c))): $(BUILD)/app.h
$(BUILD)/app.h: main.py
	@mkdir -p $(dir $@)
	python3 main.py $(APP)

$(PICAGL)/lib/libpicaGL.a:
	make -j1 -C picaGL
