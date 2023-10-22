// kernal 
// written in C++
// bootloader was written in C


// kernel.cpp
extern "C" {
    void kernel_main() {
        // Kernel code -->
        const char* kernelbasic = "Kernel Is Still in development!";
        volatile unsigned short* video_memory = (volatile unsigned short*)0xB8000;

        for (int i = 0; hello[i] != '\0'; i++) {
            video_memory[i] = (video_memory[i] & 0xFF00) | kernelbasic[i];
        }
    }
}

extern "C" void _start() {
    kernel_main();

    asm volatile ("hlt");
}
