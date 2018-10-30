#include <ucontext.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <setjmp.h>
#include <errno.h>
#include <bits/types/siginfo_t.h>
#include <sys/ucontext.h>
#include <sys/mman.h>

jmp_buf jump;

void memory_handler(int signal, siginfo_t* info, void* context) {
    if (signal == SIGSEGV) {
        longjmp(jump, 1);
    }
}


int check_addr(char* addr) {
    sigset_t x;
    sigemptyset(&x);
    sigaddset(&x, SIGSEGV);
    sigprocmask(SIG_UNBLOCK, &x, NULL);
    struct sigaction action;
    action.sa_flags = SA_SIGINFO;
    action.sa_sigaction = memory_handler;
    action.sa_mask = x;
    if (sigaction(SIGSEGV, &action, NULL) < 0) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    if (setjmp(jump) == 0) {
        return (int)*(addr);
    } else {
        return 1000;
    }
}
const int amount = 10;
void get_memory(char* addr) {
    printf("Memory:\n");
    addr -= amount;
    for (int i = -amount; i < amount; i++, addr++) {
        char prefix[] = "  ";
        if (i == 0) {
            memcpy(prefix, "=>", 2);
        }
        if (check_addr(addr) != 1000) {
            printf("%saddress = %p : data = '%c'(%d)\n", prefix, addr, *addr, *addr);
        } else {
            printf("%sbad address at address = %p\n", prefix, addr);
        }
    }
}
const char *regs_name[] = {
        "REG_R8", "REG_R9", "REG_R10", "REG_R11", "REG_R12", "REG_R13", "REG_R14", "REG_R15", "REG_RDI", "REG_RSI",
        "REG_RBP", "REG_RBX", "REG_RDX", "REG_RAX", "REG_RCX", "REG_RSP", "REG_RIP", "REG_EFL", "REG_CSGSFS", "REG_ERR",
        "REG_TRAPNO", "REG_OLDMASK", "REG_CR2"
};
void get_general_purpose_registers(void* context) {
    mcontext_t mcontext = ((ucontext_t *)context)->uc_mcontext;
    ssize_t len = __NGREG;
    printf("Registers:\n");
    for (int i = 0; i < len; i++) {
        printf("  %s = %u\n", regs_name[i], (unsigned int) mcontext.gregs[i]);
    }
}

void handler(int signal, siginfo_t* info, void* context) {
    if (signal == SIGSEGV) {
        printf("Segmentation fault. It was at memory address = %p\n", info->si_addr);
        get_memory((char *)info->si_addr);
        get_general_purpose_registers(context);
        exit(0);
    }
}

int main() {
    struct sigaction *action = calloc(0x00, sizeof(struct sigaction));
    action->sa_flags = SA_SIGINFO;
    action->sa_sigaction = handler;
    if (sigaction(SIGSEGV, action, NULL) < 0) {
        perror("sigaction");
        return 1;
    }
    char* ptr = (char *)mmap(NULL, 3, PROT_READ, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    ptr[-3] = '1';
    return 0;
}