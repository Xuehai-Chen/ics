#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ = 257,
  TK_NUM = 260

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
  {"-", '-'},
  {"/", '/'},
  {"\\*", '*'},
  {"\\(", '('},
  {"\\)", ')'},
  {"[0-9]+", TK_NUM}
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
			case TK_NUM:
				tokens[nr_token].type = TK_NUM;
				strncpy(tokens[nr_token].str, substr_start, substr_len > 32? 32: substr_len);
				break;
          default:
				tokens[nr_token].type = rules[1].token_type;
				strcpy(tokens[nr_token].str,"");
        }
		nr_token++;

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
	int priority = 1;
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
		if((type == '*' || type == '/') && priority == 1){
			op = i;
		}else if(type == '+' || type == '-'){
			op = i;
			priority = 0;
		}
	}
	return op;
}

uint32_t eval(int p, int q){
	if(p > q){
		assert(0);
	}else if(p == q){
		uint32_t result;
		sscanf(tokens[p].str, "%u", &result);
		return result;
	}else if(check_parentheses(p,q) == true){
		return eval(p + 1, q -1);
	}else{
		int op = get_dom_op(p,q);
		uint32_t val1 = eval(p, op - 1);
		uint32_t val2 = eval(op + 1, q);
		switch(tokens[op].type){
			case '+': return val1 + val2;
			case '-': return val1 - val2;
			case '*': return val1 * val2;
			case '/': return val1 / val2;
			default: assert(0);
		}
	}
}

uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */

  return eval(0, nr_token);
}
