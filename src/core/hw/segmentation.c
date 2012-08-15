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
        .dc = GDTE_DC_CODE_DPL_STRICT,                          \
        .execute = exec,                                        \
        .type = GDTE_TYPE_CODE_DATA,                            \
        .dpl = privilege,                                       \
        .present = GDTE_PRESENT,                                \
        .avl = 0,                                               \
        .longmode = 0,                                          \
        .op_size = GDTE_SIZE_32_BITS,                           \
        .granularity = g                                        \
    })
    

#define GDT64_ENTRY(privilege, exec)                            \
    ((struct gdt_entry) {                                       \
        .size_lo = 0,                                           \
        .size_hi = 0,                                           \
        .base_address_lo = 0,                                   \
        .base_address_mid = 0,                                  \
        .base_address_hi = 0,                                   \
        .accessed = 0,                                          \
        .rw = 0,                                                \
        .dc = GDTE_DC_CODE_DPL_STRICT,                          \
        .execute = exec,                                        \
        .type = GDTE_TYPE_CODE_DATA,                            \
        .dpl = privilege,                                       \
        .present = GDTE_PRESENT,                                \
        .avl = 0,                                               \
        .longmode = 1,                                          \
        .op_size = GDTE_SIZE_32_BITS,                           \
        .granularity = 0                                        \
    })
    
static struct gdt64_register GDTR __align(0x08);
static struct gdt_entry GDT64[] __align(0x08) = 
    {
        [0]             = (struct gdt_entry) {0, },
        [KSEGMENT_CODE] = GDT64_ENTRY(0, GDTE_CODE_SEGMENT),
        [KSEGMENT_DATA] = GDT64_ENTRY(0, GDTE_DATA_SEGMENT)
    };
    
    
void
gdt_init(void)
{
    GDTR.base_address = (uint64_t)GDT64;
    GDTR.size = sizeof(GDT64) - 1;
    
    // Load the GDT
    asm volatile("lgdt %0"  : : "m"(GDTR) : "memory");
}
