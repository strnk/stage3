#include <kernel/hw/segmentation.h>
#include <kernel/hw/segments.h>

#define GDT_ENTRY(base, size, privilege, exec, segrw, g)        \
    ((struct gdt_entry) {                                       \
        .size_lo = (size & 0xFFFF),                             \
        .size_hi = (size & 0xF0000) >> 16,                      \
        .base_address_lo = (base & 0xFFFF),                     \
        .base_address_mid = (base & 0xFF0000) >> 16,            \
        .base_address_hi = (base & 0xFF000000) >> 24,           \
        .accessed = 0,                                          \
        .rw = segrw,                                            \
        .dc = 0,                                                \
        .execute = exec,                                        \
        .type = GDTE_TYPE_CODE_DATA,                            \
        .dpl = privilege,                                       \
        .present = GDTE_PRESENT,                                \
        .zero = 0x00,                                           \
        .op_size = GDTE_SIZE_32_BITS,                           \
        .granularity = g                                        \
    })
    
static struct gdt_register GDTR __align(0x08);
static struct gdt_entry GDT[] __align(0x08) = 
    {
        [0]             = (struct gdt_entry) {0, },
        [KSEGMENT_CODE] = GDT_ENTRY(0x00000000, 0xFFFFF, 0, GDTE_CODE_SEGMENT, 
                            GDTE_RW_CODE_READ, GDTE_GRANULARITY_PAGE),
        [KSEGMENT_DATA] = GDT_ENTRY(0x00000000, 0xFFFFF, 0, GDTE_DATA_SEGMENT, 
                            GDTE_RW_DATA_WRITE, GDTE_GRANULARITY_PAGE)
    };
    
    
void
gdt_init(void)
{
    GDTR.base_address = (uint32_t)GDT;
    GDTR.size = sizeof(GDT) - 1;
    
    // Load the GDT
    asm volatile("lgdt %0"  : : "m"(GDTR) : "memory");
    
    // Segment register update and GDT commit
    asm volatile("ljmp %0,$1f     \n\
                    1:              \n\
                    movw %1, %%ax   \n\
                    movw %%ax, %%ss \n\
                    movw %%ax, %%ds \n\
                    movw %%ax, %%es \n\
                    movw %%ax, %%fs \n\
                    movw %%ax, %%gs"
                    :
                    : "i"(SEGMENT_SELECTOR(KSEGMENT_CODE, T_GDT, 0)),
                      "i"(SEGMENT_SELECTOR(KSEGMENT_DATA, T_GDT, 0))
                    : "memory", "eax");
}
