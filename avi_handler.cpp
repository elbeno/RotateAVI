#include "stdafx.h"
#include <vfw.h>
#include <aviriff.h>
#include "avi_handler.h"
#include "jpeg_funcs.h"
#include <cassert>
#include "avi_exceptions.h"

#include <list>
using namespace std;

// ----------------------------------------------------------------------------

static bytetotal invalid_bytetotal(-1, -1);

// ----------------------------------------------------------------------------

volatile int avi_error;
volatile unsigned int error_data;

// ----------------------------------------------------------------------------

typedef AVIOLDINDEX::_avioldindex_entry AVIINDEX_ENTRY;
static list<AVIINDEX_ENTRY>	idx_list;

static long movi_pos = 0;
static long largest_chunk_pos = 0;
static size_t largest_chunksize = 0;
static int num_frames = 0;
static int current_frame = 0;
static unsigned int lastid = 0;
static unsigned int streamtype = 0;
static int image_width = 0;
static int image_height = 0;

static unsigned char *pJPGBuffer = 0;
static unsigned char *pDecompBuffer = 0;
static unsigned char *pRotateBuffer = 0;
static unsigned char *pRotateBuffer0 = 0;
static unsigned char *pRotateBuffer1 = 0;

// ----------------------------------------------------------------------------

eRotation rot = CW_90;
volatile int percent_complete = 0;
volatile bool stop_thread = false;

// ----------------------------------------------------------------------------

static BYTE MJPGDHTSeg[0x1A4] = {
 /* JPEG DHT Segment for YCrCb omitted from MJPG data */
0xFF,0xC4,0x01,0xA2,
0x00,0x00,0x01,0x05,0x01,0x01,0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x01,0x00,0x03,0x01,0x01,0x01,0x01,
0x01,0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
0x08,0x09,0x0A,0x0B,0x10,0x00,0x02,0x01,0x03,0x03,0x02,0x04,0x03,0x05,0x05,0x04,0x04,0x00,
0x00,0x01,0x7D,0x01,0x02,0x03,0x00,0x04,0x11,0x05,0x12,0x21,0x31,0x41,0x06,0x13,0x51,0x61,
0x07,0x22,0x71,0x14,0x32,0x81,0x91,0xA1,0x08,0x23,0x42,0xB1,0xC1,0x15,0x52,0xD1,0xF0,0x24,
0x33,0x62,0x72,0x82,0x09,0x0A,0x16,0x17,0x18,0x19,0x1A,0x25,0x26,0x27,0x28,0x29,0x2A,0x34,
0x35,0x36,0x37,0x38,0x39,0x3A,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x53,0x54,0x55,0x56,
0x57,0x58,0x59,0x5A,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x73,0x74,0x75,0x76,0x77,0x78,
0x79,0x7A,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,
0x9A,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,
0xBA,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,
0xDA,0xE1,0xE2,0xE3,0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,0xEA,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,
0xF8,0xF9,0xFA,0x11,0x00,0x02,0x01,0x02,0x04,0x04,0x03,0x04,0x07,0x05,0x04,0x04,0x00,0x01,
0x02,0x77,0x00,0x01,0x02,0x03,0x11,0x04,0x05,0x21,0x31,0x06,0x12,0x41,0x51,0x07,0x61,0x71,
0x13,0x22,0x32,0x81,0x08,0x14,0x42,0x91,0xA1,0xB1,0xC1,0x09,0x23,0x33,0x52,0xF0,0x15,0x62,
0x72,0xD1,0x0A,0x16,0x24,0x34,0xE1,0x25,0xF1,0x17,0x18,0x19,0x1A,0x26,0x27,0x28,0x29,0x2A,
0x35,0x36,0x37,0x38,0x39,0x3A,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x53,0x54,0x55,0x56,
0x57,0x58,0x59,0x5A,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x73,0x74,0x75,0x76,0x77,0x78,
0x79,0x7A,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x92,0x93,0x94,0x95,0x96,0x97,0x98,
0x99,0x9A,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,
0xB9,0xBA,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,
0xD9,0xDA,0xE2,0xE3,0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,0xEA,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8,
0xF9,0xFA
};

