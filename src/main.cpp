#include <gccore.h>
#include <ogc/system.h>
#include <ogc/video.h>
#include <cstdio>

#define noinline __attribute__((noinline))

noinline
int test() {
    return 42;
}

int main() {
    VIDEO_Init();

    auto* mode = VIDEO_GetPreferredMode(nullptr);
    void* framebuffer = MEM_K0_TO_K1(
        SYS_AllocateFramebuffer(mode)
    );

    console_init(
        framebuffer, 20, 20,
        mode->fbWidth, mode->xfbHeight,
        mode->fbWidth * VI_DISPLAY_PIX_SZ
    );

    VIDEO_Configure(mode);
    VIDEO_SetNextFramebuffer(framebuffer);
    VIDEO_SetBlack(FALSE);
    VIDEO_Flush();
    VIDEO_WaitVSync();

    printf("Hello, world!\n");
    printf("test() = %d\n", test());

    while (true)
        VIDEO_WaitVSync();
}