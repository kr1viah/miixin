#include <gccore.h>
#include <ogc/system.h>
#include <ogc/video.h>
#include <cstdio>

#define noinline __attribute__((noinline))

noinline
int it(int in) {
    return in;
}

noinline
int test() {
    volatile int it2 = it(0x2a);
    return it2;
}

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
    int* pointer = reinterpret_cast<int *>(*func);
    printf("pointer = %p (%x)\n", pointer+0, pointer[0]);
    printf("pointer = %p (%x)\n", pointer+1, pointer[1]);
    printf("pointer = %p (%x)\n", pointer+2, pointer[2]);
    printf("pointer = %p (%x)\n", pointer+3, pointer[3]);

    short* _0x2aPtr = reinterpret_cast<short *>(pointer) + 7;
    printf("_0x2aPtr = %p (%x)\n", _0x2aPtr, _0x2aPtr[0]);
    _0x2aPtr[0] = 69;
    printf("_0x2aPtr = %p (%x)\n", _0x2aPtr, _0x2aPtr[0]);
}

int main() {
    init();

    printf("test() = %d\n", test());
    evil();
    printf("test() = %d\n", test());

    while (true)
        VIDEO_WaitVSync();
}