struct JFIFHeader
{
  BYTE SOI[2];          /* 00h  Start of Image Marker     */
  BYTE APP0[2];         /* 02h  Application Use Marker    */
  BYTE Length[2];       /* 04h  Length of APP0 Field      */
  BYTE Identifier[5];   /* 06h  "JFIF" (zero terminated) Id String */
  BYTE Version[2];      /* 07h  JFIF Format Revision      */
  BYTE Units;           /* 09h  Units used for Resolution */
  BYTE Xdensity[2];     /* 0Ah  Horizontal Resolution     */
  BYTE Ydensity[2];     /* 0Ch  Vertical Resolution       */
  BYTE XThumbnail;      /* 0Eh  Horizontal Pixel Count    */
  BYTE YThumbnail;      /* 0Fh  Vertical Pixel Count      */
};

static void ConstructWellFormedJPEG(unsigned char *pBuf)
{
	// mjpegs off cameras aren't that well-behaved, so fix up the jpg header
	//JFIFHeader *pJFH = (JFIFHeader *)pBuf;
	//pJFH->Identifier[0] = 'J';
	//pJFH->Identifier[1] = 'F';
	//pJFH->Identifier[2] = 'I';
	//pJFH->Identifier[3] = 'F';
	//pJFH->Version[0] = 1;
	//pJFH->Version[1] = 2;

	// add the MJPGDHTSeg
	memcpy(&pBuf[sizeof(JFIFHeader)], MJPGDHTSeg, sizeof(MJPGDHTSeg));
}

