// dshow_rotate.cpp : implementation file
//

#include "stdafx.h"
#include "rotatefilter.h"
#include <cassert>
#include "jpeg_funcs.h"


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


CRotateTransform::CRotateTransform(LPUNKNOWN pUnkOuter, HRESULT *phr, eRotation rot)
	: CTransformFilter(NAME("Rotate Transform"), pUnkOuter, GUID_NULL)
	, mRot(rot)
{
}

HRESULT CRotateTransform::CheckTransform(const CMediaType *mtIn, const CMediaType *mtOut)
{
    CheckPointer(mtIn,E_POINTER);
    CheckPointer(mtOut,E_POINTER);
	return S_OK;
}

HRESULT CRotateTransform::DecideBufferSize(IMemAllocator *pAlloc,ALLOCATOR_PROPERTIES *pProperties)
{
    // Is the input pin connected
    if (m_pInput->IsConnected() == FALSE)
        return E_UNEXPECTED;

    CheckPointer(pAlloc,E_POINTER);
    CheckPointer(pProperties,E_POINTER);
    HRESULT hr = S_OK;

    pProperties->cBuffers = 1;
	CMediaType mt = m_pInput->CurrentMediaType();
	pProperties->cbBuffer = mt.GetSampleSize();
	pProperties->cbBuffer = 640 * 480 * 3;
    ASSERT(pProperties->cbBuffer);

    // Ask the allocator to reserve us some sample memory, NOTE the function
    // can succeed (that is return NOERROR) but still not have allocated the
    // memory that we requested, so we must check we got whatever we wanted

    ALLOCATOR_PROPERTIES Actual;
    hr = pAlloc->SetProperties(pProperties,&Actual);
    if (FAILED(hr)) return hr;
    
    ASSERT( Actual.cBuffers == 1 );
    if (pProperties->cBuffers > Actual.cBuffers ||  pProperties->cbBuffer > Actual.cbBuffer)
	{
		return E_FAIL;
    }
    return S_OK;

}

HRESULT CRotateTransform::GetMediaType(int iPosition, CMediaType *pMediaType)
{
    // Is the input pin connected

    if (m_pInput->IsConnected() == FALSE)
	{
        return E_UNEXPECTED;
    }

    // This should never happen

    if (iPosition < 0)
	{
        return E_INVALIDARG;
    }

    // Do we have more items to offer

    if (iPosition > 0)
	{
        return VFW_S_NO_MORE_ITEMS;
	}

	//input
	CheckPointer(pMediaType,E_POINTER);
	*pMediaType = m_pInput->CurrentMediaType();

	if (mRot != CW_180)
	{
		VIDEOINFOHEADER *pVIH = (VIDEOINFOHEADER *) pMediaType->Format();
		int width = pVIH->bmiHeader.biWidth;
		pVIH->bmiHeader.biWidth = pVIH->bmiHeader.biHeight;
		pVIH->bmiHeader.biHeight = width;
	}

    return S_OK;

} // GetMediaType


HRESULT CRotateTransform::Copy(IMediaSample *pSource, IMediaSample *pDest) const
{
    CheckPointer(pSource,E_POINTER);   
    CheckPointer(pDest,E_POINTER);   

    // Copy the sample times

    REFERENCE_TIME TimeStart, TimeEnd;
    if (NOERROR == pSource->GetTime(&TimeStart, &TimeEnd)) {
        pDest->SetTime(&TimeStart, &TimeEnd);
    }

    LONGLONG MediaStart, MediaEnd;
    if (pSource->GetMediaTime(&MediaStart,&MediaEnd) == NOERROR) {
        pDest->SetMediaTime(&MediaStart,&MediaEnd);
    }

    // Copy the Sync point property

    HRESULT hr = pSource->IsSyncPoint();
    if (hr == S_OK) {
        pDest->SetSyncPoint(TRUE);
    }
    else if (hr == S_FALSE) {
        pDest->SetSyncPoint(FALSE);
    }
    else {  // an unexpected error has occured...
        return E_UNEXPECTED;
    }

    // Copy the preroll property

    hr = pSource->IsPreroll();
    if (hr == S_OK) {
        pDest->SetPreroll(TRUE);
    }
    else if (hr == S_FALSE) {
        pDest->SetPreroll(FALSE);
    }
    else {  // an unexpected error has occured...
        return E_UNEXPECTED;
    }

    // Copy the discontinuity property

    hr = pSource->IsDiscontinuity();
    if (hr == S_OK) {
    pDest->SetDiscontinuity(TRUE);
    }
    else if (hr == S_FALSE) {
        pDest->SetDiscontinuity(FALSE);
    }
    else {  // an unexpected error has occured...
        return E_UNEXPECTED;
    }

    // Copy the media type
	/*
    AM_MEDIA_TYPE *pMediaType;
    pSource->GetMediaType(&pMediaType);
    pDest->SetMediaType(pMediaType);
    DeleteMediaType(pMediaType);
	*/

    return NOERROR;

} // Copy


