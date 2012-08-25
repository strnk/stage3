#include <multiboot.hpp>
#include <inttypes.h>
#include <stdio.h>

using namespace Stage3::Multiboot;

uint64_t
Stage3::Multiboot::info_t::find_phys_max()
{
    Multiboot::memory_map_t::iterator it;
    uint64_t ram_size = (mem_lower + mem_upper) << 10;
    uint64_t track = 0;
    
    for (it = mmap_begin(); it != mmap_end(); it++)
    {
        if (it->addr + it->len > ram_size && it->addr != track)
            return track;
            
        track = it->addr + it->len;
    } 
    
    return track;
}

void
Stage3::Multiboot::dump(info_t* mbi)
{
    printf("  flags: 0x%x\n", mbi->flags);
    
    if (mbi->flags & MULTIBOOT_INFO_MEMORY)
    {
        printf("  available memory: lower=%dkB, upper=%dkB\n",
            mbi->mem_lower, mbi->mem_upper);
    }
    
    if (mbi->flags & MULTIBOOT_INFO_BOOTDEV)
    {
        printf("  boot device: 0x%x\n", mbi->boot_device);
    }
    
    if (mbi->flags & MULTIBOOT_INFO_CMDLINE)
    {
        printf("  commandline: %s\n", (char*)(uint64_t) mbi->cmdline);
    }
    
    if (mbi->flags & MULTIBOOT_INFO_MODS)
    {
        unsigned int i;
        module_t *m;
        
        printf("  %d mods loaded at 0x%x :\n", mbi->mods_count, mbi->mods_addr);
        
        for (i = 0, m = (module_t*)(uint64_t)mbi->mods_addr; 
            i < mbi->mods_count; 
            i++, m++)
            printf("    ` 0x%x - 0x%x -- %s\n", m->mod_start, m->mod_end,
                (char*)(uint64_t) m->cmdline);
    }
    
    if (mbi->flags & MULTIBOOT_INFO_ELF_SHDR)
    {
        elf_section_header_table_t *elf_sht = &(mbi->u.elf_sec);
        
        printf("  ELF section header table: num=%d, size=0x%x, addr=0x%x, shndx=0x%x\n",
            elf_sht->num, elf_sht->size, elf_sht->addr, elf_sht->shndx);
    }
    
    if (mbi->flags & MULTIBOOT_INFO_MEM_MAP)
    {
        memory_map_t *mmap;
        
        printf("  Memory map: addr=0x%x, length=0x%x\n", 
            mbi->mmap_addr, mbi->mmap_length);

        for (mmap = (memory_map_t*)(uintptr_t)mbi->mmap_addr;
            (uintptr_t)mmap < mbi->mmap_addr + mbi->mmap_length;
            mmap++)
        {
            printf("    ` 0x%lx - 0x%lx [%s]\n",
                mmap->addr, mmap->addr + mmap->len,
                ((mmap->type == MULTIBOOT_MEMORY_AVAILABLE)
                    ?"available":"reserved")
            );
        } 
    }
}
