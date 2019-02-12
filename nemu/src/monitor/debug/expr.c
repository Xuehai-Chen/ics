#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
	TK_NOTYPE = 256, TK_EQ = 257, TK_NEQ = 258,
	TK_DEC = 259, TK_HEX = 260, TK_REG = 261,
	TK_DEREF = 262, TK_NEG = 263,
	TK_AND = 264, TK_OR = 265, TK_NOT = 266

		/* TODO: Add more token types */
};

static struct rule {
	char *regex;
	int token_type;
} rules[] = {

	/* TODO: Add more rules.
	 * Pay attention to the precedence level of different rules.
	 */

	{" +", TK_NOTYPE},    // spaces
	{"\\+", '+'},         // plus
	{"==", TK_EQ},         // equal
	{"!=", TK_NEQ},
	{"-", '-'},
	{"/", '/'},
	{"\\*", '*'},
	{"\\(", '('},
	{"\\)", ')'},
	{"0x[A-Fa-f0-9]+", TK_HEX},
	{"[0-9]+", TK_DEC},
	{"\\$.{2,3}", TK_REG},
	{"&&", TK_AND},
	{"\\|\\|", TK_OR},
	{"!", TK_NOT}
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
	int i;
	char error_msg[128];
	int ret;

	for (i = 0; i < NR_REGEX; i ++) {
		ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
		if (ret != 0) {
			regerror(ret, &re[i], error_msg, 128);
			panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
		}
	}
}

typedef struct token {
	int type;
	char str[32];
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
	int position = 0;
	int i;
	regmatch_t pmatch;

	nr_token = 0;

	while (e[position] != '\0') {
		/* Try all rules one by one. */
		for (i = 0; i < NR_REGEX; i ++) {
			if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
				char *substr_start = e + position;
				int substr_len = pmatch.rm_eo;

				Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
						i, rules[i].regex, position, substr_len, substr_len, substr_start);
				position += substr_len;

				/* TODO: Now a new token is recognized with rules[i]. Add codes
				 * to record the token in the array `tokens'. For certain types
				 * of tokens, some extra actions should be performed.
				 */

				switch (rules[i].token_type) {
					case TK_NOTYPE:
						break;
					case TK_DEC:
					case TK_HEX:
					case TK_REG:
						tokens[nr_token].type = rules[i].token_type;
						memset(tokens[nr_token].str, 0, 32);
						strncpy(tokens[nr_token].str, substr_start, substr_len > 32? 32: substr_len);
						nr_token++;
						break;
					default:
						tokens[nr_token].type = rules[i].token_type;
						nr_token++;
				}
				break;
			}
		}

		if (i == NR_REGEX) {
			printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
			return false;
		}
	}

	return true;
}

uint8_t check_parentheses(int p, int q){
	if(tokens[p].type != '(' || tokens[q].type != ')') return false;
	int count = 0;
	for(int i = p+1; i < q; i++){
		if(tokens[i].type == '(') count++;
		if(tokens[i].type == ')') count--;
		if(count < 0) assert(0);
	}
	if(count == 0) return true;
	return false;
}

int get_dom_op(int p, int q){
	int op = p;
	int priority = 2;
	int parentheses_count = 0;
	for(int i = p; i < q; i ++){
		int type = tokens[i].type;
		if(type == '(' ){
			parentheses_count++;
			continue;
		}
		if(type == ')'){
			parentheses_count--;
			continue;
		}
		if(parentheses_count != 0) continue;
		if(priority > 1 && (type == TK_NEG || type == TK_DEREF || type == TK_NOT)){
			op = i;
		}else if(priority > 0 && (type == '*' || type == '/')){
			op = i;
			priority = 1;
		}else if(type == '+' || type == '-' || type == TK_EQ || type == TK_NEQ){
			op = i;
			priority = 0;
		}
	}
	return op;
}

uint32_t get_reg(int p){
	uint32_t result = 0;
	char reg[32];
	sscanf(tokens[p].str, "$%s", reg);
	int index = 0, width = 4;
	if(!strcmp("eip", reg)){
		return cpu.eip;
	}
	for (int i=0; i<3; i++){
		for (int j=0; j<8; j++){
			if(!strcmp(reg_name(j,1<<i), reg)){
				width = 1<<i;
				index = j;
				break;
			}
		}
	}
	switch(width){
		case 4:
			result = reg_l(index);
			break;
		case 2:
			result = reg_w(index);
			break;
		case 1:
			result = reg_b(index);
	}
	return result;
}

uint32_t eval(int p, int q, bool *success){
	if(p > q){
		*success = false;
		return 0;
	}else if(p == q){
		uint32_t result = 0;
		switch(tokens[p].type){
			case TK_DEC:
				sscanf(tokens[p].str, "%u", &result);
				break;
			case TK_HEX:
				sscanf(tokens[p].str, "0x%x", &result);
				break;
			case TK_REG:
				result = get_reg(p);
		}
		return result;
	}else if(check_parentheses(p,q) == true){
		return eval(p + 1, q -1, success);
	}else{
		int op = get_dom_op(p,q);
		uint32_t val1 = 0;
		if(op != p){
			val1 = eval(p, op - 1, success);
		}
		uint32_t val2 = eval(op + 1, q, success);
		switch(tokens[op].type){
			case '+': return val1 + val2;
			case '-': return val1 - val2;
			case '*': return val1 * val2;
			case '/': return val1 / val2;
			case TK_EQ: return val1 == val2;
			case TK_NEQ: return val1 != val2;
			case TK_AND: return val1 && val2;
			case TK_OR: return val1 || val2;
			case TK_NOT: return !val2;
			case TK_DEREF: return vaddr_read(val2, 4);
			case TK_NEG: return -val2;
			default:
						 *success = false;
						 return 0;
		}
	}
}

uint32_t expr(char *e, bool *success) {
	*success = true;
	if (!make_token(e)) {
		*success = false;
		return 0;
	}

	/* TODO: Insert codes to evaluate the expression. */
	for(int i = 0; i < nr_token; i++){
		if(tokens[i].type == '*' && (i == 0||(tokens[i-1].type != TK_DEC && tokens[i-1].type != TK_HEX))){
			tokens[i].type = TK_DEREF;
		}else if(tokens[i].type == '-' && (i == 0|| (tokens[i-1].type != TK_DEC && tokens[i-1].type != TK_HEX))){
			tokens[i].type = TK_NEG;
		}
	}

	return eval(0, nr_token - 1, success);
}
