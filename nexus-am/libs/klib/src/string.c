#include "klib.h"

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
	size_t len = 0;
	while(*(s + len) != '\0'){
		len++;
	}
	return len;
}

char *strcpy(char* dst,const char* src) {
	char* res = dst;
	do{
		*dst = *src;
		dst++;
		src++;
	}while(*src != '\0');
	return res;
}

char* strncpy(char* dst, const char* src, size_t n) {
	char* res = dst;
	size_t offset = 0;
	while(*src != '\0' && offset < n){
		*dst = *src;
		dst++;
		src++;
		offset++;
	}
	return res;
}

char* strcat(char* dst, const char* src) {
	char* res = dst;
	while(*dst != '\0'){
		dst++;
	}
	do{
		*dst = *src;
		dst++;
		src++;
	}while(*src != '\0');

	return res;
}

int strcmp(const char* s1, const char* s2) {
	//_putc('\n');_putc('c');_putc('m');_putc('p');
	while(1){
		//_putc(*s1);_putc(*s2);
		if(*s1 == *s2){
			s1++;
			s2++;
			if(*s1 == '\0') break;
			continue;
		}else if(*s1 < *s2){
			return -1;
		}else{
			return 1;
		}
	}
	return 0;
}

int strncmp(const char* s1, const char* s2, size_t n) {
	size_t offset = 0;
	while(offset < n){
		if(*s1 == *s2){
			s1++;
			s2++;
			offset++;
			if(*s1 == '\0') break;
			continue;
		}else if(*s1 < *s2){
			return -1;
		}else{
			return 1;
		}
	}
	return 0;
}

void* memset(void* v,int c,size_t n) {
	char* buf = (char*) v;
	for(size_t i = 0; i < n; i++){
		*(buf + i) = (char) c;
	}
	return v;
}

void* memcpy(void* out, const void* in, size_t n) {
	char* dst = out;
	char* src = (char*)in;
	size_t offset = 0;
	while(offset < n){
		*dst = *src;
		dst++;
		src++;
		offset++;
	}
	return out;
}

int memcmp(const void* s1, const void* s2, size_t n){
	char* c1 = (char*)s1;
	char* c2 = (char*)s2;
	size_t offset = 0;
	while(offset < n){
		if(*c1 == *c2){
			c1++;
			c2++;
			offset++;
			continue;
		}else if(*c1 < *c2){
			return -1;
		}else{
			return 1;
		}
	}
	return 0;
}

#endif
