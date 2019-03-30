#include "common.h"
#include <amdev.h>

size_t serial_write(const void *buf, size_t offset, size_t len) {
	_yield();
	for(int i = 0; i < len; i++){
		_putc(*((char*)buf + i));
	}
	return len;
}

#define NAME(key) \
	[_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
	[_KEY_NONE] = "NONE",
	_KEYS(NAME)
};

size_t events_read(void *buf, size_t offset, size_t len) {
	_yield();
	_Device* dev = _device(1);
	_KbdReg kbd;
	dev->read(_DEVREG_INPUT_KBD, &kbd, sizeof(_KbdReg));
	memset(buf, 0, len);
	//Log("keycode:%d, keydown:%d",kbd.keycode, kbd.keydown);
	if((kbd.keycode & 0xff) == _KEY_NONE){
		_Device* timer = _device(2);
		_UptimeReg uptime;
		timer->read(_DEVREG_TIMER_UPTIME, &uptime, len);
		sprintf(buf, "t %d\n", uptime.lo);
	}else{
		sprintf(buf, "k%s %s\n",(kbd.keydown ? "d":"u"), keyname[kbd.keycode & 0xff]);
	}
	//Log("event: %s", event);
	//memcpy(buf, event, len);
	//Log("buf: %s, %d", buf, len);
	return strlen(buf);
}

static char dispinfo[128] __attribute__((used));

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
	memcpy(buf, dispinfo + offset, len);
	return len;
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
	_yield();
	//Log("fb_write, offset:%d, len:%d",offset,len);
	_Device* dev = _device(3);
	_VideoInfoReg video_info;
	dev->read(_DEVREG_VIDEO_INFO, &video_info, 4);
	offset /= 4;
	len /= 4;
	int x = offset % video_info.width;
	int y = (int)(offset / video_info.width);
	int w,h;
	_FBCtlReg fb;
	//Log("len:%d\tx:%d\ty:%d\twidth:%d\theight:%d",len,x,y,video_info.width,video_info.height);
	if((len + x) >= video_info.width){
		w = video_info.width - x;
		fb.x = x;
		fb.y = y;
		fb.w = w;
		fb.h = 1;
		fb.pixels = (uint32_t *)buf;
		dev->write(_DEVREG_VIDEO_FBCTL, &fb, len);
		buf += w * 4;
		w = video_info.width;
		h = (int)((offset + len) / video_info.width) - y;
		fb.x = 0;
		fb.y = y + 1;
		fb.w = w;
		fb.h = h;
		fb.pixels = (uint32_t *)buf;
		dev->write(_DEVREG_VIDEO_FBCTL, &fb, len);
		buf += w * h * 4;
		w = (offset + len) % video_info.width;
		fb.x = 0;
		fb.y = y + h;
		fb.w = w;
		fb.h = 1;
		fb.pixels = (uint32_t*)buf;
		dev->write(_DEVREG_VIDEO_FBCTL, &fb, len);
	}else{
		fb.x = x;
		fb.y = y;
		fb.w = len;
		fb.h = 1;
		fb.pixels = (uint32_t *)buf;
		dev->write(_DEVREG_VIDEO_FBCTL, &fb, len);
	}
	return len * 4;
}

void init_device() {
	Log("Initializing devices...");
	_ioe_init();

	// TODO: print the string to array `dispinfo` with the format
	// described in the Navy-apps convention
	_Device* dev = _device(3); 
	_VideoInfoReg video_info;
	dev->read(_DEVREG_VIDEO_INFO, &video_info, 4);
	sprintf(dispinfo, "WIDTH:%d\nHEIGHT:%d", screen_width(), screen_height());
}
