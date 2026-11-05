#include <cstdio>

#include "../src/mixin.h"

int main() {
    printf("plugin main");
}

Inject("test", At(AtType::HEAD))
void injectTest() {
}
EndInject