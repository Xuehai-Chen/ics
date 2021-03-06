#include "cpu/exec.h"
#include "cpu/cc.h"

make_EHelper(test) {
	rtl_sext(&id_src->val, &id_src->val, id_src->width);
	rtl_sext(&id_dest->val, &id_dest->val, id_dest->width);
	rtl_and(&t0,&id_dest->val,&id_src->val);
	t1 = 0;
	rtl_set_CF(&t1);
	rtl_set_OF(&t1);
	t1 = (int)t0 < 0;
	rtl_set_SF(&t1);
	t1 = t0 == 0;
	rtl_set_ZF(&t1);
	print_asm_template2(test);
}

make_EHelper(and) {
	rtl_sext(&t0, &id_src->val, id_src->width);
	rtl_and(&t1, &id_dest->val, &t0);
	operand_write(id_dest,&t1);
	t2 = 0;
	rtl_set_CF(&t2);
	rtl_set_OF(&t2);
	t2 = (int)t1 < 0;
	rtl_set_SF(&t2);
	t2 = t1 == 0;
	rtl_set_ZF(&t2);
	print_asm_template2(and);
}

make_EHelper(xor) {
	rtl_xor(&t0,&id_dest->val, &id_src->val);
	operand_write(id_dest, &t0);
	t1 = 0;
	rtl_set_CF(&t1);
	rtl_set_OF(&t1);
	t1 = (int)t0 < 0;
	rtl_set_SF(&t1);
	t1 = t0 == 0;
	rtl_set_ZF(&t1);
	print_asm_template2(xor);
}

make_EHelper(or) {
	rtl_or(&t0, &id_dest->val, &id_src->val);
	operand_write(id_dest,&t0);
	t1 = 0;
	rtl_set_CF(&t1);
	rtl_set_OF(&t1);
	t1 = (int)t0 < 0;
	rtl_set_SF(&t1);
	t1 = t0 == 0;
	rtl_set_ZF(&t1);
	print_asm_template2(or);
}

make_EHelper(sar) {
	// unnecessary to update CF and OF in NEMU
	rtl_sar(&t0, &id_dest->val, &id_src->val);
	operand_write(id_dest, &t0);
	t1 = (int)t0 < 0;
	rtl_set_SF(&t1);
	t1 = t0 == 0;
	rtl_set_ZF(&t1);
	print_asm_template2(sar);
}

make_EHelper(shl) {
	// unnecessary to update CF and OF in NEMU
	rtl_shl(&t0, &id_dest->val, &id_src->val);
	operand_write(id_dest, &t0);
	t1 = (int)t0 < 0;
	rtl_set_SF(&t1);
	t1 = t0 == 0;
	rtl_set_ZF(&t1);
	print_asm_template2(shl);
}

make_EHelper(shr) {
	// unnecessary to update CF and OF in NEMU
	rtl_shr(&t0, &id_dest->val, &id_src->val);
	operand_write(id_dest, &t0);
	t1 = (int)t0 < 0;
	rtl_set_SF(&t1);
	t1 = t0 == 0;
	rtl_set_ZF(&t1);
	print_asm_template2(shr);
}

make_EHelper(setcc) {
	uint32_t cc = decoding.opcode & 0xf;

	rtl_setcc(&t2, cc);
	operand_write(id_dest, &t2);

	print_asm("set%s %s", get_cc_name(cc), id_dest->str);
}

make_EHelper(not) {

	rtl_not(&id_dest->val, &id_dest->val);
	operand_write(id_dest, &id_dest->val);
	print_asm_template1(not);
}
