#define GDTBASE    0x00000800
#define IDTBASE    0x00000000
#define IDTSIZE    0xFF
#define KERNAL     0x00000000

idtr kidtr;
// kernal 
// written in C++
// bootloader was written in C


// kernel.cpp
struct multiboot_info {
    u32 flags;
    u32 low_mem;
    u32 high_mem;
    u32 boot_device;
    u32 cmdline;
    u32 mods_count;
    u32 mods_addr;
    struct {
        u32 num;
        u32 size;
        u32 addr;
        u32 shndx;
    } elf_sec;
    unsigned long mmap_length;
    unsigned long mmap_addr;
    unsigned long drives_length;
    unsigned long drives_addr;
    unsigned long config_table;
    unsigned long boot_loader_name;
    unsigned long apm_table;
    unsigned long vbe_control_info;
    unsigned long vbe_mode_info;
    unsigned long vbe_mode;
    unsigned long vbe_interface_seg;
    unsigned long vbe_interface_off;
    unsigned long vbe_interface_len;
};

void     itoa(char *buf, unsigned long int n, int base);

void *    memset(char *dst,char src, int n);
void *    memcpy(char *dst, char *src, int n);

int     strlen(char *s);
int     strcmp(const char *dst, char *src);
int     strcpy(char *dst,const char *src);
void     strcat(void *dest,const void *src);
char *    strncpy(char *destString, const char *sourceString,int maxLength);
int     strncmp( const char* s1, const char* s2, int c );

typedef unsigned char     u8;
typedef unsigned short     u16;
typedef unsigned int     u32;
typedef unsigned long long     u64;

typedef signed char     s8;
typedef signed short     s16;
typedef signed int         s32;
typedef signed long long    s64;

struct gdtr {
    u16 limite;
    u32 base;
} __attribute__ ((packed));

struct gdtdesc {
    u16 lim0_15;
    u16 base0_15;
    u8 base16_23;
    u8 acces;
    u8 lim16_19:4;
    u8 other:4;
    u8 base24_31;
} __attribute__ ((packed));

void init_gdt_desc(u32 base, u32 limite, u8 acces, u8 other, struct gdtdesc *desc)
{
    desc->lim0_15 = (limite & 0xffff);
    desc->base0_15 = (base & 0xffff);
    desc->base16_23 = (base & 0xff0000) >> 16;
    desc->acces = acces;
    desc->lim16_19 = (limite & 0xf0000) >> 16;
    desc->other = (other & 0xf);
    desc->base24_31 = (base & 0xff000000) >> 24;
    return;
}


void init_gdt(void)
{
    default_tss.debug_flag = 0x00;
    default_tss.io_map = 0x00;
    default_tss.esp0 = 0x1FFF0;
    default_tss.ss0 = 0x18;

    /* initialize gdt segments */
    init_gdt_desc(0x0, 0x0, 0x0, 0x0, &kgdt[0]);
    init_gdt_desc(0x0, 0xFFFFF, 0x9B, 0x0D, &kgdt[1]);    /* code */
    init_gdt_desc(0x0, 0xFFFFF, 0x93, 0x0D, &kgdt[2]);    /* data */
    init_gdt_desc(0x0, 0x0, 0x97, 0x0D, &kgdt[3]);        /* stack */

    init_gdt_desc(0x0, 0xFFFFF, 0xFF, 0x0D, &kgdt[4]);    /* ucode */
    init_gdt_desc(0x0, 0xFFFFF, 0xF3, 0x0D, &kgdt[5]);    /* udata */
    init_gdt_desc(0x0, 0x0, 0xF7, 0x0D, &kgdt[6]);        /* ustack */

    init_gdt_desc((u32) & default_tss, 0x67, 0xE9, 0x00, &kgdt[7]);    /* descripteur de tss */

    /* initialize the gdtr structure */
    kgdtr.limite = GDTSIZE * 8;
    kgdtr.base = GDTBASE;

    /* copy the gdtr to its memory area */
    memcpy((char *) kgdtr.base, (char *) kgdt, kgdtr.limite);

    /* load the gdtr registry */
    asm("lgdtl (kgdtr)");

    /* initiliaz the segments */
    asm("   movw $0x10, %ax    \n \
            movw %ax, %ds    \n \
            movw %ax, %es    \n \
            movw %ax, %fs    \n \
            movw %ax, %gs    \n \
            ljmp $0x08, $next    \n \
            next:        \n");
}

struct idtr {
    u16 limite;
    u32 base;
} __attribute__ ((packed));

struct idtdesc {
    u16 offset0_15;
    u16 select;
    u16 type;
    u16 offset16_31;
} __attribute__ ((packed));    

void init_idt_desc(u16 select, u32 offset, u16 type, struct idtdesc *desc)
{
    desc->offset0_15 = (offset & 0xffff);
    desc->select = select;
    desc->type = type;
    desc->offset16_31 = (offset & 0xffff0000) >> 16;
    return;
}

void init_idt(void)
{
    /* Init irq */
    int i;
    for (i = 0; i < IDTSIZE; i++)
        init_idt_desc(0x08, (u32)_asm_schedule, INTGATE, &kidt[i]); //

    /* Vectors  0 -> 31 are for exceptions */
    init_idt_desc(0x08, (u32) _asm_exc_GP, INTGATE, &kidt[13]);        /* #GP */
    init_idt_desc(0x08, (u32) _asm_exc_PF, INTGATE, &kidt[14]);     /* #PF */

    init_idt_desc(0x08, (u32) _asm_schedule, INTGATE, &kidt[32]);
    init_idt_desc(0x08, (u32) _asm_int_1, INTGATE, &kidt[33]);

    init_idt_desc(0x08, (u32) _asm_syscalls, TRAPGATE, &kidt[48]);
    init_idt_desc(0x08, (u32) _asm_syscalls, TRAPGATE, &kidt[128]); //48

    kidtr.limite = IDTSIZE * 8;
    kidtr.base = IDTBASE;


    /* Copy the IDT to the memory */
    memcpy((char *) kidtr.base, (char *) kidt, kidtr.limite);

    /* Load the IDTR registry */
    asm("lidtl (kidtr)");
}

void init_pic(void)
{
    /* Initialization of ICW1 */
    io.outb(0x20, 0x11);
    io.outb(0xA0, 0x11);

    /* Initialization of ICW2 */
    io.outb(0x21, 0x20);    /* start vector = 32 */
    io.outb(0xA1, 0x70);    /* start vector = 96 */

    /* Initialization of ICW3 */
    io.outb(0x21, 0x04);
    io.outb(0xA1, 0x02);

    /* Initialization of ICW4 */
    io.outb(0x21, 0x01);
    io.outb(0xA1, 0x01);

    /* mask interrupts */
    io.outb(0x21, 0x0);
    io.outb(0xA1, 0x0);
}

/* 
 * Kernel Space. v_addr < USER_OFFSET are addressed by the kernel pages table
 */
for (i=0; i<256; i++) 
    pdir[i] = pd0[i];

unsigned char kernel =  
{
for (k=0; k<250p; k++ )
    kstart[k] = ks0[k];
}