HRESULT CRotateTransform::Transform(IMediaSample *pIn, IMediaSample *pOut)
{
    CheckPointer(pIn,E_POINTER);
    CheckPointer(pOut,E_POINTER);

	// mjpegs off cameras aren't that well-behaved, so fix up the jpg header
	BYTE *pSrcData;
    pIn->GetPointer(&pSrcData);
	JFIFHeader *pJFH = (JFIFHeader *)pSrcData;
	pJFH->Identifier[0] = 'J';
	pJFH->Identifier[1] = 'F';
	pJFH->Identifier[2] = 'I';
	pJFH->Identifier[3] = 'F';
	pJFH->Version[0] = 1;
	pJFH->Version[1] = 2;

	// copy all the gubbins
	Copy(pIn, pOut);

    int srclen = pIn->GetSize();

	// make the jpeg well-formed for decompression
	char *pJPEG = new char[srclen + sizeof(MJPGDHTSeg)];
	memcpy(pJPEG, pJFH, sizeof(JFIFHeader));
	memcpy(&pJPEG[sizeof(JFIFHeader)], MJPGDHTSeg, sizeof(MJPGDHTSeg));
	memcpy(&pJPEG[sizeof(JFIFHeader) + sizeof(MJPGDHTSeg)], pSrcData + sizeof(JFIFHeader), srclen - sizeof(JFIFHeader));
	int jpegsize = srclen + sizeof(MJPGDHTSeg);

	// we could save the jpeg here...
	/*
	static bool saved_decomp_jpg = false;
	if (!saved_decomp_jpg)
	{
		FILE *fp = fopen("test_decomp.jpg", "wb");
		fwrite(pJPEG, 1, jpegsize, fp);
		fclose(fp);
		saved_decomp_jpg = true;
	}
	*/

	// set up the header for bmp
	const AM_MEDIA_TYPE* pInType = &m_pInput->CurrentMediaType();
    VIDEOINFOHEADER vih;
	memcpy(&vih, (VIDEOINFOHEADER *) pInType->pbFormat, sizeof(VIDEOINFOHEADER));
	vih.bmiHeader.biCompression = BI_RGB;
	vih.bmiHeader.biCompression = 0;
	assert(vih.bmiHeader.biBitCount == 24);
	vih.bmiHeader.biSizeImage = vih.bmiHeader.biHeight * vih.bmiHeader.biWidth * 3;

	// decompress
	char *bmpdata = new char[vih.bmiHeader.biSizeImage];
	JPEGHandler jpgh_decomp((char *)pJPEG, jpegsize);
	jpgh_decomp.DecompressToRGB(bmpdata, vih.bmiHeader.biSizeImage);
	delete[] pJPEG;

	// we could save the bmp here...
	// but it's got RGB values and scanlines reversed
	/*
	static bool saved_bmp = false;
	if (!saved_bmp)
	{
		BITMAPFILEHEADER bmpfh;
		memset(&bmpfh, 0, sizeof(BITMAPFILEHEADER));
		bmpfh.bfType = MAKEWORD('B','M');
		bmpfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + vih.bmiHeader.biSizeImage;
		bmpfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

		FILE *fp = fopen("test.bmp", "wb");
		fwrite(&bmpfh, 1, sizeof(BITMAPFILEHEADER), fp);
		fwrite(&vih.bmiHeader, 1, sizeof(BITMAPINFOHEADER), fp);
		fwrite(bmpdata, 1, vih.bmiHeader.biSizeImage, fp);
		fclose(fp);
		saved_bmp = true;
	}
	*/

	// rotate the bmp
	char *bmprot = new char[vih.bmiHeader.biSizeImage];
	int destx, desty;
	char *pRotSrc;
	char *pRotDest;
	switch (mRot)
	{
	case CW_90:
		for (int srcy = 0; srcy < vih.bmiHeader.biHeight; ++srcy)
		{
			for (int srcx = 0; srcx < vih.bmiHeader.biWidth; ++srcx)
			{
				destx = vih.bmiHeader.biHeight-1-srcy;
				desty = srcx;
				pRotSrc = &bmpdata[(srcy * vih.bmiHeader.biWidth + srcx) * 3];
				pRotDest = &bmprot[(desty * vih.bmiHeader.biHeight + destx) * 3];

				*pRotDest++ = *pRotSrc++;
				*pRotDest++ = *pRotSrc++;
				*pRotDest++ = *pRotSrc++;
			}
		}
		break;

	case CW_180:
		for (int srcy = 0; srcy < vih.bmiHeader.biHeight; ++srcy)
		{
			for (int srcx = 0; srcx < vih.bmiHeader.biWidth; ++srcx)
			{
				destx = vih.bmiHeader.biWidth-1-srcx;
				desty = vih.bmiHeader.biHeight-1-srcy;
				pRotSrc = &bmpdata[(srcy * vih.bmiHeader.biWidth + srcx) * 3];
				pRotDest = &bmprot[(desty * vih.bmiHeader.biWidth + destx) * 3];

				*pRotDest++ = *pRotSrc++;
				*pRotDest++ = *pRotSrc++;
				*pRotDest++ = *pRotSrc++;
			}
		}
		break;

	case ACW_90:
		for (int srcy = 0; srcy < vih.bmiHeader.biHeight; ++srcy)
		{
			for (int srcx = 0; srcx < vih.bmiHeader.biWidth; ++srcx)
			{
				destx = srcy;
				desty = vih.bmiHeader.biWidth-1-srcx;
				pRotSrc = &bmpdata[(srcy * vih.bmiHeader.biWidth + srcx) * 3];
				pRotDest = &bmprot[(desty * vih.bmiHeader.biHeight + destx) * 3];

				*pRotDest++ = *pRotSrc++;
				*pRotDest++ = *pRotSrc++;
				*pRotDest++ = *pRotSrc++;
			}
		}
		break;
	}

	// compress the bmp
    BYTE *pDestData;
    pOut->GetPointer(&pDestData);
	int destlen = pOut->GetSize();
	assert(destlen == vih.bmiHeader.biSizeImage);
	JPEGHandler jpgh_comp(bmprot, vih.bmiHeader.biSizeImage);
	int complen;
	if (mRot != CW_180)
		complen = jpgh_comp.CompressFromRGB((char *)pDestData, destlen, vih.bmiHeader.biHeight, vih.bmiHeader.biWidth);
	else
		complen = jpgh_comp.CompressFromRGB((char *)pDestData, destlen, vih.bmiHeader.biWidth, vih.bmiHeader.biHeight);
	pOut->SetActualDataLength(complen);

	// done with the decompressed source
	delete[] bmprot;
	delete[] bmpdata;

	// we could save the jpeg here...
	/*
	static bool saved_comp_jpg = false;
	if (!saved_comp_jpg)
	{
		FILE *fp = fopen("test_comp.jpg", "wb");
		fwrite(pDestData, 1, complen, fp);
		fclose(fp);
		saved_comp_jpg = true;
	}
	*/

	return S_OK;
}

