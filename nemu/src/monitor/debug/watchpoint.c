#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
	int i;
	for (i = 0; i < NR_WP; i ++) {
		wp_pool[i].NO = i;
		wp_pool[i].next = &wp_pool[i + 1];
	}
	wp_pool[NR_WP - 1].next = NULL;

	head = NULL;
	free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP* get_head(){
	return head;
}

WP* new_wp(){
	if(free_ == NULL) assert(0);
	WP* result = free_;
	free_ = free_->next;
	WP* temp = head;
	result->next = temp;
	head = result;
	return result;
}

void free_wp(int n){
	WP *wp = &wp_pool[n];
	WP* curr = head;
	WP* prev = head;
	while(curr->NO != wp->NO){
		prev = curr;
		curr = curr->next;
	}
	if(curr == NULL) assert(0);
	if(curr != prev){
		prev->next = curr->next;
	}else{
		head = curr->next;
	}
	WP* temp = free_;
	wp->next = temp;
	free_ = wp;
}

bool check_watchpoints(){
	bool result = false;
	WP* curr = head;
	while(curr != NULL){
		bool success;
		uint32_t value = expr(curr->expr, &success);
		if(!success){
			assert(0);
		}else{
			if(value != curr->value){
				result = true;
				printf("watchpoint NO.%d hit:\nexpr: %s\nold value = %-10d\t0x%-10x\tnew value = %-10d\t0x%-10x\n", curr->NO, curr->expr, curr->value, curr->value, value, value);
				curr->value = value;
			}
		}
		curr = curr->next;
	}
	return result;
}

