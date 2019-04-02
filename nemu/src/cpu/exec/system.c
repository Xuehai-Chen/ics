#include "cpu/exec.h"
#include "device/mmio.h"
#include "device/port-io.h"

void difftest_skip_ref();
void difftest_skip_dut();
uint32_t pio_read_l(ioaddr_t);
uint32_t pio_read_w(ioaddr_t);
uint32_t pio_read_b(ioaddr_t);
void pio_write_l(ioaddr_t, uint32_t);
void pio_write_w(ioaddr_t, uint32_t);
void pio_write_b(ioaddr_t, uint32_t);
void raise_intr(uint8_t, vaddr_t);

make_EHelper(lidt) {
	t0 = vaddr_read(id_dest->addr + 2, 2);
	rtl_andi(&t0, &t0, 0xffff);
	t1 = vaddr_read(id_dest->addr + 4, 2);
	rtl_shli(&t1, &t1, 16);
	rtl_add(&t0, &t0, &t1);
	t1 = vaddr_read(id_dest->addr, 2);
	rtl_set_idtr(&t0, &t1);

	print_asm_template1(lidt);
}

make_EHelper(mov_r2cr) {
	//Log("dest reg:0x%-10x\tsrc:0x%-10x\twidth:%d",id_dest->reg,id_src->val,id_src->width);
	switch(id_dest->reg){
		case 0:
			rtl_set_cr0(id_src->val);
			break;
		case 3:
			rtl_set_cr3(id_src->val);
			break;
		default:
			assert(0);
	}

	print_asm("movl %%%s,%%cr%d", reg_name(id_src->reg, 4), id_dest->reg);
}

make_EHelper(mov_cr2r) {
	//Log("dest:0x%-10x\tsrc reg:0x%-10x\twidth:%d",id_dest->val,id_src->reg,id_src->width);
	switch(id_src->reg){
		case 0:
			rtl_get_cr0(&t0);
			break;
		case 3:
			rtl_get_cr3(&t0);
			break;
		default:
			assert(0);
	}
	operand_write(id_dest, &t0);

	print_asm("movl %%cr%d,%%%s", id_src->reg, reg_name(id_dest->reg, 4));

#if defined(DIFF_TEST)
	difftest_skip_ref();
#endif
}

make_EHelper(int) {
	raise_intr(id_dest->val, decoding.seq_eip);

	print_asm("int %s", id_dest->str);

#if defined(DIFF_TEST) && defined(DIFF_TEST_QEMU)
	difftest_skip_dut();
#endif
}

make_EHelper(iret) {
	rtl_pop(&decoding.jmp_eip);
	rtl_j(decoding.jmp_eip);
	rtl_pop(&t0);
	rtl_set_cs(&t0);
	rtl_pop(&t0);
	rtl_set_eflags(&t0);

	print_asm("iret");
}

make_EHelper(in) {
	switch(id_dest->width){
		case 4:
			id_dest->val = pio_read_l(id_src->val);
			break;
		case 2:
			id_dest->val = pio_read_w(id_src->val);
			break;
		case 1:
			id_dest->val = pio_read_b(id_src->val);
			break;
	}
	operand_write(id_dest, &id_dest->val);
	//Log("dest:0x%-10x\tsrc:0x%-10x\twidth:%d",id_dest->val,id_src->val,id_src->width);

	print_asm_template2(in);

#if defined(DIFF_TEST)
	difftest_skip_ref();
#endif
}

make_EHelper(out) {
	//Log("dest:0x%-10x\tsrc:0x%-10x\twidth:%d",id_dest->val,id_src->val,id_src->width);
	switch(id_src->width){
		case 4:
			pio_write_l(id_dest->val, id_src->val);
			break;
		case 2:
			pio_write_w(id_dest->val, id_src->val);
			break;
		case 1:
			pio_write_b(id_dest->val, id_src->val);
			break;
	}
	print_asm_template2(out);

#if defined(DIFF_TEST)
	difftest_skip_ref();
#endif
}
