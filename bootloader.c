#include <ascii.h>
#include <widthspec.h>

void main () {
// still in devlopment !
// more code here in future



  
    asm volatile (
        "mov $0x2, %%ah\n"  
        "mov $0x1, %%al\n"  
        "mov $0x0, %%ch\n" 
        "mov $0x0, %%dh\n"  
        "mov $0x80, %%dl\n" 
        "mov $0x1000, %%bx\n" 
        "int $0x13\n"       
        "mov %0, %%ax\n"    
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"
        "mov $0x1000, %%ax\n" 
        "mov %%ax, %%ip\n"
        "mov $0x8, %%ax\n"    
        "mov %%ax, %%sp\n"
        "movw $0xAA55, 0x7DFE\n" 
        "ljmp $0x0, $0x1000\n"  
        : "g" (0x7E00) 
        : "ax", "bx", "cx", "dx"
    );
    while (1) {
        asm("hlt");
    }
}
