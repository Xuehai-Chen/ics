#include <am.h>
#include <x86.h>
#include <amdev.h>
#include <klib.h>

#define SCREEN_PORT 0x100

static uint32_t* const fb __attribute__((used)) = (uint32_t *)0x40000;

size_t video_read(uintptr_t reg, void *buf, size_t size) {
	switch (reg) {
		case _DEVREG_VIDEO_INFO: 
			{
				_VideoInfoReg *info = (_VideoInfoReg *)buf;
				uint32_t val = inl(SCREEN_PORT);
				info->width = (val >> 16) & 0xffff;
				info->height = val & 0xffff;
				return sizeof(_VideoInfoReg);
			}
	}
	return 0;
}

size_t video_write(uintptr_t reg, void *buf, size_t size) {
	switch (reg) {
		case _DEVREG_VIDEO_FBCTL: 
			{
				_FBCtlReg *ctl = (_FBCtlReg *)buf;
				int width = screen_width();
				int height = screen_height();
				int i;
				int x = ctl->x, y = ctl->y;
				int w = ctl->w, h = ctl->h;
				uint32_t *pixels = ctl->pixels;
				int cp_bytes = sizeof(uint32_t) * (w < width - x ? w : width - x);
				for (i = 0; i < h && i < (height - y); i++) {
					memcpy(&fb[(y + i) *width + x], pixels, cp_bytes);
					pixels += w;
				}
				if (ctl->sync) {
					// do nothing, hardware syncs.
				}
				return sizeof(_FBCtlReg);
			}
	}
	return 0;
}

void vga_init() {
}
