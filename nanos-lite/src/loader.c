#include "common.h"

#define DEFAULT_ENTRY ((void *)0x4000000)

size_t get_ramdisk_size();

void ramdisk_read(void*, off_t, size_t);

uintptr_t loader(_Protect *as, const char *filename) {
	size_t ramdisk_size = get_ramdisk_size();
	ramdisk_read(DEFAULT_ENTRY, 0, ramdisk_size);
  return (uintptr_t)DEFAULT_ENTRY;
}
