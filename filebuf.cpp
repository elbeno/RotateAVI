#include "stdafx.h"
#include "filebuf.h"
#include <assert.h>
#include "avi_exceptions.h"

FileBuf::FileBuf(TCHAR *filename, bool write, size_t size)
	: buf_current_size(0)
	, buf_max_size(size)

{
	fp = _tfopen(filename, write ? "wb" : "rb");
	buf = new unsigned char[size];
	buf_current = buf;
	writefilepos = 0;
}

FileBuf::~FileBuf()
{
	// write out the remainder
	if (buf_current_size)
		fwrite(buf, 1, buf_current_size, fp);
	delete[] buf;
	if (fp)
		fclose(fp);
}


size_t FileBuf::read(void *buffer, size_t size)
{
	assert(size <= buf_max_size);

	// if we are asking for more than we have, we need to read more
	size_t used_data_size = buf_current - buf;
	size_t live_data_size = buf_current_size - used_data_size;
	if (size > live_data_size)
	{
		// if we are going to read beyond the end of the live data, shift data down
		size_t unused_data_size = buf_max_size - buf_current_size;
		if (size > unused_data_size)
		{
			memmove(buf, buf_current, live_data_size);
			buf_current = buf;
			buf_current_size = live_data_size;
		}

		size_t s = fread(&buf[buf_current_size], 1, buf_max_size-buf_current_size, fp);
		if (s == -1)
		{
			avi_error = AVIE_FILE_READ_ERROR;
			return -1;
		}
		buf_current_size += s;
	}

	memcpy(buffer, buf_current, size);
	buf_current += size;
	return size;
}

size_t FileBuf::write(const void *buffer, size_t size)
{
	assert(size <= buf_max_size);

	// if we are going to write beyond the end of our buffer, we must write it out first
	size_t unused_data_size = buf_max_size - buf_current_size;
	if (size > unused_data_size)
	{
		size_t s = fwrite(buf, 1, buf_current_size, fp);
		if (s == -1)
		{
			avi_error = AVIE_FILE_WRITE_ERROR;
			return -1;
		}
		buf_current_size = 0;
		buf_current = buf;
		writefilepos += (long)s;
	}

	memcpy(buf_current, buffer, size);
	buf_current += size;
	buf_current_size += size;
	return size;
}

size_t FileBuf::writeat(long pos, const void *buffer, size_t size)
{
	if (writefilepos > pos)
	{
		// already written at this pos - just seek back and rewrite
		long current_pos = ftell(fp);
		fseek(fp, pos, SEEK_SET);
		fwrite(buffer, 1, size, fp);
		fseek(fp, current_pos, SEEK_SET);
	}
	else
	{
		// this pos is in the buffer - write it direct
		pos -= writefilepos;
		memcpy(&buf[pos], buffer, size);
	}
	return size;
}


