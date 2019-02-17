#include <am.h>
#include <x86.h>
#include <stdio.h>

#define RTC_PORT 0x48   // Note that this is not standard
#define I8042_DATA_PORT 0x60
#define I8042_STATUS_PORT 0x64
static unsigned long boot_time;

void _ioe_init() {
	boot_time = inl(RTC_PORT);
}

unsigned long _uptime() {
	unsigned long now = inl(RTC_PORT);
	return now - boot_time;
}

uint32_t* const fb = (uint32_t *)0x40000;

_Screen _screen = {
	.width  = 400,
	.height = 300,
};

extern void* memcpy(void *, const void *, int);

void _draw_rect(const uint32_t *pixels, int x, int y, int w, int h) {
	int i;
	int cp_bytes = sizeof(uint32_t) * (w < _screen.width - x ? w : _screen.width - x);
	for (i = 0; i < h && i < (_screen.height - y); i++) {
		memcpy(&fb[(y +i) *_screen.width + x], pixels, cp_bytes);
		pixels += w;
	}
}

void _draw_sync() {
}

int _read_key() {
	uint64_t status = inb(I8042_STATUS_PORT);
	if(status == 1){
		uint64_t key = inl(I8042_DATA_PORT);
		//printf("keyboard status:%d, key:0x%x\n", status,key);
		return key;
	}
	return _KEY_NONE;
}
