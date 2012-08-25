/* multiboot.h - Multiboot header file. */
/* Copyright (C) 1999,2003,2007,2008,2009  Free Software Foundation, Inc.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to
*  deal in the Software without restriction, including without limitation the
*  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
*  sell copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*
*  The above copyright notice and this permission notice shall be included in
*  all copies or substantial portions of the Software.
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL ANY
*  DEVELOPER OR DISTRIBUTOR BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
*  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
*  IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef MULTIBOOT_HEADER
#define MULTIBOOT_HEADER 1

#define MULTIBOOT_SEARCH                        8192

#define MULTIBOOT_HEADER_MAGIC                  0x1BADB002
#define MULTIBOOT_BOOTLOADER_MAGIC              0x2BADB002
#define uNSUPPORTED                   0x0000fffc
#define MULTIBOOT_MOD_ALIGN                     0x00001000
#define MULTIBOOT_INFO_ALIGN                    0x00000004

/* Flags set in the 'flags' member of the multiboot header. */
#define MULTIBOOT_PAGE_ALIGN                    0x00000001
#define MULTIBOOT_MEMORY_INFO                   0x00000002
#define MULTIBOOT_VIDEO_MODE                    0x00000004
#define MULTIBOOT_AOUT_KLUDGE                   0x00010000

/* Flags to be set in the 'flags' member of the multiboot info structure. */
#define MULTIBOOT_INFO_MEMORY                   0x00000001
#define MULTIBOOT_INFO_BOOTDEV                  0x00000002
#define MULTIBOOT_INFO_CMDLINE                  0x00000004
#define MULTIBOOT_INFO_MODS                     0x00000008
#define MULTIBOOT_INFO_AOUT_SYMS                0x00000010
#define MULTIBOOT_INFO_ELF_SHDR                 0X00000020
#define MULTIBOOT_INFO_MEM_MAP                  0x00000040
#define MULTIBOOT_INFO_DRIVE_INFO               0x00000080
#define MULTIBOOT_INFO_CONFIG_TABLE             0x00000100
#define MULTIBOOT_INFO_BOOT_LOADER_NAME         0x00000200
#define MULTIBOOT_INFO_APM_TABLE                0x00000400
#define MULTIBOOT_INFO_VIDEO_INFO               0x00000800

#ifndef ASM_FILE
#include <stdlib.h>
#include <inttypes.h>

namespace Stage3 {
namespace Multiboot {

    struct header
    {
        uint32_t magic;

        /* Feature flags. */
        uint32_t flags;

        /* The above fields plus this one must equal 0 mod 2^32. */
        uint32_t checksum;

        /* These are only valid if MULTIBOOT_AOUT_KLUDGE is set. */
        uint32_t header_addr;
        uint32_t load_addr;
        uint32_t load_end_addr;
        uint32_t bss_end_addr;
        uint32_t entry_addr;

        /* These are only valid if MULTIBOOT_VIDEO_MODE is set. */
        uint32_t mode_type;
        uint32_t width;
        uint32_t height;
        uint32_t depth;
    };

    /* The symbol table for a.out. */
    struct aout_symbol_table
    {
        uint32_t tabsize;
        uint32_t strsize;
        uint32_t addr;
        uint32_t reserved;
    };
    typedef struct aout_symbol_table aout_symbol_table_t;

    /* The section header table for ELF. */
    struct elf_section_header_table
    {
        uint32_t num;
        uint32_t size;
        uint32_t addr;
        uint32_t shndx;
    };
    typedef struct elf_section_header_table elf_section_header_table_t;

    struct mmap_entry
    {
        uint32_t size;
        uint64_t addr;
        uint64_t len;
        
        #define MULTIBOOT_MEMORY_AVAILABLE              1
        #define MULTIBOOT_MEMORY_RESERVED               2
        uint32_t type;
        
        bool isAvailable() { return type == MULTIBOOT_MEMORY_AVAILABLE; }
        
        class iterator
        {
            mmap_entry* p;
            
            public:
            iterator() : p(NULL) { }
            iterator(mmap_entry* ptr) : p(ptr) { }
            iterator(const iterator& it) : p(it.p) { }
            
            iterator& operator++() { ++p; return *this; }
            iterator operator++(int) 
                { iterator tmp(*this); operator++(); return tmp; }
            bool operator==(const iterator& rhs) 
                { return (uintptr_t)p == (uintptr_t)rhs.p; }
            bool operator!=(const iterator& rhs) 
                { return (uintptr_t)p != (uintptr_t)rhs.p; }
            struct mmap_entry& operator*() { return *p; }
            struct mmap_entry* operator->() { return p; }
        };
    } __attribute__((packed));
    typedef struct mmap_entry memory_map_t;

    struct mod_list
    {
        /* the memory used goes from bytes 'mod_start' to 'mod_end-1' inclusive */
        uint32_t mod_start;
        uint32_t mod_end;

        /* Module command line */
        uint32_t cmdline;

        /* padding to take it to 16 bytes (must be zero) */
        uint32_t pad;
    };
    typedef struct mod_list module_t;

    struct info
    {
        /* Multiboot info version number */
        uint32_t flags;

        /* Available memory from BIOS */
        uint32_t mem_lower;
        uint32_t mem_upper;

        /* "root" partition */
        uint32_t boot_device;

        /* Kernel command line */
        uint32_t cmdline;

        /* Boot-Module list */
        uint32_t mods_count;
        uint32_t mods_addr;

        union
        {
            aout_symbol_table_t aout_sym;
            elf_section_header_table_t elf_sec;
        } u;

        /* Memory Mapping buffer */
        uint32_t mmap_length;
        uint32_t mmap_addr;

        /* Drive Info buffer */
        uint32_t drives_length;
        uint32_t drives_addr;

        /* ROM configuration table */
        uint32_t config_table;

        /* Boot Loader Name */
        uint32_t boot_loader_name;

        /* APM table */
        uint32_t apm_table;

        /* Video */
        uint32_t vbe_control_info;
        uint32_t vbe_mode_info;
        uint16_t vbe_mode;
        uint16_t vbe_interface_seg;
        uint16_t vbe_interface_off;
        uint16_t vbe_interface_len;
        
        memory_map_t::iterator mmap_begin() 
            { return memory_map_t::iterator((memory_map_t*)(uintptr_t)mmap_addr); }
        memory_map_t::iterator mmap_end()
            { return memory_map_t::iterator((memory_map_t*)((uintptr_t)mmap_addr
                +(uintptr_t)mmap_length+sizeof(memory_map_t))); 
            }
            
        uint64_t find_phys_max();
    };
    typedef struct info info_t;


    uint64_t
    find_phys_max(info_t* mbi);

    void
    dump(info_t* mbi);
}
}

#endif /* ! ASM_FILE */

#endif /* ! MULTIBOOT_HEADER */
