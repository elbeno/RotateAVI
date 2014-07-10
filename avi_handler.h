#pragma once

#include <stdio.h>
#include "rotation.h"
#include "filebuf.h"

extern volatile int percent_complete;
extern volatile bool stop_thread;
extern eRotation rot;

struct bytetotal
{
	bytetotal(int r = 0, int w = 0) : read(r), written(w) {}
	bytetotal &operator+=(const bytetotal &b) { read += b.read; written += b.written; return *this; }

	size_t read;
	size_t written;
};

class ChunkHandler
{
public:
	ChunkHandler(unsigned int id, FileBuf &pIn, FileBuf &pOut);
	virtual ~ChunkHandler() {}

	virtual bytetotal ProcessChunk();
	virtual bytetotal ProcessSubChunks();
	virtual bool ReadChunkSize();
	static ChunkHandler *CreateRiffChunk(FileBuf &pIn, FileBuf &pOut);
	static void Cleanup();

protected:
	FileBuf &mfpIn;
	FileBuf &mfpOut;
	unsigned int chunkid;
	unsigned int listid;
	size_t chunksize;

	int RotateImage(unsigned char *pSrc, unsigned char *pDest, int srclen, int width, int height);
	ChunkHandler *Create(FileBuf &pIn, FileBuf &pOut);
};

