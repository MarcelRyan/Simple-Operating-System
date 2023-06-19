#ifndef _GDT_H
#define _GDT_H

#include "lib-header/stdtype.h"

#define GDT_MAX_ENTRY_COUNT 32

#define GDT_USER_CODE_SEGMENT_SELECTOR   0x18
#define GDT_USER_DATA_SEGMENT_SELECTOR   0x20
#define GDT_TSS_SELECTOR                 0x28

extern struct GDTR _gdt_gdtr;

/**
 * Segment Descriptor storing system segment information.
 * Struct defined exactly as Intel Manual Segment Descriptor definition (Figure 3-8 Segment Descriptor).
 * Manual can be downloaded at www.intel.com/content/www/us/en/architecture-and-technology/64-ia-32-architectures-software-developer-vol-3a-part-1-manual.html/ 
 *
 * @param segment_low  16-bit lower-bit segment limit
 * @param base_low     16-bit lower-bit base address
 * @param base_mid     8-bit middle-bit base address
 * @param type_bit     4-bit contain type flags
 * @param non_system   1-bit contain system
 */
struct SegmentDescriptor {
    // First 32-bit
    uint16_t segment_low;
    uint16_t base_low;

    // Next 16-bit (Bit 32 to 47)
    uint8_t base_mid;
    uint8_t type        : 4;
    uint8_t non_system  : 1;
    uint8_t ring        : 2;
    uint8_t present     : 1;

    // Last 16-bit
    uint8_t segment_high:4;
    uint8_t avl         :1; // value: zero
    uint8_t mode64      :1; // value: zero
    uint8_t db_bit      :1; // should always be set to 1 for 32-bit code and data segments
    uint8_t granularity :1;
    uint8_t base_high;
} __attribute__((packed));

/**
 * Global Descriptor Table containing list of segment descriptor. One GDT already defined in memory.c.
 * More details at https://wiki.osdev.org/GDT_Tutorial
 * @param table Fixed-width array of SegmentDescriptor with size GDT_MAX_ENTRY_COUNT
 */
struct GlobalDescriptorTable {
    struct SegmentDescriptor table[GDT_MAX_ENTRY_COUNT];
} __attribute__((packed));

/**
 * GDTR, carrying information where's the GDT located and GDT size.
 * Global kernel variable defined at memory.c.
 * 
 * @param size    Global Descriptor Table size, use sizeof operator
 * @param address GDT address, GDT should already defined properly
 */
struct GDTR {
    uint16_t                     size;
    struct GlobalDescriptorTable *address;
} __attribute__((packed));



// Set GDT_TSS_SELECTOR with proper TSS values, accessing _interrupt_tss_entry
void gdt_install_tss(void);
#endif