int ChunkHandler::RotateImage(unsigned char *pSrc, unsigned char *pDest, int srclen, int width, int height)
{
	// video memory
	int img_rgb_size = width * height * 3;
	if (pDecompBuffer == 0)
	{
		pDecompBuffer = new unsigned char[img_rgb_size];
		pRotateBuffer0 = new unsigned char[img_rgb_size];
		pRotateBuffer1 = new unsigned char[img_rgb_size];
		pRotateBuffer = pRotateBuffer0;
	}

	// save original jpeg header and move it down in the buffer
	JFIFHeader hdr;
	memcpy(&hdr, &pSrc[sizeof(MJPGDHTSeg)], sizeof(JFIFHeader));
	memcpy(pSrc, &hdr, sizeof(JFIFHeader));

	// well-form the jpg
	ConstructWellFormedJPEG(pSrc);
	int jpglen = srclen + sizeof(MJPGDHTSeg);

	// we could save the jpeg here...
	static bool saved_decomp_jpg = true;
	if (!saved_decomp_jpg)
	{
		FILE *fp = fopen("test_decomp.jpg", "wb");
		if (fp)
		{
			fwrite(pJPGBuffer, 1, jpglen, fp);
			fclose(fp);
		}
		saved_decomp_jpg = true;
	}

	// decompress
	JPEGHandler jpgh_decomp(pJPGBuffer, jpglen);
	jpgh_decomp.DecompressToRGB(pDecompBuffer, img_rgb_size);

	memcpy(pRotateBuffer, pDecompBuffer, img_rgb_size);
	// rotate
	int destx, desty;
	unsigned char *pRotSrc;
	unsigned char *pRotDest;
	switch (rot)
	{
#ifndef CRIPPLEWARE
	case CW_90:
		for (int srcy = 0; srcy < height; ++srcy)
		{
			for (int srcx = 0; srcx < width; ++srcx)
			{
				destx = height-1-srcy;
				desty = srcx;
				pRotSrc = &pDecompBuffer[(srcy * width + srcx) * 3];
				pRotDest = &pRotateBuffer[(desty * height + destx) * 3];

				*pRotDest++ = *pRotSrc++;
				*pRotDest++ = *pRotSrc++;
				*pRotDest++ = *pRotSrc++;
			}
		}
		break;
#endif

	case CW_180:
		for (int srcy = 0; srcy < height; ++srcy)
		{
			for (int srcx = 0; srcx < width; ++srcx)
			{
				destx = width-1-srcx;
				desty = height-1-srcy;
				pRotSrc = &pDecompBuffer[(srcy * width + srcx) * 3];
				pRotDest = &pRotateBuffer[(desty * width + destx) * 3];

				*pRotDest++ = *pRotSrc++;
				*pRotDest++ = *pRotSrc++;
				*pRotDest++ = *pRotSrc++;
			}
		}
		break;

	case MIRROR_X:
		for (int srcy = 0; srcy < height; ++srcy)
		{
			for (int srcx = 0; srcx < width; ++srcx)
			{
				destx = width-1-srcx;
				desty = srcy;
				pRotSrc = &pDecompBuffer[(srcy * width + srcx) * 3];
				pRotDest = &pRotateBuffer[(desty * width + destx) * 3];

				*pRotDest++ = *pRotSrc++;
				*pRotDest++ = *pRotSrc++;
				*pRotDest++ = *pRotSrc++;
			}
		}
		break;

	case MIRROR_Y:
		for (int srcy = 0; srcy < height; ++srcy)
		{
			desty = height-1-srcy;
			pRotSrc = &pDecompBuffer[srcy * width * 3];
			pRotDest = &pRotateBuffer[desty * width * 3];
			memcpy(pRotDest, pRotSrc, width*3);
		}
		break;

	default:
	case ACW_90:
		for (int srcy = 0; srcy < height; ++srcy)
		{
			for (int srcx = 0; srcx < width; ++srcx)
			{
				destx = srcy;
				desty = width-1-srcx;
				pRotSrc = &pDecompBuffer[(srcy * width + srcx) * 3];
				pRotDest = &pRotateBuffer[(desty * height + destx) * 3];

				*pRotDest++ = *pRotSrc++;
				*pRotDest++ = *pRotSrc++;
				*pRotDest++ = *pRotSrc++;
			}
		}
		break;
	}

	// compress
	JPEGHandler jpgh_comp(pRotateBuffer, img_rgb_size);
	int ret_size;
	if (rot == CW_90 || rot == ACW_90)
		ret_size = jpgh_comp.CompressFromRGB(pDest, img_rgb_size, height, width);
	else
		ret_size = jpgh_comp.CompressFromRGB(pDest, img_rgb_size, width, height);

	// we could save the jpeg here...
	static bool saved_comp_jpg = true;
	if (!saved_comp_jpg)
	{
		FILE *fp = fopen("test_comp.jpg", "wb");
		if (fp)
		{
			fwrite(pDest, 1, sizeof(JFIFHeader), fp);
			fwrite(MJPGDHTSeg, 1, sizeof(MJPGDHTSeg), fp);
			fwrite(&pDest[sizeof(JFIFHeader)], 1, ret_size-sizeof(JFIFHeader), fp);
			fclose(fp);
		}
		saved_comp_jpg = true;
	}

	// fiddle the jpeg header again
	memcpy(pDest, &hdr, sizeof(JFIFHeader));

	return ret_size;
}

// ----------------------------------------------------------------------------

ChunkHandler::ChunkHandler(unsigned int id, FileBuf &pIn, FileBuf &pOut)
	: mfpIn(pIn)
	, mfpOut(pOut)
	, chunkid(id)
	, chunksize(0)
{
	lastid = chunkid;
}

bytetotal ChunkHandler::ProcessSubChunks()
{
	bytetotal b;
	while (b.read < chunksize)
	{
		ChunkHandler *pHandler = Create(mfpIn, mfpOut);
		if (pHandler == 0)
			return invalid_bytetotal;
		bytetotal ret = pHandler->ProcessChunk();
		if (ret.read == -1)
		{
			delete pHandler;
			return invalid_bytetotal;
		}
		b += ret;
		delete pHandler;
	}
	return b;
}

