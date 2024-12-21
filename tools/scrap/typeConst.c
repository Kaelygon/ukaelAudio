






#include <stdio.h>

typedef struct {
    const char* const addr; //Neither the value nor adress should be modified. Read-only
} ConstTest;

const char* const* getAddr(const ConstTest* input) {
    return &input->addr;
}

void readAddr(const ConstTest* input) {
    const char* const* litAddrPtr = getAddr(input);

    printf("%p\n", (void *)litAddrPtr);
    printf("%s\n", *litAddrPtr);
}

int main() { 
    const char* literal = "A4AA0F38";

    //read only struct to be passed around 
    const ConstTest sample = { .addr = literal }; 

    readAddr(&sample);

    return 0;
}
