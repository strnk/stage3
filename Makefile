CC          := gcc
CFLAGS      := -Wall -nostdlib -nostdinc -ffreestanding -m32 -DSTAGE3 -g
LDFLAGS     := --warn-common -melf_i386 
KERNEL      := stage3
MAP         := stage3.map
BOOT_IMAGE  := boot

# Directories
ROOT        := $(shell pwd)
SRC         := $(ROOT)/src
INCLUDE     := $(ROOT)/include

 
# File list
INCLUDES    := $(INCLUDE) $(INCLUDE)/libc $(INCLUDE)/bootstrap 
SOURCES     := 

include make/Makefile.bootstrap
include make/Makefile.core
include make/Makefile.libc

OBJECTS     := $(patsubst %.S,%.o, $(SOURCES:.c=.o))
CC_INCLUDES := $(addprefix -I, $(INCLUDES))

# Main target
all: $(BOOT_IMAGE) $(MAP)

$(BOOT_IMAGE): $(KERNEL)
ifeq ($(wildcard $(BOOT_IMAGE)), )
	./extra/mkboot.sh $(BOOT_IMAGE) $<
else
	@echo [GEN] $(subst $(PWD)/,,$@) from $(subst $(PWD)/,,$<)
	@./extra/mount.sh $@ $<
endif

$(KERNEL): $(OBJECTS) $(SRC)/bootstrap/linker.lds
	@echo [LD] $(subst $(PWD)/,,$@)
	@$(LD) $(LDFLAGS) -T $(SRC)/bootstrap/linker.lds -o $@ $(OBJECTS)

$(MAP): $(KERNEL)
	@echo [MAP] $(subst $(PWD)/,,$@)
	@nm -C $< | cut -d ' ' -f 1,3 > $@

# Create objects from C source code
%.o: %.c
	@echo [CC] $(subst $(PWD)/,,$@)
	@$(CC) $(CC_INCLUDES) -c $< $(CFLAGS) -o $@

# Create objects from assembler (.S) source code
%.o: %.S
	@echo [AS] $(subst $(PWD)/,,$@)
	@$(CC) $(CC_INCLUDES) -c $< $(CFLAGS) -DASM_SOURCE=1 -o $@

# Clean directory
clean:
	@echo [RM] \*~
	@find . -name '*~' -print | xargs rm -f
	@echo [RM] \*.o
	@find . -name '*.o' -print | xargs rm -f
	
distclean:
	@echo [RM] \*~
	@find . -name '*~' -print | xargs rm -f
	@echo [RM] \*.o
	@find . -name '*.o' -print | xargs rm -f
	rm $(BOOT_IMAGE)