bool ChunkHandler::ReadChunkSize()
{
	size_t bytes_read = mfpIn.read(&chunksize, 4);
	if (chunkid != FCC('LIST') && chunkid != FCC('RIFF'))
	{
		if (largest_chunksize < chunksize) largest_chunksize = chunksize;
	}
	return bytes_read == 4;
}

static unsigned char *pChunkData = 0;
static size_t unk_chunk_datasize = 0;

bytetotal ChunkHandler::ProcessChunk()
{
	bytetotal ret;
	if (!ReadChunkSize()) { return invalid_bytetotal; }

	mfpOut.write(&chunkid, 4);
	mfpOut.write(&chunksize, 4);

	chunksize += chunksize&1;

	// allocate space for unknown chunk
	if (chunksize > unk_chunk_datasize)
	{
		delete[] pChunkData;
		pChunkData = 0;
	}
	if (pChunkData == 0)
	{
		unk_chunk_datasize = chunksize;
		pChunkData = new unsigned char[chunksize];
	}

	// copy the unknown chunk unchanged to the output
	ret.read = mfpIn.read(pChunkData, chunksize);
	ret.written = mfpOut.write(pChunkData, chunksize);

	if (ret.read == -1) { return invalid_bytetotal; }
	if (ret.written == -1) { return invalid_bytetotal; }

	ret.read += 8;
	ret.written += 8;
	return ret;
}

// ----------------------------------------------------------------------------

class RIFFChunkHandler : public ChunkHandler
{
public:
	RIFFChunkHandler(unsigned int id, FileBuf &pIn, FileBuf &pOut) : ChunkHandler(id, pIn, pOut) {}
	virtual bytetotal	ProcessChunk();
};

bytetotal RIFFChunkHandler::ProcessChunk()
{
	// file stats
	largest_chunksize = 0;
	idx_list.clear();
	movi_pos = 0;
	num_frames = 0;
	current_frame = 0;

	bytetotal ret;
	if (!ReadChunkSize()) return invalid_bytetotal;

	// check type is 'AVI '
	unsigned int fcc;
	size_t bytes_read = mfpIn.read(&fcc, 4);
	if (bytes_read != 4) return invalid_bytetotal;
	if (fcc != FCC('AVI '))
	{
		avi_error = AVIE_BAD_AVI_CODE;
		error_data = fcc;
		return invalid_bytetotal;
	}
	chunksize -= 4;

	// write 'RIFF', leave space for size
	unsigned int riff = FCC('RIFF');
	mfpOut.write(&riff, 4);
	long writesizepos = mfpOut.tell();
	mfpOut.write(&chunksize, 4);

	// write 'AVI '
	mfpOut.write(&fcc, 4);

	ret = ProcessSubChunks();
	if (ret.read == -1) return invalid_bytetotal;
	ret.written += 4; // for 'AVI '

	// write the index chunk
	if (idx_list.size())
	{
		unsigned int idx1 = FCC('idx1');
		mfpOut.write(&idx1, 4);
		size_t idx1size = sizeof(AVIINDEX_ENTRY) * idx_list.size();
		mfpOut.write(&idx1size, 4);
		AVIINDEX_ENTRY entry;
		for (list<AVIINDEX_ENTRY>::iterator it = idx_list.begin(); it != idx_list.end(); ++it)
		{
			entry = *it;
			mfpOut.write(&entry, sizeof(AVIINDEX_ENTRY));
		}
		idx_list.clear();
		ret.written += 8 + idx1size;
		if (largest_chunksize < idx1size) largest_chunksize = idx1size;
	}

	// write the size and the largest chunk
	mfpOut.writeat(writesizepos, &ret.written, 4);
	mfpOut.writeat(largest_chunk_pos, &largest_chunksize, 4);

	// pad to evenness
	char pad = 0;
	if (ret.written & 1) { mfpOut.write(&pad, 1); ++ret.written; }

	ret.read += 12;
	ret.written += 12;
	return ret;
}

// ----------------------------------------------------------------------------

