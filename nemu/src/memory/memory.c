#include "nemu.h"

#define PMEM_SIZE (128 * 1024 * 1024)

#define pmem_rw(addr, type) *(type *)({\
		Assert(addr < PMEM_SIZE, "physical address(0x%08x) is out of bound", addr); \
		guest_to_host(addr); \
		})

int is_mmio(paddr_t);
uint32_t mmio_read(paddr_t, int, int);
void mmio_write(paddr_t, int, uint32_t, int);

uint8_t pmem[PMEM_SIZE];

/* Memory accessing interfaces */

uint32_t paddr_read(paddr_t addr, int len) {
	int mmio_id = is_mmio(addr);
	if(mmio_id == -1){
		return pmem_rw(addr, uint32_t) & (~0u >> ((4 - len) << 3));
	}else{
		return mmio_read(addr, len, mmio_id);
	}
}

void paddr_write(paddr_t addr, uint32_t data, int len) {
	int mmio_id = is_mmio(addr);
	if(mmio_id == -1){
		memcpy(guest_to_host(addr), &data, len);
	}else{
		mmio_write(addr, len, data, mmio_id);
	}
}

uint32_t vaddr_read(vaddr_t addr, int len) {
	return paddr_read(addr, len);
}

void vaddr_write(vaddr_t addr, uint32_t data, int len) {
	paddr_write(addr, data, len);
}
