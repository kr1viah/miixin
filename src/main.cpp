#include <gccore.h>
#include <ogc/cache.h>
#include <ogc/system.h>
#include <ogc/video.h>
#include <cstdio>
#include "evil.cpp"

#define invalidate(x) do { DCFlushRange(x, sizeof(*x)); ICInvalidateRange(x, sizeof(*x)); } while(0)

int test();

void init() {
    VIDEO_Init();

    GXRModeObj *mode = VIDEO_GetPreferredMode(nullptr);
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
}

int test() {
    volatile int it2 = 0xffffff;
    return it2;
}

int test2() {
    volatile int it = test();
    return it;
}

int main() {
    init();
    kr1v::evil();

    printf("test() = %d\n", test());

    while (SYS_MainLoop())
        VIDEO_WaitVSync();
}

#define KR1V
#include "../plugin/main.cpp"