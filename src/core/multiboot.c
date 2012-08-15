#include <multiboot.h>
#include <inttypes.h>
#include <stdio.h>


uint64_t
multiboot_find_phys_max(multiboot_info_t* mbi)
{
    multiboot_memory_map_t *mmap;
    uint64_t ram_size = (mbi->mem_lower + mbi->mem_upper) << 10;
    uint64_t track = 0;
    
    for (mmap = (multiboot_memory_map_t*)(uint64_t)mbi->mmap_addr;
        (uint64_t)mmap < mbi->mmap_addr + mbi->mmap_length;
        mmap++)
    {
        if (mmap->addr + mmap->len > ram_size && mmap->addr != track)
            return track;
            
        track = mmap->addr + mmap->len;
    } 
    
    return track;
}

void
multiboot_dump(multiboot_info_t* mbi)
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
        int i;
        multiboot_module_t *m;
        
        printf("  %d mods loaded at 0x%x :\n", mbi->mods_count, mbi->mods_addr);
        
        for (i = 0, m = (multiboot_module_t*)(uint64_t)mbi->mods_addr; 
            i < mbi->mods_count; 
            i++, m++)
            printf("    ` 0x%x - 0x%x -- %s\n", m->mod_start, m->mod_end,
                (char*)(uint64_t) m->cmdline);
    }
    
    if (mbi->flags & MULTIBOOT_INFO_ELF_SHDR)
    {
        multiboot_elf_section_header_table_t *elf_sht = &(mbi->u.elf_sec);
        
        printf("  ELF section header table: num=%d, size=0x%x, addr=0x%x, shndx=0x%x\n",
            elf_sht->num, elf_sht->size, elf_sht->addr, elf_sht->shndx);
    }
    
    if (mbi->flags & MULTIBOOT_INFO_MEM_MAP)
    {
        multiboot_memory_map_t *mmap;
        
        printf("  Memory map: addr=0x%x, length=0x%x\n", 
            mbi->mmap_addr, mbi->mmap_length);
            
        for (mmap = (multiboot_memory_map_t*)(uint64_t)mbi->mmap_addr;
            (uint64_t)mmap < mbi->mmap_addr + mbi->mmap_length;
            mmap++)
        {
            printf("    ` 0x%x%x - 0x%x%x [%s]\n",
                (unsigned int)(mmap->addr >> 32), 
                (unsigned int)(mmap->addr & 0xFFFFFFFF),
                (unsigned int)(mmap->len >> 32), 
                (unsigned int)(mmap->len & 0xFFFFFFFF),
                ((mmap->type == MULTIBOOT_MEMORY_AVAILABLE)
                    ?"available":"reserved")
            );
        } 
    }
}