class ListChunkHandler : public ChunkHandler
{
public:
	ListChunkHandler(unsigned int id, FileBuf &pIn, FileBuf &pOut) : ChunkHandler(id, pIn, pOut) {}
	virtual bytetotal	ProcessChunk();
};

bytetotal ListChunkHandler::ProcessChunk()
{
	bytetotal ret;
	if (!ReadChunkSize()) return invalid_bytetotal;

	// read list type
	size_t bytes_read = mfpIn.read(&listid, 4);
	if (bytes_read != 4) return invalid_bytetotal;
	chunksize -= 4;

	// write LIST, leave space for size
	mfpOut.write(&chunkid, 4);
	long writesizepos = mfpOut.tell();
	mfpOut.write(&chunksize, 4);

	// write list type
	mfpOut.write(&listid, 4);
	if (listid == FCC('movi')) movi_pos = writesizepos + 4;

	ret = ProcessSubChunks();
	if (ret.read == -1) return invalid_bytetotal;
	chunksize = ret.written + 4; // for list type

	// write the size
	mfpOut.writeat(writesizepos, &chunksize, 4);
	
	// pad if needed
	char pad = 0;
	if (ret.written & 1) { mfpOut.write(&pad, 1); ++ret.written; }

	ret.read += 12;
	ret.written += 12;
	return ret;
}

// ----------------------------------------------------------------------------

class AVIHeaderChunkHandler : public ChunkHandler
{
public:
	AVIHeaderChunkHandler(int id, FileBuf &pIn, FileBuf &pOut) : ChunkHandler(id, pIn, pOut) {}
	virtual bytetotal	ProcessChunk();
};

bytetotal AVIHeaderChunkHandler::ProcessChunk()
{
	bytetotal ret;
	if (!ReadChunkSize()) return invalid_bytetotal;

	if (chunksize != sizeof(AVIMAINHEADER)-8) return invalid_bytetotal;
	AVIMAINHEADER avih;
	avih.fcc = chunkid;
	avih.cb = sizeof(AVIMAINHEADER)-8;
	char *pRestOfHeader = (char *)&avih + 8;
	ret.read = mfpIn.read(pRestOfHeader, chunksize);
	if (ret.read != chunksize) return invalid_bytetotal;

	/*
	if (avih.dwFlags & AVIF_COPYRIGHTED)
	{
		avi_error = AVIE_COPYRIGHTED;
		return invalid_bytetotal;
	}
	*/

	avih.dwFlags |= AVIF_HASINDEX;	// we're going to make an index, whether we have one or not
	num_frames = avih.dwTotalFrames;
	
	avih.dwSuggestedBufferSize = 0;	// we don't know the buffer size we'll need yet
	intptr_t offset = (char *)&avih.dwSuggestedBufferSize - (char *)&avih;
	largest_chunk_pos = mfpOut.tell() + offset;

	// rotate
	if (rot == CW_90 || rot == ACW_90)
	{
		DWORD width = avih.dwWidth;
		avih.dwWidth = avih.dwHeight;
		avih.dwHeight = width;
	}

	// write header chunk
	ret.written = mfpOut.write(&avih, sizeof(AVIMAINHEADER));
	if (ret.written == -1) return invalid_bytetotal;

	ret.read += 8;
	return ret;
}

// ----------------------------------------------------------------------------

class StreamHeaderChunkHandler : public ChunkHandler
{
public:
	StreamHeaderChunkHandler(int id, FileBuf &pIn, FileBuf &pOut) : ChunkHandler(id, pIn, pOut) {}
	virtual bytetotal	ProcessChunk();
};

