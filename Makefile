# Written by Burton Sumner with help from Tonc Library (https://www.coronac.com/tonc/text/setup.htm)


# If project name not explicitly specified during make command execution, use default name, "main."
ifeq '$(PROJ_NAME)' ''
	PROJ_NAME=$(shell pwd | sed 's-[a-zA-Z/]*/--g')
endif
ifeq '$(IP)' ''
	IP=$(shell ip route show | grep -i default | awk '{print $$3}' )
endif
#-------------------------------Project Files and Dirs-----------------------------------------------------

TARGET=lib$(PROJ_NAME)
ASM=./asm
SRC=./src
IWRAM_SRC=./iwsrc
BIN=./bin
INC=./include
PRIV_INC=./include_priv
REL=./release

ROM_C_OBJS=$(shell find $(SRC) -type f -iname '*.c' | sed 's-\./src-\./bin-g' | sed 's/\.c/\.o/g')
ROM_CXX_OBJS=$(shell find $(SRC) -type f -iname '*.cpp' | sed 's-\./src-\./bin-g' | sed 's/\.cpp/\.o/g')

IWRAM_C_OBJS=$(shell find $(IWRAM_SRC) -type f -iname '*.c' | sed 's-\./iwsrc-\./bin-g' | sed 's/\.c/\.o/g')
IWRAM_CXX_OBJS=$(shell find $(IWRAM_SRC) -type f -iname '*.cpp' | sed 's-\./iwsrc-\./bin-g' | sed 's/\.cpp/\.o/g')


C_OBJS=$(ROM_C_OBJS) $(IWRAM_C_OBJS)
CXX_OBJS=$(ROM_CXX_OBJS) $(IWRAM_CXX_OBJS)

S_OBJS=$(shell find $(ASM) -type f -iname '*.s' | sed 's-\./asm-\./bin-g' | sed 's/\.s/\.o/g')

#-------------------------------Build Definitions----------------------------------------------------------

PREFIX=arm-none-eabi-
CC=$(PREFIX)gcc
CXX=$(PREFIX)g++
LD=$(PREFIX)g++
AS=$(PREFIX)gcc
DB=$(PREFIX)gdb
AR=$(PREFIX)ar
OBJ_CPY=$(PREFIX)objcopy
MGBA="/mnt/c/Program Files/mGBA/mGBA.exe"

ARCH=-mthumb-interwork -mthumb
IARCH=-mthumb-interwork -marm
SPECS=-specs=gba.specs

CFLAGS_BASE=-O2 -Wall -Wextra -fno-strict-aliasing -I$(INC) -I$(PRIV_INC) -DUSE_BIOS_VSYNC

ROM_CFLAGS=$(CFLAGS_BASE) $(ARCH)

IWRAM_CFLAGS=$(CFLAGS_BASE) $(IARCH) -mlong-calls

LDFLAGS=$(ARCH) $(SPECS)

ASFLAGS=-xassembler-with-cpp -I$(INC) -I$(PRIV_INC)



.PHONY: build clean

#-------------------------------Build----------------------------------------------------------------------

.SILENT:


build: clean $(IWRAM_SRC) $(SRC) $(BIN) $(TARGET).a

release_build: clean clean_release $(REL) build
	mv $(BIN)/$(TARGET).a .
	@tar -zcvf $(REL)/$(TARGET)_lib_release.tar.gz ./$(TARGET).a $(INC)
	rm -f $(TARGET).a



$(TARGET).a: $(C_OBJS) $(CXX_OBJS) $(S_OBJS)
	$(AR) -crs $(BIN)/$@ $^

$(IWRAM_SRC) $(SRC) $(ASM) $(BIN) $(REL):
	mkdir -p $@

debug_lib: clean build gbuild

$(GROM_CXX_OBJS): $(BIN)/%.debug.o : $(SRC)/%.cpp
	$(CXX) -c $< $(GROM_CFLAGS) -o $@

$(ROM_CXX_OBJS): $(BIN)/%.o : $(SRC)/%.cpp
	$(CXX) -c $< $(ROM_CFLAGS) -o $@

$(GROM_C_OBJS): $(BIN)/%.debug.o : $(SRC)/%.c
	$(CC) -c $< $(GROM_CFLAGS) -o $@

$(ROM_C_OBJS): $(BIN)/%.o : $(SRC)/%.c
	$(CC) -c $< $(ROM_CFLAGS) -o $@

$(GS_OBJS): $(BIN)/%.debug.o : $(ASM)/%.s
	$(AS) $(GASFLAGS) -c $< -o $@

$(S_OBJS): $(BIN)/%.o : $(ASM)/%.s
	$(AS) $(ASFLAGS) -c $< -o $@

$(GIWRAM_C_OBJS): $(BIN)/%.debug.o : $(IWRAM_SRC)/%.c
	$(CC) -c $< $(GIWRAM_CFLAGS) -o $@

$(IWRAM_C_OBJS): $(BIN)/%.o : $(IWRAM_SRC)/%.c
	$(CC) -c $< $(IWRAM_CFLAGS) -o $@

$(GIWRAM_CXX_OBJS): $(BIN)/%.debug.o : $(IWRAM_SRC)/%.cpp
	$(CXX) -c $< $(GIWRAM_CFLAGS) -o $@

$(IWRAM_CXX_OBJS): $(BIN)/%.o : $(IWRAM_SRC)/%.cpp
	$(CC) -c $< $(IWRAM_CFLAGS) -o $@

#-------------------------------Build----------------------------------------------------------------------
clean: $(BIN)
	@rm -fv $(BIN)/*.elf
	@rm -fv $(BIN)/*.o
	@rm -fv $(BIN)/*.gba
	@rm -fv $(BIN)/*.a

clean_saves: $(BIN)
	@rm -fv $(BIN)/*.sav

clean_release: $(REL)
	@rm -rfv $(REL)/include
	@rm -fv $(REL)/*

