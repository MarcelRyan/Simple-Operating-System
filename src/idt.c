#include "lib-header/idt.h"


struct InterruptDescriptorTable interrupt_descriptor_table = {};
struct IDTR _idt_idtr = {
        sizeof(interrupt_descriptor_table)-1,
        &interrupt_descriptor_table
};

void initialize_idt(void) {
    /* Iterate all isr_stub_table,
    * Set all IDT entry with set_interrupt_gate()
    * with following values:
    * Vector: i
    * Handler Address: isr_stub_table[i]
    * Segment: GDT_KERNEL_CODE_SEGMENT_SELECTOR
    * Privilege: 0 by default. for 0x30 to 0x3F: 3
    */
    for (int i = 0; i < ISR_STUB_TABLE_LIMIT; ++i) {
        int privilege = 0;
        if (i >= 0x30 && i <= 0x3F) {
            privilege = 3;
        }
        set_interrupt_gate(i, isr_stub_table[i], GDT_KERNEL_CODE_SEGMENT_SELECTOR, privilege);
    }
    __asm__ volatile("lidt %0" : : "m"(_idt_idtr));
    __asm__ volatile("sti");
}

void set_interrupt_gate(uint8_t int_vector, void *handler_address,
                        uint16_t gdt_seg_selector, uint8_t privilege) {
    struct IDTGate *idt_int_gate = &interrupt_descriptor_table.table[int_vector];

    idt_int_gate->offset_low = ((uint32_t)handler_address) & 0xFFFF;
    idt_int_gate->offset_high = ((uint32_t)handler_address >> 16) & 0xFFFF;
    idt_int_gate->privilege = privilege;
    idt_int_gate->segment = gdt_seg_selector;
    idt_int_gate->_reserved = 0;
    // Target system 32-bit and flag this as valid interrupt gate
    idt_int_gate->_r_bit_1 = INTERRUPT_GATE_R_BIT_1;
    idt_int_gate->_r_bit_2 = INTERRUPT_GATE_R_BIT_2;
    idt_int_gate->_r_bit_3 = INTERRUPT_GATE_R_BIT_3;
    idt_int_gate->gate_32  = 1;
    idt_int_gate->valid_bit = 1;
}