#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint64_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
	static char *line_read = NULL;

	if (line_read) {
		free(line_read);
		line_read = NULL;
	}

	line_read = readline("(nemu) ");

	if (line_read && *line_read) {
		add_history(line_read);
	}

	return line_read;
}

static int cmd_c(char *args) {
	cpu_exec(-1);
	return 0;
}

static int cmd_q(char *args) {
	return -1;
}

static int cmd_help(char *args);

static int cmd_si(char *args);

static int cmd_info(char *args);

static int cmd_x(char *args);

static int cmd_w(char *args);

static int cmd_d(char *args);

static struct {
	char *name;
	char *description;
	int (*handler) (char *);
} cmd_table [] = {
	{ "help", "Display informations about all supported commands", cmd_help },
	{ "c", "Continue the execution of the program", cmd_c },
	{ "q", "Exit NEMU", cmd_q },
	{ "si", "single execution", cmd_si },
	{ "info", "print prog status", cmd_info },
	{ "x", "print memory", cmd_x },
	{ "w", "set watchpoint", cmd_w},
	{ "d", "delete watchpoint", cmd_d}

	/* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
	/* extract the first argument */
	char *arg = strtok(NULL, " ");
	int i;

	if (arg == NULL) {
		/* no argument given */
		for (i = 0; i < NR_CMD; i ++) {
			printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
		}
	}
	else {
		for (i = 0; i < NR_CMD; i ++) {
			if (strcmp(arg, cmd_table[i].name) == 0) {
				printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
				return 0;
			}
		}
		printf("Unknown command '%s'\n", arg);
	}
	return 0;
}

static int cmd_si(char *args) {
	uint64_t n;
	if(args == NULL){
		n = 1;
	}else{
		sscanf(args, "%lu", &n);
	}
	cpu_exec(n);
	return 0;
}

static int cmd_info(char *args) {
	if(!strcmp(args, "r")){
		printf("info of registers:\n");
		printf("eax:\t0x%-10x\t%d\n",cpu.eax,cpu.eax);
		printf("ecx:\t0x%-10x\t%d\n",cpu.ecx,cpu.ecx);
		printf("edx:\t0x%-10x\t%d\n",cpu.edx,cpu.edx);
		printf("ebx:\t0x%-10x\t%d\n",cpu.ebx,cpu.ebx);
		printf("esp:\t0x%-10x\t%d\n",cpu.esp,cpu.esp);
		printf("ebp:\t0x%-10x\t%d\n",cpu.ebp,cpu.ebp);
		printf("esi:\t0x%-10x\t%d\n",cpu.esi,cpu.esi);
		printf("edi:\t0x%-10x\t%d\n",cpu.edi,cpu.edi);
		printf("eip:\t0x%-10x\t%d\n",cpu.eip,cpu.eip);
		printf("EFLAGS:\t0x%-10x\t%d\n",cpu.EFLAGS.val,cpu.EFLAGS.val);
	}else if(!strcmp(args, "w")){
		WP* curr = get_head();
		printf("watchpoints:\n");
		while(curr != NULL){
			printf("NO.%d\texpr:%s\tcurrent value:%u\n", curr->NO, curr->expr, curr->value);
			curr = curr->next;
		}
	}
	return 0;
}

static int cmd_x(char *args) {
	int n;
	paddr_t addr;
	char *arg = strtok(args, " ");
	sscanf(arg, "%d", &n);
	arg += strlen(arg) + 1;
	bool success;
	addr = expr(arg, &success);
	for (int i = 0; i<n; i++){
		if(i%4 == 0) printf("0x%x:\t", addr + 4*i);
		printf("0x%-10x\t",vaddr_read(addr + 4*i, 4));
		if(i%4 == 3 || i == n-1) printf("\n");
	}
	return 0;
}

static int cmd_w(char *args) {
	WP* wp = new_wp();
	strncpy(wp->expr, args, strlen(args) > 1024 ? 1024: strlen(args));
	bool success;
	uint32_t result;
	result = expr(args, &success);
	if(success){
		wp->value = result;
	}else {assert(0);}
	return 0;
}

static int cmd_d(char *args) {
	int n;
	sscanf(args, "%d", &n);
	if(n >= 32) assert(0);
	free_wp(n);
	return 0;
}

void ui_mainloop(int is_batch_mode) {
	if (is_batch_mode) {
		cmd_c(NULL);
		return;
	}

	while (1) {
		char *str = rl_gets();
		char *str_end = str + strlen(str);

		/* extract the first token as the command */
		char *cmd = strtok(str, " ");
		if (cmd == NULL) { continue; }

		/* treat the remaining string as the arguments,
		 * which may need further parsing
		 */
		char *args = cmd + strlen(cmd) + 1;
		if (args >= str_end) {
			args = NULL;
		}

#ifdef HAS_IOE
		extern void sdl_clear_event_queue(void);
		sdl_clear_event_queue();
#endif

		int i;
		for (i = 0; i < NR_CMD; i ++) {
			if (strcmp(cmd, cmd_table[i].name) == 0) {
				if (cmd_table[i].handler(args) < 0) { return; }
				break;
			}
		}

		if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
	}
}
