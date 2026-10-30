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

void evil() {
    int (*func)() = test;
    short* _0x2aPtr = reinterpret_cast<short *>(func) + 3;
    _0x2aPtr[0] = 69;

    invalidate(_0x2aPtr);
}

int test() {
    volatile int it2 = 0x2a;
    return it2;
}

int main() {
    try {
        init();

        printf("test() = %d\n", test());
        evil();
        kr1v::evil();
        printf("test() = %d\n", test());

        while (true)
            VIDEO_WaitVSync();
    } catch (error_t e) {
        printf("error: %d", e);
    }
}