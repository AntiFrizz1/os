#include <cstdio>
#include <unistd.h>
#include <fcntl.h>
#include <cerrno>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <cstdlib>
#include <string.h>

int main(int argc, char* argv[]) {
    unsigned char code[] = {
            0xb8, 0x00, 0x00, 0x00, 0x00,
            0xba, 0x00, 0x00, 0x00, 0x00,
            0x01, 0xd0,
            0xc3
    };
    if (argc != 3) {
        printf("Incorrect amount of arguments\n");
        return 1;
    }
    int a = atoi(argv[1]);
    int b = atoi(argv[2]);
    for (int i = 0; i < 4; i++) {
        code[i + 1] = a % 256;
        code[i + 6] = b % 256;
        a /= 256;
        b /= 256;
    }
    void *ptr = mmap(0, sizeof(code), PROT_READ|PROT_WRITE, MAP_ANONYMOUS| MAP_PRIVATE, -1, 0);
    if (ptr == MAP_FAILED) {
        perror("Error: Can't call mmap");
        return 1;
    }
    ptr = memcpy(ptr, code, sizeof(code));
    if (mprotect(ptr, sizeof(code), PROT_EXEC) < 0) {
        perror("Error: Can't call mprotect");
    }
    int (*f)() = (int (*)())(ptr);
    printf("%d\n", (*f)());
    if (munmap(ptr, sizeof(code)) < 0) {
        perror("munmap");
    }
    return 0;
}