#include "stdafx.h"
#include "rotation.h"
#include "RotateAVI.h"
#include "RotateAVIDlg.h"
#include "jpeg_funcs.h"
#include "avi_handler.h"
#include <vfw.h>

#pragma comment(lib, "vfw32")

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

static int ConstructWellFormedJPEG(char *pSrc, char *pDest, int srclen)
{
	// mjpegs off cameras aren't that well-behaved, so fix up the jpg header
	JFIFHeader *pJFH = (JFIFHeader *)pSrc;
	pJFH->Identifier[0] = 'J';
	pJFH->Identifier[1] = 'F';
	pJFH->Identifier[2] = 'I';
	pJFH->Identifier[3] = 'F';
	pJFH->Version[0] = 1;
	pJFH->Version[1] = 2;

	// make the jpeg well-formed for decompression
	memcpy(pDest, pJFH, sizeof(JFIFHeader));
	memcpy(&pDest[sizeof(JFIFHeader)], MJPGDHTSeg, sizeof(MJPGDHTSeg));
	memcpy(&pDest[sizeof(JFIFHeader) + sizeof(MJPGDHTSeg)], pSrc + sizeof(JFIFHeader), srclen - sizeof(JFIFHeader));
	return srclen + sizeof(MJPGDHTSeg);
}