bytetotal StreamHeaderChunkHandler::ProcessChunk()
{
	bytetotal ret;
	if (!ReadChunkSize()) return invalid_bytetotal;

	if (chunksize != sizeof(AVISTREAMHEADER)-8) return invalid_bytetotal;
	AVISTREAMHEADER avish;
	avish.fcc = chunkid;
	avish.cb = sizeof(AVISTREAMHEADER)-8;
	char *pRestOfHeader = (char *)&avish + 8;
	ret.read = mfpIn.read(pRestOfHeader, chunksize);
	if (ret.read != chunksize) return invalid_bytetotal;

	streamtype = avish.fccType;
	if (avish.fccType == streamtypeVIDEO)
	{
		// check for MJPG
		if (avish.fccHandler != FCC('MJPG')
			&& avish.fccHandler != FCC('mjpg'))
		{
			avi_error = AVIE_STREAM_HEADER_NOT_MJPG;
			error_data = avish.fccHandler;
			return invalid_bytetotal;
		}

		// rotate
		if (rot == CW_90 || rot == ACW_90)
		{
			short r = avish.rcFrame.right;
			avish.rcFrame.right = avish.rcFrame.bottom;
			avish.rcFrame.bottom = r;
		}
	}

	// write header chunk
	ret.written = mfpOut.write(&avish, sizeof(AVISTREAMHEADER));
	if (ret.written == -1) return invalid_bytetotal;

	ret.read += 8;
	return ret;
}

// ----------------------------------------------------------------------------

class StreamFormatChunkHandler : public ChunkHandler
{
public:
	StreamFormatChunkHandler(int id, FileBuf &pIn, FileBuf &pOut) : ChunkHandler(id, pIn, pOut) {}
	virtual bytetotal	ProcessChunk();
};

bytetotal StreamFormatChunkHandler::ProcessChunk()
{
	bytetotal ret;
	if (!ReadChunkSize()) return invalid_bytetotal;

	if (chunksize != sizeof(BITMAPINFOHEADER)) return invalid_bytetotal;
	BITMAPINFOHEADER bih;
	ret.read = mfpIn.read(&bih, chunksize);
	if (ret.read != chunksize) return invalid_bytetotal;

	// check for MJPG
	if (bih.biCompression != FCC('MJPG')
		&& bih.biCompression != FCC('mjpg'))
	{
		avi_error = AVIE_STREAM_FORMAT_NOT_MJPG;
		error_data = bih.biCompression;
		return invalid_bytetotal;
	}

	if (bih.biBitCount != 24)
	{
		avi_error = AVIE_STREAM_FORMAT_NOT_24BIT;
		error_data = bih.biBitCount;
		return invalid_bytetotal;
	}

	image_width = bih.biWidth;
	image_height = bih.biHeight;

	// rotate
	if (rot == CW_90 || rot == ACW_90)
	{
		DWORD width = bih.biWidth;
		bih.biWidth = bih.biHeight;
		bih.biHeight = width;
	}

	// write header chunk
	mfpOut.write(&chunkid, 4);
	mfpOut.write(&chunksize, 4);
	ret.written = mfpOut.write(&bih, sizeof(BITMAPINFOHEADER));
	if (ret.written == -1) return invalid_bytetotal;

	ret.read += 8;
	ret.written += 8;
	return ret;
}

// ----------------------------------------------------------------------------

class VideoFrameChunkHandler : public ChunkHandler
{
public:
	VideoFrameChunkHandler(unsigned int id, FileBuf &pIn, FileBuf &pOut) : ChunkHandler(id, pIn, pOut) {}
	virtual bytetotal	ProcessChunk();
};

static unsigned char *pNewVidData = 0;

