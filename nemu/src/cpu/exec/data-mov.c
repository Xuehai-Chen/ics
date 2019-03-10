#include "cpu/exec.h"

make_EHelper(mov) {
	operand_write(id_dest, &id_src->val);
	print_asm_template2(mov);
}

make_EHelper(push) {
	rtl_push(&id_dest->val);
	print_asm_template1(push);
}

make_EHelper(pop) {
	rtl_pop(&id_dest->val);
	operand_write(id_dest, &id_dest->val);

	print_asm_template1(pop);
}

make_EHelper(pusha) {
	t3 = decoding.is_operand_size_16 ? 2 : 4;
	rtl_lr(&t0, R_ESP, t3);
	rtl_lr(&t1, R_EAX, t3);
	rtl_push(&t1);
	rtl_lr(&t1, R_ECX, t3);
	rtl_push(&t1);
	rtl_lr(&t1, R_EDX, t3);
	rtl_push(&t1);
	rtl_lr(&t1, R_EBX, t3);
	rtl_push(&t1);
	rtl_push(&t0);
	rtl_lr(&t1, R_EBP, t3);
	rtl_push(&t1);
	rtl_lr(&t1, R_ESI, t3);
	rtl_push(&t1);
	rtl_lr(&t1, R_EDI, t3);
	rtl_push(&t1);

	print_asm("pusha");
}

make_EHelper(popa) {
	t3 = decoding.is_operand_size_16 ? 2 : 4;
	rtl_pop(&t0);
	rtl_sr(R_EDI, &t0, t3);
	rtl_pop(&t0);
	rtl_sr(R_ESI, &t0, t3);
	rtl_pop(&t0);
	rtl_sr(R_EBP, &t0, t3);
	rtl_pop(&t0);
	rtl_pop(&t0);
	rtl_sr(R_EBX, &t0, t3);
	rtl_pop(&t0);
	rtl_sr(R_EDX, &t0, t3);
	rtl_pop(&t0);
	rtl_sr(R_ECX, &t0, t3);
	rtl_pop(&t0);
	rtl_sr(R_EAX, &t0, t3);

	print_asm("popa");
}

make_EHelper(leave) {
	id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
	rtl_lr(&t1, 5, id_dest->width);
	rtl_sr(4, &t1, id_dest->width);
	rtl_pop(&t2);
	rtl_sr(5, &t2, id_dest->width);

	print_asm("leave");
}

make_EHelper(cltd) {
	if (decoding.is_operand_size_16) {
		rtl_lr(&t0, R_AX, 2);
		if((t0 >> 15) & 0x1){
			t1 = 0xFFFF;
		}else{
			t1 = 0;
		}
		rtl_sr(R_DX, &t1, 2);
	}
	else {
		rtl_lr(&t0, R_EAX, 4);
		if((int)t0 < 0){
			t1 = 0xFFFFFFFF;
		}else{
			t1 = 0;
		}
		rtl_sr(R_EDX, &t1, 4);
	}

	print_asm(decoding.is_operand_size_16 ? "cwtl" : "cltd");
}

make_EHelper(cwtl) {
	if (decoding.is_operand_size_16) {
		rtl_lr(&t0, R_AL, 1);
		rtl_sext(&t1, &t0, 1);
		rtl_sr(R_AX, &t1, 2);
	}
	else {
		rtl_lr(&t0, R_AX, 2);
		rtl_sext(&t1, &t0, 2);
		rtl_sr(R_EAX, &t1, 4);
	}

	print_asm(decoding.is_operand_size_16 ? "cbtw" : "cwtl");
}

make_EHelper(movsx) {
	id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
	rtl_sext(&t0, &id_src->val, id_src->width);
	operand_write(id_dest, &t0);
	print_asm_template2(movsx);
}

make_EHelper(movzx) {
	id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
	operand_write(id_dest, &id_src->val);
	print_asm_template2(movzx);
}

make_EHelper(lea) {
	operand_write(id_dest, &id_src->addr);
	print_asm_template2(lea);
}