void CRotateAVIDlg::ProcessAVI(const TCHAR *source_filename, const TCHAR *dest_filename, eRotation rot)
{
	TCHAR error_buf[1024];

    PAVIFILE source_avi = 0;
    PAVIFILE dest_avi = 0;
    PAVISTREAM pSrcVidStream = 0;
    PAVISTREAM pSrcAudioStream = 0;
	PAVISTREAM pDestVidStream = 0;
	PAVISTREAM pDestAudioStream = 0;
	char *pSrcBuffer = 0;
	char *pJPGBuffer = 0;
	char *pDecompBuffer = 0;
	char *pRotateBuffer = 0;
	char *pDestBuffer = 0;

    AVIFileInit();

	// source setup

    if (AVIFileOpen(&source_avi, source_filename, OF_READ, NULL) != AVIERR_OK)
	{
		_stprintf(error_buf, TEXT("Couldn't open file %s"), source_filename);
		MessageBox(error_buf);
		goto cleanup;
	}

	AVIFILEINFO src_avi_info;
    AVIFileInfo(source_avi, &src_avi_info, sizeof(AVIFILEINFO));

    if (AVIFileGetStream(source_avi, &pSrcVidStream, streamtypeVIDEO, 0) != AVIERR_OK)
    {
		_stprintf(error_buf, TEXT("No video stream in %s"), source_filename);
		MessageBox(error_buf);
		goto cleanup;
    }

	BITMAPINFOHEADER srcBIH;
	long srcvidstreamsize;
    AVIStreamFormatSize(pSrcVidStream, 0, &srcvidstreamsize); 
    if (srcvidstreamsize > sizeof(BITMAPINFOHEADER))
	{
		_stprintf(error_buf, TEXT("Unable to handle video stream format in %s"), source_filename);
		MessageBox(error_buf);
		goto cleanup;
	}
 
    srcvidstreamsize = sizeof(BITMAPINFOHEADER); 
    if (AVIStreamReadFormat(pSrcVidStream, 0, &srcBIH, &srcvidstreamsize) != AVIERR_OK)
	{
		_stprintf(error_buf, TEXT("Error reading stream format in %s"), source_filename);
		MessageBox(error_buf);
		goto cleanup;
	}
    if (srcBIH.biCompression != MKFOURCC('M','J','P','G'))
	{
		_stprintf(error_buf, TEXT("%s is not motion JPEG format"), source_filename);
		MessageBox(error_buf);
		goto cleanup;
	}
 
	AVISTREAMINFO vidstream_info;
    if (AVIStreamInfo(pSrcVidStream, &vidstream_info, sizeof(AVISTREAMINFO)) != AVIERR_OK)
	{
		_stprintf(error_buf, TEXT("Error reading stream info in %s"), source_filename);
		MessageBox(error_buf);
		goto cleanup;
	}

    int firstVidSrcFrame = AVIStreamStart(pSrcVidStream);
    if (firstVidSrcFrame == -1)
	{
		_stprintf(error_buf, TEXT("Video stream start error in %s"), source_filename);
		MessageBox(error_buf);
		goto cleanup;
	}
    int numVidSrcFrames = AVIStreamLength(pSrcVidStream);
    if (numVidSrcFrames == -1)
	{
		_stprintf(error_buf, TEXT("Video stream length error in %s"), source_filename);
		MessageBox(error_buf);
		goto cleanup;
	}

    AVIFileGetStream(source_avi, &pSrcAudioStream, streamtypeAUDIO, 0);
	int firstAudioSrcFrame = 0;
	int numAudioSrcFrames = 0;
	if (pSrcAudioStream)
	{
		firstAudioSrcFrame = AVIStreamStart(pSrcAudioStream);
		if (firstAudioSrcFrame == -1)
		{
			_stprintf(error_buf, TEXT("Audio stream start error in %s"), source_filename);
			MessageBox(error_buf);
			goto cleanup;
		}
		numAudioSrcFrames = AVIStreamLength(pSrcAudioStream);
		if (numAudioSrcFrames == -1)
		{
			_stprintf(error_buf, TEXT("Audio stream length error in %s"), source_filename);
			MessageBox(error_buf);
			goto cleanup;
		}
	}

	// dest setup

	BITMAPINFOHEADER destBIH;
	destBIH = srcBIH;
	if (rot != CW_180)
	{
		destBIH.biWidth = srcBIH.biHeight;
		destBIH.biHeight = srcBIH.biWidth;
	}

    if (AVIFileOpen(&dest_avi, dest_filename, OF_CREATE | OF_WRITE, NULL) != AVIERR_OK)
	{
		_stprintf(error_buf, TEXT("Couldn't open file %s"), dest_filename);
		MessageBox(error_buf);
		goto cleanup;
	}
	vidstream_info.rcFrame.left = vidstream_info.rcFrame.top = 0;
	vidstream_info.rcFrame.right = destBIH.biWidth;
	vidstream_info.rcFrame.bottom = destBIH.biHeight;
 
    if (AVIFileCreateStream(dest_avi, &pDestVidStream, &vidstream_info) != AVIERR_OK)
	{
		_stprintf(error_buf, TEXT("Error creating video stream in %s"), dest_filename);
		MessageBox(error_buf);
		goto cleanup;
	}
 
    if (AVIStreamSetFormat(pDestVidStream, 0, &destBIH, sizeof(BITMAPINFOHEADER)) != AVIERR_OK)
	{ 
		_stprintf(error_buf, TEXT("Error setting video stream format in %s"), dest_filename);
		MessageBox(error_buf);
		goto cleanup;
    } 

    if (AVIStreamSetFormat(pDestVidStream, 0, &destBIH, sizeof(BITMAPINFOHEADER)) != AVIERR_OK)
	{ 
		_stprintf(error_buf, TEXT("Error setting video stream format in %s"), dest_filename);
		MessageBox(error_buf);
		goto cleanup;
    } 

	// video memory
	int img_rgb_size = srcBIH.biHeight * srcBIH.biWidth * 3;
	pSrcBuffer = new char[img_rgb_size];
	pJPGBuffer = new char[img_rgb_size];
	pDecompBuffer = new char[img_rgb_size];
	pRotateBuffer = new char[img_rgb_size];
	pDestBuffer = new char[img_rgb_size];

	long bytes_read;
	long bytes_written;

	for (int i = firstVidSrcFrame; i < numVidSrcFrames; ++i)
	{
		if (AVIStreamRead(pSrcVidStream, i, 1, pSrcBuffer, img_rgb_size, &bytes_read, 0) != AVIERR_OK)
		{
			_stprintf(error_buf, TEXT("Error reading video stream from %s"), source_filename);
			MessageBox(error_buf);
			goto cleanup;
		}

		// well-form the jpg
		int jpglen = ConstructWellFormedJPEG(pSrcBuffer, pJPGBuffer, bytes_read);
		// decompress
		JPEGHandler jpgh_decomp(pJPGBuffer, jpglen);
		jpgh_decomp.DecompressToRGB(pDecompBuffer, img_rgb_size);
		// rotate
		int destx, desty;
		char *pRotSrc;
		char *pRotDest;
		switch (rot)
		{
		case CW_90:
			for (int srcy = 0; srcy < srcBIH.biHeight; ++srcy)
			{
				for (int srcx = 0; srcx < srcBIH.biWidth; ++srcx)
				{
					destx = srcBIH.biHeight-1-srcy;
					desty = srcx;
					pRotSrc = &pDecompBuffer[(srcy * srcBIH.biWidth + srcx) * 3];
					pRotDest = &pRotateBuffer[(desty * srcBIH.biHeight + destx) * 3];

					*pRotDest++ = *pRotSrc++;
					*pRotDest++ = *pRotSrc++;
					*pRotDest++ = *pRotSrc++;
				}
			}
			break;

		case CW_180:
			for (int srcy = 0; srcy < srcBIH.biHeight; ++srcy)
			{
				for (int srcx = 0; srcx < srcBIH.biWidth; ++srcx)
				{
					destx = srcBIH.biWidth-1-srcx;
					desty = srcBIH.biHeight-1-srcy;
					pRotSrc = &pDecompBuffer[(srcy * srcBIH.biWidth + srcx) * 3];
					pRotDest = &pRotateBuffer[(desty * srcBIH.biWidth + destx) * 3];

					*pRotDest++ = *pRotSrc++;
					*pRotDest++ = *pRotSrc++;
					*pRotDest++ = *pRotSrc++;
				}
			}
			break;

		case ACW_90:
			for (int srcy = 0; srcy < srcBIH.biHeight; ++srcy)
			{
				for (int srcx = 0; srcx < srcBIH.biWidth; ++srcx)
				{
					destx = srcy;
					desty = srcBIH.biWidth-1-srcx;
					pRotSrc = &pDecompBuffer[(srcy * srcBIH.biWidth + srcx) * 3];
					pRotDest = &pRotateBuffer[(desty * srcBIH.biHeight + destx) * 3];

					*pRotDest++ = *pRotSrc++;
					*pRotDest++ = *pRotSrc++;
					*pRotDest++ = *pRotSrc++;
				}
			}
			break;
		}
		// compress
		JPEGHandler jpgh_comp(pRotateBuffer, img_rgb_size);
		if (rot != CW_180)
			destBIH.biSizeImage = jpgh_comp.CompressFromRGB(pDestBuffer, img_rgb_size, srcBIH.biHeight, srcBIH.biWidth);
		else
			destBIH.biSizeImage = jpgh_comp.CompressFromRGB(pDestBuffer, img_rgb_size, srcBIH.biWidth, srcBIH.biHeight);

		if (AVIStreamWrite(pDestVidStream, i, 1, pDestBuffer, destBIH.biSizeImage, AVIIF_KEYFRAME, NULL, &bytes_written) != AVIERR_OK)
		{
			_stprintf(error_buf, TEXT("Error writing video stream to %s"), dest_filename);
			MessageBox(error_buf);
			goto cleanup;
		}
	} 
 
cleanup:
	delete[] pSrcBuffer;
	delete[] pDestBuffer;
	delete[] pJPGBuffer;
	delete[] pDecompBuffer;
	delete[] pRotateBuffer;
	if (pDestAudioStream) AVIStreamRelease(pDestAudioStream);
	if (pDestVidStream) AVIStreamRelease(pDestVidStream);
	if (pSrcAudioStream) AVIStreamRelease(pSrcAudioStream);
	if (pSrcVidStream) AVIStreamRelease(pSrcVidStream);
	if (dest_avi) AVIFileRelease(dest_avi);
	if (source_avi) AVIFileRelease(source_avi);

	AVIFileExit();
}


