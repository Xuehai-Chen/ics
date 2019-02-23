#include "fs.h"

typedef struct {
	char *name;
	size_t size;
	off_t disk_offset;
	off_t open_offset;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB, FD_EVENTS, FD_DISPINFO, FD_NORMAL};

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
	{"stdin (note that this is not the actual stdin)", 0, 0},
	{"stdout (note that this is not the actual stdout)", 0, 0},
	{"stderr (note that this is not the actual stderr)", 0, 0},
	[FD_FB] = {"/dev/fb", 0, 0},
	[FD_EVENTS] = {"/dev/events", 0, 0},
	[FD_DISPINFO] = {"/proc/dispinfo", 128, 0},
#include "files.h"
};

#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))

void ramdisk_read(void*, off_t, size_t);
void ramdisk_write(const void*, off_t, size_t);
void dispinfo_read(void*, off_t, size_t);
void fb_write(const void*, off_t, size_t);
size_t events_read(void*, size_t);
extern _Screen _screen;

size_t fs_filesz(int fd){
	return file_table[fd].size;
}

int fs_open(const char *pathname, int flags, int mode){
	for(int i = 0; i < NR_FILES; i++){
		if(!strcmp(pathname, file_table[i].name)){
			//file_table[i].open_offset = 0;
			return i;
		}
	}
	Log("file not found: %s",pathname);
	assert(0);
	return -1;
}

ssize_t fs_read(int fd, void *buf, size_t len){
	assert(fd >= 0 && fd < NR_FILES);
	if(fd < 4) return 0;
	if(fd == FD_EVENTS) return events_read(buf, len);
	Finfo *fp = &file_table[fd];
	len = fp->open_offset + len > fp->size ? fp->size - fp->open_offset : len;
	//Log("fd:%d, open_offset:%d, disk_offset:%d", fd, fp->open_offset, fp->disk_offset);
	if(fd == FD_DISPINFO) {
		dispinfo_read(buf, fp->open_offset, len);
		fp->open_offset += strlen(buf);
		//Log("buf:%s, size:%d\n", buf, strlen(buf));
		return strlen(buf);
	}
	ramdisk_read(buf, fp->disk_offset + fp->open_offset, len);
	fp->open_offset += len;
	return len;
}

ssize_t fs_write(int fd, const void *buf, size_t len){
	assert(fd >= 0 && fd < NR_FILES);
	//printf("fd:%d, len:%d", fd,  len);
	if(fd == FD_STDOUT || fd == FD_STDERR){
		for(int i = 0; i < len; i++){
			_putc(*((char*)buf + i));
		}
		return len;
	}else if(fd == FD_FB){
		Finfo *fp = &file_table[fd];
		len = fp->open_offset + len > fp->size ? fp->size - fp->open_offset : len;
		fb_write(buf, fp->open_offset, len);
		fp->open_offset += len;
		return len;
	}
	Finfo *fp= &file_table[fd];
	len = fp->open_offset + len > fp->size ? fp->size - fp->open_offset : len;
	ramdisk_write(buf, fp->disk_offset + fp->open_offset, len);
	fp->open_offset += len;
	return len;
}

off_t fs_lseek(int fd, off_t offset, int whence){
	//Log("fd:%d, offset:%d, whence:%d", fd, offset, whence);
	assert(fd >= 0 && fd < NR_FILES);
	if(fd < 3) return 0;
	Finfo *fp = &file_table[fd];
	switch(whence){
		case SEEK_SET:
			if(offset < 0) offset = 0;
			if(offset > fp->size) offset = fp->size;
			fp->open_offset = offset;
			break;
		case SEEK_CUR:
			if((offset + fp->open_offset) > fp->size) offset = fp->size - fp->open_offset;
			if((offset + fp->open_offset) < 0) offset = 0 - fp->open_offset;
			fp->open_offset += offset;
			break;
		case SEEK_END:
			if(offset > 0) offset = 0;
			if(offset + fp->size < 0) offset = -fp->size;
			fp->open_offset = fp->size + offset;
			break;
		default:
			assert(0);
	}
	//Log("open_offset:%d", fp->open_offset);
	return fp->open_offset;
}

int fs_close(int fd){
	return 0;
}

void init_fs() {
	// TODO: initialize the size of /dev/fb
	Finfo *fp = &file_table[FD_FB];
	fp->size = _screen.width * _screen.height * sizeof(uint32_t);
}
