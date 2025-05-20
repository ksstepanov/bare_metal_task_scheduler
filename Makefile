ifeq ($(OS),Windows_NT)
  ifeq ($(shell uname -s),) # not in a bash-like shell
	CLEANUP = del /F /Q
	MKDIR = mkdir
  else # in a bash-like shell, like msys
	CLEANUP = rm -rf
	MKDIR = mkdir -p
  endif
	TARGET_EXTENSION=.elf
else
	CLEANUP = rm -rf
	MKDIR = mkdir -p
	TARGET_EXTENSION=.elf
endif

# ========================== PATHS declaration: ==================================
PATHB = build/
PATHO = $(PATHB)objs/
PATH_SRC_MAIN=src/
PATH_SRC_PORT=port/
INCLUDE_DIRS=	-Iinclude/\
		-Iport/
		
# ========================== Build files: =========================================
SRC_MAIN = $(wildcard $(PATH_SRC_MAIN)*.c)
SRC_PORT = $(wildcard $(PATH_SRC_PORT)*.c)
OBJS_MAIN = $(patsubst $(PATH_SRC_MAIN)%.c,$(PATHO)$(PATH_SRC_MAIN)%.o,$(SRC_MAIN))
OBJS_PORT_ = $(patsubst $(PATH_SRC_PORT)%.c,$(PATHO)$(PATH_SRC_PORT)%.o,$(SRC_PORT))

# ========================== Target build configuration: ==========================
OPENOCD_SEMIHOSTING=1
DEBUG_ENABLE=1

CC=arm-none-eabi-gcc
LINK=$(CC)
MACH=cortex-m4
ARM_TARGET=-mcpu=$(MACH) -mthumb
CFLAGS= $(ARM_TARGET) $(FLOAT) -std=gnu11 -O0
FLOAT=-mfloat-abi=soft
LDFLAGS=$(ARM_TARGET) $(FLOAT) -T stm32f412_linker_script.ld -Wl,-Map=$(PATHB)scheduler.map
#LDFLAGS+=-nostdlib
# CFLAGS+=-DNOSTD  -g

ifeq ($(OPENOCD_SEMIHOSTING),1)
    # add C stdlib nano with openocd semihosting syscalls
    LDFLAGS+=--specs=rdimon.specs -lc -lrdimon 
    # pass define to C code to initialize semihosting in runtime
    CFLAGS+="-DOPENOCD_SEMIHOSTING_ENABLED"
    # Remove syscalls from build, because 'rdimon' already contains their implementations
    OBJS_PORT=$(filter-out %syscalls.o, $(OBJS_PORT_))
    ifeq ($(DEBUG_ENABLE),1)
        CFLAGS+="-DDEBUG_ON"
    endif
else
    LDFLAGS+=--specs=nano.specs   # add C stdlib nano
    OBJS_PORT=$(OBJS_PORT_)
endif
# -Wl,-Map=$(PATHB)scheduler.map Here '-Wl' specifically tels that next argument is for linker, othervise it is not recognized.


PROG_NAME=scheduler
EXE=$(PROG_NAME)$(TARGET_EXTENSION)


# ========================== Recipes: =========================================
.PHONY: all
.PHONY: clean

all: $(PATHB)$(EXE)

$(PATHB)$(EXE): $(PATHO) $(OBJS_MAIN) $(OBJS_PORT)
	$(info $(OBJS_MAIN))
	$(info $(OBJS_PORT))
	$(LINK) -o $@ $(OBJS_MAIN) $(OBJS_PORT) $(LDFLAGS)
	
$(PATHO)$(PATH_SRC_PORT)%.o:: $(PATH_SRC_PORT)%.c
	$(CC) -c $(CFLAGS) $(INCLUDE_DIRS) $< -o $@

$(PATHO)$(PATH_SRC_MAIN)%.o:: $(PATH_SRC_MAIN)%.c
	$(info $(OBJS_MAIN))
	$(CC) -c $(CFLAGS) $(INCLUDE_DIRS) $< -o $@
	
$(PATHB):
	@$(MKDIR) $(PATHB)
$(PATHO): $(PATHB)
	@$(MKDIR) $(PATHO)
	@$(MKDIR) $(PATHO)$(PATH_SRC_MAIN)
	@$(MKDIR) $(PATHO)$(PATH_SRC_PORT)

clean:
	$(CLEANUP) $(PATHB)

objdump:
	arm-none-eabi-objdump -D $(PATHB)$(EXE) > $(PATHB)$(PROG_NAME).objdump

# ==================== OpenOCD ============================
BOARDS_PATH=$(HOME)/opt/openocd/xpack-openocd-0.12.0-6-linux-x64/xpack-openocd-0.12.0-6/openocd/scripts/board

load:
	openocd_get_boards.sh
	openocd -f $(BOARDS_PATH)/stm32f412g-disco.cfg

