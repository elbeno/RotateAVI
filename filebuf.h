#pragma once

#include <stdio.h>

class FileBuf
{
public:
	FileBuf(TCHAR *filename, bool write, size_t size);
	~FileBuf();

	size_t read(void *buffer, size_t size);
	size_t write(const void *buffer, size_t size);
	size_t writeat(long pos, const void *buffer, size_t size);
	long tell() { return ftell(fp) + (buf_current - buf); }
	bool IsValid() const { return fp != 0; }

private:
	FILE			*fp;
	long			writefilepos;
	unsigned char	*buf;
	unsigned char	*buf_current;
	size_t			buf_current_size;
	size_t			buf_max_size;
};
