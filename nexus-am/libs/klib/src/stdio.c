#include "klib.h"
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
	char buf[1024];
	va_list ap;
	va_start(ap, fmt);
	int res = vsprintf(buf, fmt, ap);
	va_end(ap);
	for(int i = 0; i < res; i++){
		_putc(*(buf+i));
	}
	return res;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
const char *src = fmt;
	char *buf = out;
	char  *s;
	int i;
	memset(out, '\0', strlen(out));
	while(*src){ 
		//_putc('\n');
		int count = 0;
		while(*src && *src != '%'){ 
			//_putc(*src);
			src++;
			count++;
		}
		memcpy(buf, src - count, count);
		/**
		_putc(count+'0');
		for(int j = 0; j< count; j++){
			_putc(*(buf+j));
		}
		*/
		buf += count;
		if(!*src) break;
		src++;
		//_putc(*src);
		int idx = 1000000000;
		/**
		while(*src <= '9' && *src >= '0') {
			_putc(*src);
			//_putc('\n');
			src++;
		}
		*/
		switch(*(src++)){
			case 'c':
			case 's':
				s = va_arg(ap, char*);
				strcpy(buf, s);
				buf+=strlen(s);
				break;
			case 'd':
				i = va_arg(ap, int);
				if(i == 0){
					*(buf++) = '0';
					break;
				}else if(i < 0){
					*(buf++) = '-';
				}
				while(i/idx == 0) idx/=10;
				while(idx > 0){
					*(buf++) = '0' + i/idx;
					i = i%idx;
					idx/=10;
				}
				break;
		}
	}
	/**
	_putc('\n');
	for(int j = 0; j < strlen(out); j++){
		_putc(*(out+j));
	}
	*/
	return buf-out;
}

int sprintf(char *out, const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	int res = vsprintf(out, fmt, ap);
	va_end(ap);
	return res;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
	_putc('\n');
	_putc('!');
	_putc('\n');
	return 0;
}

#endif
