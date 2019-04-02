#include "fs.h"

typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

typedef struct {
	char *name;
	size_t size;
	size_t disk_offset;
	ReadFn read;
	WriteFn write;
	off_t open_offset;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB};

size_t invalid_read(void *buf, size_t offset, size_t len) {
	panic("should not reach here");
	return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
	panic("should not reach here");
	return 0;
}

void ramdisk_read(void*, off_t, size_t);
void ramdisk_write(const void*, off_t, size_t);
size_t serial_write(const void*, size_t, size_t);
size_t dispinfo_read(void*, size_t, size_t);
size_t fb_write(const void*, size_t, size_t);
size_t events_read(void*, size_t, size_t);

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
	{"stdin", 0, 0, invalid_read, invalid_write},
	{"stdout", 0, 0, invalid_read, serial_write},
	{"stderr", 0, 0, invalid_read, serial_write},
	{"/dev/fb", 0, 0, invalid_read, fb_write},
	{"/proc/dispinfo", 128, 0, dispinfo_read, invalid_write},
	{"/dev/events", 0, 0, events_read, invalid_write},
	{"/dev/tty", 0, 0, invalid_read, serial_write},
#include "files.h"
};

#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))

size_t fs_filesz(int fd){
	return file_table[fd].size;
}

int fs_open(const char *pathname, int flags, int mode){
	for(int i = 0; i < NR_FILES; i++){
		if(!strcmp(pathname, file_table[i].name)){
			//Log("pathname:%s, file_table[i].name:%s, i:%d", pathname, file_table[i].name, i);
			Finfo *fp = &file_table[i];
			fp->open_offset = 0;
			return i;
		}
	}
	Log("file not found: %s",pathname);
	assert(0);
	return -1;
}

ssize_t fs_read(int fd, void *buf, size_t len){
	assert(fd >= 0 && fd < NR_FILES);
	Finfo *fp = &file_table[fd];
	//Log("fd:%d, open_offset:%d, disk_offset:%d, len:%d", fd, fp->open_offset, fp->disk_offset, len);
	if(fd != 5){
		len = fp->open_offset + len > fp->size ? fp->size - fp->open_offset : len;
	}
	if(fp->read == NULL){
		ramdisk_read(buf, fp->disk_offset + fp->open_offset, len);
	}else{
		len = fp->read(buf, fp->disk_offset + fp->open_offset, len);
	}
	fp->open_offset += len;
	//Log("content len:%d", strlen(buf));
	return len;
}

ssize_t fs_write(int fd, const void *buf, size_t len){
	assert(fd >= 0 && fd < NR_FILES);
	//printf("fd:%d, len:%d\n", fd,  len);
	Finfo *fp= &file_table[fd];
	//TODO len should be handled properly
	if(fd != 1 && fd != 2 && fd != 6){
		len = fp->open_offset + len > fp->size ? fp->size - fp->open_offset : len;
	}
	if(fp->write == NULL){
		ramdisk_write(buf, fp->disk_offset + fp->open_offset, len);
	}else{
		len = fp->write(buf, fp->disk_offset + fp->open_offset, len);
	}
	fp->open_offset += len;
	//printf("open_offset:%d\n", fp->open_offset);
	return len;
}

off_t fs_lseek(int fd, off_t offset, int whence){
	assert(fd >= 0 && fd < NR_FILES);
	//Log("fd:%d, offset:%d, whence:%d", fd, offset, whence);
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
	//Log("closing file: %d", fd);
	return 0;
}

void init_fs() {
	// TODO: initialize the size of /dev/fb
	Finfo *fp = &file_table[FD_FB];
	fp->size = screen_width() * screen_height() * sizeof(uint32_t);
}
