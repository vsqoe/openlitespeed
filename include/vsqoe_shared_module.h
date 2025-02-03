#ifndef VSQOE_SHARED_MODULE_H
#define VSQOE_SHARED_MODULE_H

#include <stdint.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    volatile uint64_t value;
} shared_struct;

static inline shared_struct* shared_value_init(const char* filename) {
    // Create a new file for the shared value
    int fd = open(filename, O_RDWR | O_CREAT, 0666);
    if (fd == -1) {
        return NULL;
    }
    // Set the file size to the size of the shared structure
    if (ftruncate(fd, sizeof(shared_struct)) == -1) {
        close(fd);
        return NULL;
    }
    // Map the file into memory
    shared_struct* shared_value = (shared_struct *)
        mmap(NULL, sizeof(shared_struct), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shared_value == MAP_FAILED) {
        close(fd);
        return NULL;
    }
    close(fd);
    return shared_value;
}

static inline void shared_value_write(shared_struct* shared_value, uint64_t value) {
    // Use a store-release operation to ensure visibility
    __atomic_store_n(&shared_value->value, value, __ATOMIC_RELEASE);
}

static inline uint64_t shared_value_read(shared_struct* shared_value) {
    // Use a load-acquire operation to ensure visibility
    return __atomic_load_n(&shared_value->value, __ATOMIC_ACQUIRE);
}

#ifdef __cplusplus
}
#endif

#endif // VSQOE_SHARED_MODULE_H
