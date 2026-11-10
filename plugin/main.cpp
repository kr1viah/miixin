#include <cstdio>

#include "../src/mixin.h"

#ifndef KR1V
int main() {
    printf("plugin main");
}
#endif

Inject("test", At(AtType::HEAD))
void injectTest() {
}
EndInject

WrapOperation("test2", At(AtType::INVOKE, "test"))
int injectTwo(int (*original)()) {
    return original() + 5;
}
EndWrapOperation