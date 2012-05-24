#ifndef _HW_SEGMENTS_H_
#define _HW_SEGMENTS_H_

#define SEGMENTATION_FLAT   1

#define SEGMENTATION_SCHEME SEGMENTATION_FLAT

#if SEGMENTATION_SCHEME == SEGMENTATION_FLAT
# define KSEGMENT_CODE  1
# define KSEGMENT_DATA  2
#else
# error "Unknown segmentation scheme"
#endif

// Table indicator flag of a segment selector
#define T_GDT   0
#define T_LDT   1

#define SEGMENT_SELECTOR(descriptor, ti, privilege) \
    (descriptor << 3) | (ti << 2) | (privilege & 0x03)
    
#endif // _HW_SEGMENTS_H_
