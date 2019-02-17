#include "common.h"

#define NAME(key) \
	[_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
	[_KEY_NONE] = "NONE",
	_KEYS(NAME)
};

extern _Screen _screen;

size_t events_read(void *buf, size_t len) {
	int key = _read_key();
	char event[len];
	if(key == _KEY_NONE){
		sprintf(event, "t %u\n", _uptime());
	}else{
		sprintf(event, "k%c %s\n",(key & 0x8000 ? 'd':'u'), keyname[key & 0xff]);
	}
	memcpy(buf, event, len);
	return strlen(buf);
}

static char dispinfo[128] __attribute__((used));

void dispinfo_read(void *buf, off_t offset, size_t len) {
	//printf("the dispinfo buf:%s, the dispinfo:%s, len:%d\n", buf, dispinfo, len);
	memcpy(buf, dispinfo + offset, len);
}

void fb_write(const void *buf, off_t offset, size_t len) {
	//int width = _screen.width * 4;
	//printf("offset:%d, len:%d\n", offset, len);
	offset /= 4;
	len /= 4;
	int x = offset % _screen.width;
	int y = (int)(offset / _screen.width);
	int w,h;
	if((len + x) >= _screen.width){
		w = _screen.width - x;
		_draw_rect(buf, x, y, w, 1);
		buf += w * 4;
		w = _screen.width;
		h = (int)((offset + len) / _screen.width) - y;
		_draw_rect(buf, 0, y + 1, w, h);
		buf += w * h * 4;
		w = (offset + len) % _screen.width;
		_draw_rect(buf, 0, y + h, w, 1);
	}else{
		_draw_rect(buf, x, y, len, 1);
	}
}

void init_device() {
	_ioe_init();

	// TODO: print the string to array `dispinfo` with the format
	// described in the Navy-apps convention
	sprintf(dispinfo, "WIDTH:%d\nHEIGHT:%d", _screen.width, _screen.height);
}
