#include "Tools.h"

#if defined(__arm__)
#define process_vm_readv_syscall 376
#define process_vm_writev_syscall 377
#elif defined(__aarch64__)
#define process_vm_readv_syscall 270
#define process_vm_writev_syscall 271
#elif defined(__i386__)
#define process_vm_readv_syscall 347
#define process_vm_writev_syscall 348
#else
#define process_vm_readv_syscall 310
#define process_vm_writev_syscall 311
#endif

pid_t target_pid = -1;

ssize_t process_v(pid_t __pid, const struct iovec *__local_iov, unsigned long __local_iov_count, const struct iovec *__remote_iov, unsigned long __remote_iov_count, unsigned long __flags, bool iswrite) {
    return syscall((iswrite ? process_vm_writev_syscall : process_vm_readv_syscall), __pid, __local_iov, __local_iov_count, __remote_iov, __remote_iov_count, __flags);
}

bool pvm(void *address, void *buffer, size_t size, bool write = false) {
    struct iovec local[1];
    struct iovec remote[1];

    local[0].iov_base = buffer;
    local[0].iov_len = size;
    remote[0].iov_base = address;
    remote[0].iov_len = size;

    if (target_pid == -1) {
        target_pid = getpid();
    }

    ssize_t bytes = process_v(target_pid, local, 1, remote, 1, 0, write);
    return bytes == size;
}
bool Tools::Read(void *addr, void *buffer, size_t length) {
    return memcpy(buffer, addr, length) != 0;
}

bool Tools::Write(void *addr, void *buffer, size_t length) {
    return memcpy(addr, buffer, length) != 0;
}

bool Tools::ReadAddr(void *addr, void *buffer, size_t length) {
    unsigned long page_size = sysconf(_SC_PAGESIZE);
    unsigned long size = page_size * sizeof(uintptr_t);
    return mprotect((void *) ((uintptr_t) addr - ((uintptr_t) addr % page_size) - page_size), (size_t) size, PROT_EXEC | PROT_READ | PROT_WRITE) == 0 && memcpy(buffer, addr, length) != 0;
}

bool Tools::WriteAddr(void *addr, void *buffer, size_t length) {
    unsigned long page_size = sysconf(_SC_PAGESIZE);
    unsigned long size = page_size * sizeof(uintptr_t);
    return mprotect((void *) ((uintptr_t) addr - ((uintptr_t) addr % page_size) - page_size), (size_t) size, PROT_EXEC | PROT_READ | PROT_WRITE) == 0 && memcpy(addr, buffer, length) != 0;
}

bool Tools::PVM_ReadAddr(void *addr, void *buffer, size_t length) {
    return pvm(addr, buffer, length, false);
}

bool Tools::PVM_WriteAddr(void *addr, void *buffer, size_t length) {
    return pvm(addr, buffer, length, true);
}

bool Tools::IsPtrValid(void *addr) {
    if (!addr) {
        return false;
    }
    static int fd = -1;
    if (fd == -1) {
        fd = open("/dev/random", O_WRONLY);
    }
    return write(fd, addr, 4) >= 0;
}

uintptr_t Tools::GetBaseAddress(const char *name) {
    uintptr_t base = 0;
    char line[512];

    FILE *f = fopen("/proc/self/maps", "r");

    if (!f) {
        return 0;
    }

    while (fgets(line, sizeof line, f)) {
        uintptr_t tmpBase;
        char tmpName[256];
        if (sscanf(line, "%" PRIXPTR "-%*" PRIXPTR " %*s %*s %*s %*s %s", &tmpBase, tmpName) > 0) {
            if (!strcmp(basename(tmpName), name)) {
                base = tmpBase;
                break;
            }
        }
    }

    fclose(f);
    return base;
}

uintptr_t Tools::GetEndAddress(const char *name) {
    uintptr_t end = 0;
    char line[512];

    FILE *f = fopen("/proc/self/maps", "r");

    if (!f) {
        return 0;
    }

    while (fgets(line, sizeof line, f)) {
        uintptr_t tmpEnd;
        char tmpName[256];
        if (sscanf(line, "%*" PRIXPTR "-%" PRIXPTR " %*s %*s %*s %*s %s", &tmpEnd, tmpName) > 0) {
            if (!strcmp(basename(tmpName), name)) {
                end = tmpEnd;
            } else {
                if (end)
                    break;
            }
        }
    }

    fclose(f);
    return end;
}