// check if we can support this specific proposed type and format
HRESULT CRotateTransform::CheckInputType(const CMediaType *pmt) 
{
	return S_OK;
    if (*pmt->Type() != MEDIATYPE_Video)
	{
		return S_FALSE;
    }
    if ((*pmt->FormatType() != FORMAT_VideoInfo) &&
		(*pmt->FormatType() != FORMAT_VideoInfo2))
	{
		return S_FALSE;
    }

    if (*pmt->Subtype() != MEDIASUBTYPE_MJPG)
	{
		return S_FALSE;
    }

    return S_OK;
}


// methods from strmbase.lib that are confused about char/wchar_t

STDMETHODIMP
CBaseFilter::QueryVendorInfo(
    LPWSTR* pVendorInfo)
{
    UNREFERENCED_PARAMETER(pVendorInfo);
    return E_NOTIMPL;
}

STDMETHODIMP
CBaseFilter::JoinFilterGraph(
    IFilterGraph * pGraph,
    LPCWSTR pName)
{
    CAutoLock cObjectLock(m_pLock);

    // NOTE: we no longer hold references on the graph (m_pGraph, m_pSink)

    m_pGraph = pGraph;
    if (m_pGraph) {
        HRESULT hr = m_pGraph->QueryInterface(IID_IMediaEventSink,
                        (void**) &m_pSink);
        if (FAILED(hr)) {
            ASSERT(m_pSink == NULL);
        }
        else m_pSink->Release();        // we do NOT keep a reference on it.
    } else {
        // if graph pointer is null, then we should
        // also release the IMediaEventSink on the same object - we don't
        // refcount it, so just set it to null
        m_pSink = NULL;
    }


    if (m_pName) {
        delete[] m_pName;
        m_pName = NULL;
    }

    if (pName) {
        DWORD nameLen = wcslen(pName)+1;
        m_pName = new WCHAR[nameLen];
        if (m_pName) {
            CopyMemory(m_pName, pName, nameLen*sizeof(WCHAR));
        } else {
            // !!! error here?
            ASSERT(FALSE);
        }
    }

    return NOERROR;
}

//
// FindPin
//
// If Id is In or Out then return the IPin* for that pin
// creating the pin if need be.  Otherwise return NULL with an error.

STDMETHODIMP CTransformFilter::FindPin(LPCWSTR Id, IPin **ppPin)
{
    CheckPointer(ppPin,E_POINTER);
    ValidateReadWritePtr(ppPin,sizeof(IPin *));

    if (0==wcscmp(Id,L"In")) {
        *ppPin = GetPin(0);
    } else if (0==wcscmp(Id,L"Out")) {
        *ppPin = GetPin(1);
    } else {
        *ppPin = NULL;
        return VFW_E_NOT_FOUND;
    }

    HRESULT hr = NOERROR;
    //  AddRef() returned pointer - but GetPin could fail if memory is low.
    if (*ppPin) {
        (*ppPin)->AddRef();
    } else {
        hr = E_OUTOFMEMORY;  // probably.  There's no pin anyway.
    }
    return hr;
}