bytetotal VideoFrameChunkHandler::ProcessChunk()
{
	bytetotal ret;
	if (!ReadChunkSize()) { ret.read = -1; return ret; }

	size_t file_chunksize = chunksize + (chunksize&1);

	AVIINDEX_ENTRY idx_entry;
	idx_entry.dwChunkId = chunkid;
	idx_entry.dwFlags = AVIIF_KEYFRAME;
	idx_entry.dwOffset = mfpOut.tell() - movi_pos;

	// allocate space for jpeg buffer
	int img_rgb_size = image_width * image_height * 3;
	if (pJPGBuffer == 0)
		pJPGBuffer = new unsigned char[img_rgb_size];

	// this is a jpeg without the MJPGDHTSeg
	assert(file_chunksize <= img_rgb_size - sizeof(MJPGDHTSeg));
	ret.read = mfpIn.read(&pJPGBuffer[sizeof(MJPGDHTSeg)], file_chunksize);

	if (pNewVidData == 0)
	    pNewVidData = new unsigned char[image_width * image_height * 3];

	chunksize = RotateImage(pJPGBuffer, pNewVidData, (int)chunksize, image_width, image_height);

	file_chunksize = chunksize + (chunksize&1);
	idx_entry.dwSize = (DWORD)file_chunksize;
	idx_list.push_back(idx_entry);

	// write chunk
	mfpOut.write(&chunkid, 4);
	mfpOut.write(&chunksize, 4);
	ret.written = mfpOut.write(pNewVidData, chunksize);

	if (ret.written == -1) return invalid_bytetotal;

	// pad if needed
	char pad = 0;
	if (ret.written & 1) { mfpOut.write(&pad, 1); ++ret.written; }

	ret.read += 8;
	ret.written += 8;

	++current_frame;
	percent_complete = 100 * current_frame / num_frames;

	return ret;
}

// ----------------------------------------------------------------------------

class IdxChunkHandler : public ChunkHandler
{
public:
	IdxChunkHandler(unsigned int id, FileBuf &pIn, FileBuf &pOut) : ChunkHandler(id, pIn, pOut) {}
	virtual bytetotal	ProcessChunk();
};

bytetotal IdxChunkHandler::ProcessChunk()
{
	bytetotal ret;
	if (!ReadChunkSize()) { ret.read = -1; return ret; }

	chunksize += chunksize&1;

	// allocate space for unknown chunk
	if (chunksize > unk_chunk_datasize)
	{
		delete[] pChunkData;
		pChunkData = 0;
	}
	if (pChunkData == 0)
	{
		unk_chunk_datasize = chunksize;
		pChunkData = new unsigned char[chunksize];
	}

	// read the chunk, don't write anything
	mfpIn.read(pChunkData, chunksize);

	ret.read = chunksize + 8;
	return ret;
}

// ----------------------------------------------------------------------------

class IndexedFrameChunkHandler : public ChunkHandler
{
public:
	IndexedFrameChunkHandler(unsigned int id, FileBuf &pIn, FileBuf &pOut) : ChunkHandler(id, pIn, pOut) {}
	virtual bytetotal	ProcessChunk();
};

bytetotal IndexedFrameChunkHandler::ProcessChunk()
{
	bytetotal ret;
	if (!ReadChunkSize()) { ret.read = -1; return ret; }

	size_t file_chunksize = chunksize + (chunksize&1);

	AVIINDEX_ENTRY idx_entry;
	idx_entry.dwChunkId = chunkid;
	idx_entry.dwFlags = 0;		
	if ((chunkid >> 16) == 0x6264)		// 'db'
		idx_entry.dwFlags = AVIIF_KEYFRAME;
	else if ((chunkid >> 16) == 0x7063)	// 'pc
		idx_entry.dwFlags = AVIIF_NO_TIME;
	idx_entry.dwOffset = mfpOut.tell() - movi_pos;
	idx_entry.dwSize = (DWORD)file_chunksize;
	idx_list.push_back(idx_entry);

	mfpOut.write(&chunkid, 4);
	mfpOut.write(&chunksize, 4);

	// allocate space for unknown chunk
	if (file_chunksize > unk_chunk_datasize)
	{
		delete[] pChunkData;
		pChunkData = 0;
	}
	if (pChunkData == 0)
	{
		unk_chunk_datasize = file_chunksize;
		pChunkData = new unsigned char[file_chunksize];
	}

	// copy the unknown chunk unchanged to the output
	ret.read = mfpIn.read(pChunkData, file_chunksize);
	ret.written = mfpOut.write(pChunkData, file_chunksize);

	if (ret.read == -1) { return invalid_bytetotal; }
	if (ret.written == -1) { return invalid_bytetotal; }

	ret.read += 8;
	ret.written += 8;
	return ret;
}

// ----------------------------------------------------------------------------

