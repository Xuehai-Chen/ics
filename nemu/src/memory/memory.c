#include "nemu.h"

#define PMEM_SIZE (128 * 1024 * 1024)

#define pmem_rw(addr, type) *(type *)({\
		Assert(addr < PMEM_SIZE, "physical address(0x%08x) is out of bound", addr); \
		guest_to_host(addr); \
		})

uint8_t pmem[PMEM_SIZE];

int is_mmio(paddr_t);

uint32_t mmio_read(paddr_t, int, int);

void mmio_write(paddr_t, int, uint32_t, int);

enum{TYPE_WRITE, TYPE_READ};
/* Memory accessing interfaces */

paddr_t page_translate(paddr_t addr, int type){
	uint32_t cr3 = rtl_get_cr3();
	uint32_t pte_idx = paddr_read((paddr_t)((void *)cr3 + ((addr > 22) & 0x3ff)), 4);
	assert(pte_idx &0x1);
	pte_addr = (paddr_t)((void*)pte_idx + ((addr > 12) & 0x3ff));
	uint32_t pte = paddr_read(pte_addr, 4);
	assert(pte & 0x1);
	switch(type){
		case TYPE_WRITE:
			paddr_write(pte_addr, 4, pte | 0x40);
			break;
		case TYPE_READ:
			paddr_write(pte_addr, 4, pte | 0x20);
			break;
		default:
			assert(0);
	}
	return (addr & 0xfff) + (pte & ~(0xfff));
}

uint32_t paddr_read(paddr_t addr, int len) {
	int mmio_id = is_mmio(addr);
	if(mmio_id == -1){
		return pmem_rw(addr, uint32_t) & (~0u >> ((4 - len) << 3));
	}else{
		return mmio_read(addr, len, mmio_id);
	}
}

void paddr_write(paddr_t addr, int len, uint32_t data) {
	int mmio_id = is_mmio(addr);
	if(mmio_id == -1){
		memcpy(guest_to_host(addr), &data, len);
	}else{
		mmio_write(addr, len, data, mmio_id);
	}
}

uint32_t vaddr_read(vaddr_t addr, int len) {
	if((((void*)addr + len)^addr) & ~0x3ff) assert(0);
	return paddr_read(page_translate(addr, TYPE_READ), len);
}

void vaddr_write(vaddr_t addr, int len, uint32_t data) {
	if((((void*)addr + len)^addr) & ~0x3ff) assert(0);
	paddr_write(page_translate(addr, TYPE_WRITE), len, data);
}
