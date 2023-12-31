#include "lib-header/interrupt.h"
#include "lib-header/portio.h"
#include "lib-header/keyboard.h"
#include "lib-header/syscall-helper.h"
#include "lib-header/idt.h"


void syscall(struct CPURegister cpu, __attribute__((unused)) struct InterruptStack info);

void activate_keyboard_interrupt(void) {
    out(PIC1_DATA, PIC_DISABLE_ALL_MASK ^ (1 << IRQ_KEYBOARD));
    out(PIC2_DATA, PIC_DISABLE_ALL_MASK);
}

void io_wait(void) {
    out(0x80, 0);
}
void pic_ack(uint8_t irq) {
    if (irq >= 8)
        out(PIC2_COMMAND, PIC_ACK);
    out(PIC1_COMMAND, PIC_ACK);
}
void pic_remap(void) {
    uint8_t a1, a2;

// Save masks
    a1 = in(PIC1_DATA);
    a2 = in(PIC2_DATA);

// Starts the initialization sequence in cascade mode
    out(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    out(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    out(PIC1_DATA, PIC1_OFFSET); // ICW2: Master PIC vector offset
    io_wait();
    out(PIC2_DATA, PIC2_OFFSET); // ICW2: Slave PIC vector offset
    io_wait();
    out(PIC1_DATA, 0b0100);

// ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
    io_wait();
    out(PIC2_DATA, 0b0010);

// ICW3: tell Slave PIC its cascade identity (00000010)
    io_wait();
    out(PIC1_DATA, ICW4_8086);
    io_wait();
    out(PIC2_DATA, ICW4_8086);
    io_wait();

// Restore masks
    out(PIC1_DATA, a1);
    out(PIC2_DATA, a2);
}

void main_interrupt_handler(__attribute__((unused)) struct CPURegister cpu, uint32_t int_number,
                            __attribute__((unused)) struct InterruptStack info) {
    switch (int_number) {
        case PIC1_OFFSET + IRQ_KEYBOARD:
            keyboard_isr();
            break;
        case 0x30:
            syscall(cpu, info);
            break;
    }
}

struct TSSEntry _interrupt_tss_entry = {
        .ss0 = GDT_KERNEL_DATA_SEGMENT_SELECTOR,
};
void set_tss_kernel_current_stack(void) {
    uint32_t stack_ptr;
    // Reading base stack frame instead esp
    __asm__ volatile ("mov %%ebp, %0": "=r"(stack_ptr) : /* <Empty> */);
    // Add 8 because 4 for ret address and other 4 is for stack_ptr variable
    _interrupt_tss_entry.esp0 = stack_ptr + 8; 
}

/*
 * this method should not be visible to outsiders (used only by interrupt handler).
 * hence we do not define it in the header file.
 * */
void syscall(struct CPURegister cpu, __attribute__((unused)) struct InterruptStack info) {
    switch (cpu.eax) {
        case READ_SYSCALL:
            syscall_read(cpu.ebx, cpu.ecx);
            break;
        case READ_DIR_SYSCALL:
            syscall_read_dir(cpu.ebx, cpu.ecx);
            break;
        case WRITE_SYSCALL:
            syscall_write(cpu.ebx, cpu.ecx);
            break;
        case DELETE_SYSCALL:
            syscall_del(cpu.ebx, cpu.ecx);
            break;
        case MOVE_SYSCALL:
            syscall_move(cpu.ebx, cpu.ecx, cpu.edx);
            break;
        case FGETS_SYSCALL:
            syscall_fgets(cpu.ebx, cpu.ecx);
            break;
        case PUTS_SYSCALL:
            syscall_puts(cpu.ebx, cpu.ecx, cpu.edx);
            break;

     }
}