ChunkHandler *ChunkHandler::CreateRiffChunk(FileBuf &pIn, FileBuf &pOut)
{
	if (stop_thread)
		return 0;

	// read chunk ID
	unsigned int chunkid;
	size_t bytes_read = pIn.read(&chunkid, 4);
	if (bytes_read != 4)
		return 0;

	if (chunkid != FCC('RIFF'))
	{
		avi_error = AVIE_BAD_RIFF_CODE;
		error_data = chunkid;
		return 0;
	}
	return new RIFFChunkHandler(chunkid, pIn, pOut);
}

ChunkHandler *ChunkHandler::Create(FileBuf &pIn, FileBuf &pOut)
{
	if (stop_thread)
		return 0;

	// read chunk ID
	unsigned int chunkid;
	size_t bytes_read = pIn.read(&chunkid, 4);
	if (bytes_read != 4)
		return 0;

	switch (chunkid)
	{
	case FCC('LIST'):
		return new ListChunkHandler(chunkid, pIn, pOut);
		break;

	case FCC('avih'):
		return new AVIHeaderChunkHandler(chunkid, pIn, pOut);
		break;

	case FCC('strh'):
		if (listid != FCC('strl'))
		{
			avi_error = AVIE_STRH_OUTSIDE_STRL;
			return 0;
		}
		return new StreamHeaderChunkHandler(chunkid, pIn, pOut);
		break;

	case FCC('strf'):
		if (listid != FCC('strl'))
		{
			avi_error = AVIE_STRF_OUTSIDE_STRL;
			return 0;
		}
		if (lastid != FCC('strh'))
		{
			avi_error = AVIE_STRF_BEFORE_STRH;
			return 0;
		}
		if (streamtype == streamtypeVIDEO)
			return new StreamFormatChunkHandler(chunkid, pIn, pOut);
		return new ChunkHandler(chunkid, pIn, pOut);
		break;

	case FCC('strd'):
		if (listid != FCC('strl'))
		{
			avi_error = AVIE_STRD_OUTSIDE_STRL;
			return 0;
		}
		return new ChunkHandler(chunkid, pIn, pOut);
		break;

	case FCC('strn'):
		if (listid != FCC('strl'))
		{
			avi_error = AVIE_STRN_OUTSIDE_STRL;
			return 0;
		}
		return new ChunkHandler(chunkid, pIn, pOut);
		break;

	case FCC('idx1'):
		if (movi_pos == 0)
		{
			avi_error = AVIE_IDX1_BEFORE_MOVI;
			return 0;
		}
		return new IdxChunkHandler(chunkid, pIn, pOut);
		break;

	default:
		if (   (chunkid >> 16) == 0x6264	// 'db'
			|| (chunkid >> 16) == 0x6277	// 'wb'
			|| (chunkid >> 16) == 0x7063	// 'pc'
			|| (chunkid >> 16) == 0x6364)	// 'dc'
		{
			if (listid != FCC('movi')
				&& listid != FCC('rec '))
			{
				avi_error = AVIE_STREAMCHUNK_OUTSIDE_MOVI_OR_REC;
				error_data = chunkid;
				return 0;
			}
			if ((chunkid >> 16) == 0x6364)		// 'dc'
				return new VideoFrameChunkHandler(chunkid, pIn, pOut);
			return new IndexedFrameChunkHandler(chunkid, pIn, pOut);
		}
		return new ChunkHandler(chunkid, pIn, pOut);
		break;
	}

	return 0;
}

void ChunkHandler::Cleanup()
{
	delete[] pJPGBuffer;
	delete[] pDecompBuffer;
	delete[] pRotateBuffer0;
	delete[] pRotateBuffer1;
	delete[] pChunkData;
	delete[] pNewVidData;
	unk_chunk_datasize = 0;
	pChunkData = 0;
	pJPGBuffer = 0;
	pNewVidData = 0;
	pDecompBuffer = 0;
	pRotateBuffer = 0;
	pRotateBuffer0 = 0;
	pRotateBuffer1 = 0;
}

