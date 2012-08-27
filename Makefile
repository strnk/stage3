PHYS_KBASE  := 0x0000000000200000
VIRT_KBASE  := 0xFFFFFFFF80000000

CC          := gcc
CFLAGS      := -g -m64 -Wall -Wextra \
               -mno-red-zone -mcmodel=large \
               -nostdlib -nostdinc -ffreestanding \
               -DSTAGE3 -D__PHYS_KBASE=$(PHYS_KBASE) \
               -D__VIRT_KBASE=$(VIRT_KBASE)
               
CXX         := g++
CXXFLAGS    := -g -m64 -Wall -Wextra \
               -mno-red-zone -mcmodel=large \
               -nostdlib -nostdinc -ffreestanding \
               -DSTAGE3 -D__PHYS_KBASE=$(PHYS_KBASE) \
               -D__VIRT_KBASE=$(VIRT_KBASE) \
               -fno-rtti -nostartfiles -fno-exceptions \
               -fno-stack-protector -std=c++0x -fno-builtin \
               -nodefaultlibs

LD          := ld
LDFLAGS     := --defsym PHYS_KBASE=$(PHYS_KBASE) \
                --defsym VIRT_KBASE=$(VIRT_KBASE) \
                --warn-common
                
KERNEL      := stage3
MAP         := stage3.map
BOOT_IMAGE  := boot
BOOT_ISO    := boot.iso

# Directories
ROOT        := $(shell pwd)
SRC         := $(ROOT)/src
INCLUDE     := $(ROOT)/include

 
# File list
INCLUDES    := $(INCLUDE) $(INCLUDE)/libc $(INCLUDE)/libc/c++ $(INCLUDE)/bootstrap 
SOURCES     := 

include make/Makefile.bootstrap
include make/Makefile.core
include make/Makefile.libc

OBJECTS     := $(patsubst %.cpp,%.o, $(patsubst %.S,%.o, $(SOURCES:.c=.o)))
CC_INCLUDES := $(addprefix -I, $(INCLUDES))

# Main target
all: $(BOOT_IMAGE) $(MAP)

runb: $(BOOT_ISO)
	bochs -f ./extra/bochsrc
    
rund: $(KERNEL)
	qemu-system-x86_64 -kernel $(KERNEL) -net none -m 64 -no-reboot -no-shutdown -s -monitor stdio
	
run: $(KERNEL)
	qemu-system-x86_64 -kernel $(KERNEL) -net none -m 64 -no-reboot
	
	
$(BOOT_IMAGE): $(KERNEL)
ifeq ($(wildcard $(BOOT_IMAGE)), )
	./extra/mkboot.sh $(BOOT_IMAGE) $<
else
	@echo [GEN] $(subst $(PWD)/,,$@) from $(subst $(PWD)/,,$<)
	@./extra/mount.sh $@ $<
endif

$(BOOT_ISO): $(KERNEL)
	@echo [GEN] $(subst $(PWD)/,,$@) from $(subst $(PWD)/,,$<)
	@./extra/mkiso.sh $@ $<
    

$(KERNEL): $(OBJECTS) $(SRC)/bootstrap/linker.lds
	@echo [LD] $(subst $(PWD)/,,$@)
	@$(LD) $(LDFLAGS) -T $(SRC)/bootstrap/linker.lds -o $@ $(OBJECTS)

$(MAP): $(KERNEL)
	@echo [MAP] $(subst $(PWD)/,,$@)
	@nm -C $< | cut -d ' ' -f 1,3 | sort > $@

# Create objects from C++ source code
%.o: %.cpp
	@echo [CXX] $(subst $(PWD)/,,$@)
	@$(CXX) $(CC_INCLUDES) -c $< $(CXXFLAGS) -o $@
    
# Create objects from C source code
%.o: %.c
	@echo [CC] $(subst $(PWD)/,,$@)
	@$(CC) $(CC_INCLUDES) -c $< $(CFLAGS) -o $@

# Create objects from assembler (.S) source code
%.o: %.S
	@echo [AS] $(subst $(PWD)/,,$@)
	@$(CC) $(CC_INCLUDES) -c $< $(CFLAGS) -DASM_SOURCE=1 -DASM_FILE=1 -o $@

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
	@echo [RM] $(BOOT_IMAGE) $(BOOT_ISO) $(KERNEL) $(MAP) 
	@rm -f $(BOOT_IMAGE) $(BOOT_ISO) $(KERNEL) $(MAP) 
